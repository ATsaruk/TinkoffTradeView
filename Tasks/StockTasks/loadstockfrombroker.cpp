#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "loadstockfrombroker.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

constexpr quint64 SECS_IN_ONE_DAY   = 24 * 3600;
constexpr quint64 SECS_IN_ONE_WEEK  = 7 * SECS_IN_ONE_DAY;
constexpr quint64 SECS_IN_ONE_MONTH = 30 * SECS_IN_ONE_DAY;

LoadStockFromBroker::LoadStockFromBroker(QThread *parent)
    : IBaseTask(parent)
{
    logDebug << "loadStocksFromBrokerTask;loadStocksFromBrokerTask();+constructor!";
}

LoadStockFromBroker::~LoadStockFromBroker()
{
    logDebug << "loadStocksFromBrokerTask;~loadStocksFromBrokerTask();-destructor!";
}

void LoadStockFromBroker::setData(const StockKey &stockKey_, const Range &range)
{
    if (!range.isValid())
        throw std::invalid_argument("LoadStockFromBroker::setData: invalid loadRange!");

    stockKey = stockKey_;
    loadRange = range;

    qint64 maxLoadRange = getMaxLoadInterval(stockKey.interval());
    curRange.setRange(loadRange.getEnd(), -maxLoadRange);
    curRange.constrain(loadRange);
}

void LoadStockFromBroker::exec()
{
    Glo.broker->mutex.lock();
    connect(Glo.broker, &Broker::Api::getResopnse, this, &LoadStockFromBroker::onResponse);

    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::sendRequest()
{
    if (isStopRequested)
        return false;

    if (!Glo.broker->loadCandles(stockKey, curRange))
        return false;

    return true;
}

void LoadStockFromBroker::onResponse(QByteArray answer)
{
    if (!readCandles(answer) || !getNextLoadRange()) {
        finishTask();
        return;
    }

    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::getNextLoadRange()
{
    auto candleInterval = stockKey.intervalToSec();
    if ( curRange.getBegin() < loadRange.getBegin().addSecs(candleInterval) )
        return false;

    qint64 maxLoadRange = getMaxLoadInterval(stockKey.interval()) + candleInterval;
    curRange.displace(-maxLoadRange, -maxLoadRange);
    curRange.constrain(loadRange);
    return true;
}

void LoadStockFromBroker::finishTask()
{
    Glo.broker->mutex.unlock();

    removeIncompleteCandle(candles);

    emit finished();
}

QString LoadStockFromBroker::getName()
{
    return "TaskLoadStocksFromBroker";
}

Candles& LoadStockFromBroker::getResult()
{
    return candles;
}

qint64 LoadStockFromBroker::getMaxLoadInterval(const StockKey::INTERVAL &interval)
{
    switch (interval) {
    case StockKey::INTERVAL::DAY  :   //no break;
    case StockKey::INTERVAL::WEEK :
        //Свечи длительностью 1 день и более загрузаются с максимальным интервалом 1 месяц
        return SECS_IN_ONE_MONTH;

    case StockKey::INTERVAL::HOUR :
        //Свечи длительностью 1 час загрузаются с максимальным интервалом 1 неделя
        return SECS_IN_ONE_WEEK;

    default :
        break;
    }

    //Все свечи длительностью меньше часа имеют максимальный интервал загрузки 1 сутки
    return SECS_IN_ONE_DAY;
}

bool LoadStockFromBroker::readCandles(const QByteArray &answer)
{
    QJsonDocument doc = QJsonDocument::fromJson(answer);
    QJsonObject root(doc.object());

    QString status = root.value("status").toString();
    if (status.toLower() != "ok") {
        logCritical << QString("LoadStockFromBroker;readCandles;Status isn't ok:;%1").arg(QString(answer));
        return false;
    }

    //https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/market/get_market_candles
    QJsonObject payload(root.value("payload").toObject());

    if (!checkStockKey(payload)) {
        logCritical << QString("LoadStockFromBroker;readCandles;Error stock key:;%1").arg(QString(answer));
        return false;
    }

    QJsonArray candlesArray = payload.value("candles").toArray();

    candles.reserve(candles.size() + candlesArray.size());
    for (const auto &it: candlesArray)
        candles.emplace_back( Candle::fromJson(it.toObject()) );

    return true;
}

bool LoadStockFromBroker::checkStockKey(const QJsonObject &payload)
{
    try {  //StockKey::fromJson кидается исключениями
        StockKey recievedKey;
        recievedKey.fromJson(payload);   //payload содержит ключ акции
        if(recievedKey != stockKey) {
            QString errorCode = QString("LoadStockFromBroker;checkStockKey();recievedKey!=stockKey:;%1;%2")
                    .arg(recievedKey.keyToString(), stockKey.keyToString());
            throw std::logic_error(errorCode.toUtf8().data());
        }
    }  catch (std::exception &error) {
        logCritical << error.what();
        return false;
    }
    return true;
}

/* Проверяем последнюю свечу и если она незавершенная удаляем ее, поясню зачем это:
 * Например сейчас 17:42 и мы запрашиваем информацию по 15 минутным свечам с начала суток.
 * Время у последней полученной свечи будет 17:30 и если ничего не делать, то эта свеча будет записана в базу данных с таким временем.
 * И допустим через час мы захотим вновь загрузить свечную информацию, в итоге мы опять получим свечу на 17:30, только в этот раз она
 * будет содержать в себе полные данные за 15 минут, но в базу данных она уже не запишется, т.к. так уже етсь свеча на 17:30!
 * Т.к. в базе данных primary key для записи это figi + interval + time.
 * В итоге свеча так и останется незавершенной! это было вяснено постфактум, когда заметил отличие на моем графике и графике брокера!
 */
void LoadStockFromBroker::removeIncompleteCandle(Candles &candles)
{
    if(candles.empty())
        return;

    Candle &lastCandle = candles.back();

    uint64_t candleDuration = stockKey.intervalToSec();
    QDateTime timeCandleComplite = lastCandle.dateTime.addSecs(candleDuration);

    if (timeCandleComplite > loadRange.getEnd())
        candles.pop_back();
}

}
