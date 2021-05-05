#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "loadstockfrombroker.h"

#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromBroker::LoadStockFromBroker(const StockKey &stockKey)
    : IBaseTask("LoadStockFromBroker")
{
    stock().key = stockKey;
}

LoadStockFromBroker::~LoadStockFromBroker()
{
}


void LoadStockFromBroker::exec()
{
    Glo.broker->mutex.lock();

    assert(range().isValid() && "LoadStockFromBroker::setData: invalid loadRange!");

    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(stock().key.interval());
    curRange.setRange(range().getEnd(), -maxLoadRange);
    curRange.constrain(range());

    connect(Glo.broker, &Broker::Api::getResopnse, this, &LoadStockFromBroker::onResponse);

    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::sendRequest()
{
    if (isStopRequested)
        return false;

    if (!Glo.broker->loadCandles(stock().key, curRange))
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
    auto candleInterval = stock().key.time();
    if ( curRange.getBegin() < range().getBegin().addSecs(candleInterval) )
        return false;

    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(stock().key.interval()) + candleInterval;
    curRange.addSecs(-maxLoadRange);
    curRange.constrain(range());
    return true;
}

void LoadStockFromBroker::finishTask()
{
    Glo.broker->mutex.unlock();

    if (!stock().candles.empty()) {
        std::sort(stock().candles.begin(), stock().candles.end());
        removeIncompleteCandle();
    }

    emit finished();
}

void LoadStockFromBroker::setData(SharedInterface &inputData)
{
    range = inputData;
}

SharedInterface &LoadStockFromBroker::getResult()
{
    return *stock;
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

    stock().candles.reserve(stock().candles.size() + candlesArray.size());
    for (const auto &it: candlesArray)
        stock().candles.emplace_back( Candle::fromJson(it.toObject()) );

    return true;
}

bool LoadStockFromBroker::checkStockKey(const QJsonObject &payload)
{
    try {  //StockKey::fromJson кидается исключениями
        StockKey recievedKey;
        recievedKey.fromJson(payload);   //payload содержит ключ акции
        if(recievedKey != stock().key) {
            QString errorCode = QString("LoadStockFromBroker;checkStockKey();recievedKey!=stockKey:;%1;%2")
                    .arg(recievedKey.keyToString(), stock().key.keyToString());
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
void LoadStockFromBroker::removeIncompleteCandle()
{
    Candle &lastCandle = stock().candles.back();

    uint64_t candleDuration = stock().key.time();
    QDateTime timeCandleComplite = lastCandle.dateTime.addSecs(candleDuration);

    if (timeCandleComplite > range().getEnd())
        stock().candles.pop_back();
}

}
