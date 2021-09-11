#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "loadstockfrombroker.h"

#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromBroker::LoadStockFromBroker(const Data::StockKey &stockKey)
    : IBaseTask("LoadStockFromBroker")
{
    stock->key = stockKey;
}

LoadStockFromBroker::~LoadStockFromBroker()
{
}

void LoadStockFromBroker::setData(SharedInterface &inputData)
{
    range = inputData;
    if (!range->isValid())
        logCritical << "LoadStockFromBroker::setData:;invalid loadRange!";
}

SharedInterface &LoadStockFromBroker::getResult()
{
    return &stock;
}

void LoadStockFromBroker::exec()
{
    Glo.broker->mutex.lock();

    if (!range->isValid()) {
        logCritical << "LoadStockFromBroker::exec:;invalid loadRange!";
        finishTask();
        return;
    }

    //Формируем первый подинтервал, длинна которого не более чем максимально доступный интервал загрузки
    //для данной длительности свечи (интервалы определяет брокер)
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(stock->key.interval());
    subRange.setRange(range->getEnd(), -maxLoadRange);
    subRange.constrain(range);

    connect(Glo.broker.data(), &Broker::Api::getResopnse, this, &LoadStockFromBroker::onResponse);

    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::sendRequest()
{
    if (isStopRequested)
        return false;

    if (!Glo.broker->loadCandles(stock->key, subRange))
        return false;

    return true;
}

void LoadStockFromBroker::onResponse(QByteArray answer)
{
    //В соответствии с принципом ленивых вычислений функции будут выполнены по порядку readCandles, getNextLoadRange,
    //sendRequest если хотя бы одна из них false, последующие функцие выполнятся не будут, и будет вызов finishTask()
    if (!readCandles(answer) || !getNextLoadRange() || !sendRequest())
        finishTask();
}

bool LoadStockFromBroker::getNextLoadRange()
{
    long candleInterval = stock->key.intervalToSec();
    if ( subRange.getBegin() < range->getBegin().addSecs(candleInterval) )
        return false;

    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(stock->key.interval()) + candleInterval;
    subRange.addSecs(-maxLoadRange);
    subRange.constrain(range);
    return true;
}

void LoadStockFromBroker::finishTask()
{
    Glo.broker->mutex.unlock();

    if (!stock->candles.empty()) {
        std::sort(stock->candles.begin(), stock->candles.end());
        removeIncompleteCandle();
    }

    emit finished();
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

    stock->candles.reserve(stock->candles.size() + candlesArray.size());
    for (const auto &it: candlesArray)
        stock->candles.emplace_back( Data::Candle::fromJson(it.toObject()) );

    return true;
}

bool LoadStockFromBroker::checkStockKey(const QJsonObject &payload)
{
    try {  //StockKey::fromJson кидается исключениями, в случае не возможности определить интервал свечи
        Data::StockKey recievedKey;
        recievedKey.fromJson(payload);   //payload содержит ключ акции
        if(recievedKey != stock->key) {
            QString errorCode = QString("LoadStockFromBroker;checkStockKey();recievedKey!=stockKey:;%1;%2")
                    .arg(recievedKey.keyToString(), stock->key.keyToString());
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
    Data::Candle &lastCandle = stock->candles.back();

    long candleDuration = stock->key.intervalToSec();
    QDateTime timeCandleComplite = lastCandle.dateTime.addSecs(candleDuration);

    if (timeCandleComplite > range->getEnd())
        stock->candles.pop_back();
}

}
