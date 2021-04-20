#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "loadstockfrombroker.h"
#include "DataBase/Query/stocksquery.h"
#include "Core/globals.h"

namespace Task {

constexpr quint64 SECS_IN_ONE_DAY   = 3600 * 24;
constexpr quint64 SECS_IN_ONE_WEEK  = 3600 * 24 * 7;
constexpr quint64 SECS_IN_ONE_MONTH = 3600 * 24 * 30;

LoadStockFromBroker::LoadStockFromBroker(QThread *parent)
    : IBaseTask(parent)
{
    logDebug << "loadStocksFromBrokerTask;loadStocksFromBrokerTask();+constructor!";
}

LoadStockFromBroker::~LoadStockFromBroker()
{
    logDebug << "loadStocksFromBrokerTask;~loadStocksFromBrokerTask();-destructor!";
}

QString LoadStockFromBroker::getName()
{
    return "TaskLoadStocksFromBroker";
}

void LoadStockFromBroker::setData(const StockKey &stockKey_, const DateRange &range, const qint64 minCandleCount)
{
    stockKey = stockKey_;
    loadRange = range;
    minCandles = minCandleCount;
}

void LoadStockFromBroker::exec()
{
    if (!initRanges()) {
        emit finished();
        return; //нечего загружать
    }

    Glo.broker->mutex.lock();
    connect(Glo.broker, &Broker::Api::getResopnse, this, &LoadStockFromBroker::onResponse);

    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::sendRequest()
{
    if (isStopRequested)
        return false;

    if (!checkRange())
        if (!loadExtraRange())
            return false;

    if (!Glo.broker->loadCandles(stockKey, curRange))
        return false;

    shiftCurRange();

    return true;
}

bool LoadStockFromBroker::initRanges()
{
    if ( !loadRange.isValid() )
        return false;

    extraRange.setRange(loadRange.getEnd(), -14 * 24 * 3600 - loadRange.toSec());

    qint64 maxLoadRange = getMaxLoadInterval(stockKey.interval());
    curRange.setRange(loadRange.getBegin(), maxLoadRange);
    curRange.constrain(extraRange);

    return true;
}

bool LoadStockFromBroker::checkRange()
{
    if (curRange.getBegin() <= loadRange.getBegin()) {
        //Згружается extraRange, как только получим достаточно свечей, выходим!
        if (candles.size() >= minCandles)
            return false;
    }

    return curRange.toSec() >= stockKey.intervalToSec();
}

bool LoadStockFromBroker::loadExtraRange()
{
    if (curRange.getBegin() <= extraRange.getBegin())
        return false;   //выходим, потому что подинтервал уже загружен

    if (candles.size() >= minCandles)
        return false;

    long candleInterval = stockKey.intervalToSec();
    qint64 maxLoadRange = getMaxLoadInterval(stockKey.interval());
    curRange.setRange(loadRange.getBegin().addSecs(-candleInterval), -maxLoadRange);

    return curRange.isValid();
}

void LoadStockFromBroker::shiftCurRange()
{
    qint64 maxLoadRange = getMaxLoadInterval(stockKey.interval()) + stockKey.intervalToSec();

    if (curRange.getBegin() < loadRange.getBegin())
        maxLoadRange *= -1; //Началась загрузка extraRange, поэтому двигаемся в обратном направлении до границы extraRange

    curRange.displace(maxLoadRange, maxLoadRange);
    curRange.constrain(extraRange);
}

void LoadStockFromBroker::finishTask()
{
    Glo.broker->mutex.unlock();

    if ( !candles.empty() && !isStopRequested ) {
        logInfo << QString("TaskLoadStocksFromBroker;finishTask();loaded;%1;candles;%2;%3")
                   .arg(candles.size()).arg(candles.front().dateTime.toString(), candles.back().dateTime.toString());

        removeIncompleteCandle(candles);

        Candles newCandles;
        newCandles = Glo.stocks->insertCandles(stockKey, candles);
        DB::StocksQuery::placeCandles(Glo.dataBase, stockKey, newCandles);
    }
    emit finished();
}

void LoadStockFromBroker::onResponse(QByteArray answer)
{
    QJsonDocument doc = QJsonDocument::fromJson(answer);
    QJsonObject root(doc.object());

    QString status = root.value("status").toString();
    if (status.toLower() != "ok") {
        logWarning << QString(answer);
        finishTask();
        return;
    }

    //https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/market/get_market_candles
    QJsonObject payload(root.value("payload").toObject());

    try {
        StockKey recievedKey;
        //payload содержит ключ акции
        recievedKey.fromJson(payload);   //тоже кидается исключениями
        if(recievedKey != stockKey) {
            QString errorCode = QString("TaskLoadStockFromBroker;onResponse();recievedKey(%1) != key(%2);answer=%3")
                    .arg(recievedKey.keyToString(), stockKey.keyToString(), answer);
            throw std::logic_error(errorCode.toUtf8().data());
        }
    }  catch (std::exception &error) {
        logCritical << error.what();
        finishTask();
        return;
    }

    QJsonArray candlesArray = payload.value("candles").toArray();

    candles.reserve(candles.size() + candlesArray.size());
    for (const auto &it: candlesArray) {
        Candle candle;
        candle.fromJson(it.toObject());
        candles.push_back( std::move(candle) );
    }

    if (!sendRequest())
        finishTask();
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
