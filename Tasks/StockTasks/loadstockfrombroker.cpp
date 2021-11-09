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
    _stock.create();
    _stock->setStockKey(stockKey);
}

LoadStockFromBroker::~LoadStockFromBroker()
{
}

void LoadStockFromBroker::setData(SharedInterface &inputData)
{
    _range = inputData;
    if (!_range->isValid())
        logCritical << "LoadStockFromBroker::setData:;invalid loadRange!";
}

SharedInterface &LoadStockFromBroker::getResult()
{
    return &_stock;
}

void LoadStockFromBroker::exec()
{
    Glo.broker->mutex.lock();

    if (!_range->isValid()) {
        logCritical << "LoadStockFromBroker::exec:;invalid loadRange!";
        finishTask();
        return;
    }

    //Формируем первый подинтервал, длинна которого не более чем максимально доступный интервал загрузки
    //для данной длительности свечи (интервалы определяет брокер)
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(_stock->key().interval());
    _subRange.setRange(_range->end(), -maxLoadRange);
    _subRange.constrain(_range);

    connect(Glo.broker.data(), &Broker::IBroker::getResopnse, this, &LoadStockFromBroker::onResponse);

    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::sendRequest()
{
    if (isStopRequested || !_subRange.isValid())
        return false;

    if (!Glo.broker->loadCandles(_stock->key(), _subRange))
        return false;

    return true;
}

void LoadStockFromBroker::onResponse(const QByteArray &answer)
{
    //В соответствии с принципом ленивых вычислений функции будут выполнены по порядку readCandles, getNextLoadRange,
    //sendRequest если хотя бы одна из них false, последующие функцие выполнятся не будут, и будет вызов finishTask()
    if (!readCandles(answer) || !getNextLoadRange() || !sendRequest())
        finishTask();
}

bool LoadStockFromBroker::getNextLoadRange()
{
    if ( _subRange.begin() <= _stock->key().prevCandleTime(_range->begin()) )
        return false;

    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(_stock->key().interval());
    _subRange.shift(-maxLoadRange);
    _subRange.constrain(_range);
    return true;
}

void LoadStockFromBroker::finishTask()
{
    Glo.broker->mutex.unlock();

    auto &candles = _stock->getCandles();
    if (!candles.empty()) {
        std::sort(candles.begin(), candles.end());
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

    auto &candles = _stock->getCandles();
    auto pushBack = [&candles](const auto &it){ candles.push_back(Data::Candle::fromJson(it.toObject())); };
    std::for_each(candlesArray.begin(), candlesArray.end(), pushBack);

    return true;
}

bool LoadStockFromBroker::checkStockKey(const QJsonObject &payload)
{
    try {  //StockKey::fromJson кидается исключениями, в случае не возможности определить интервал свечи
        Data::StockKey recievedKey;
        recievedKey.fromJson(payload);   //payload содержит ключ акции
        if(recievedKey != _stock->key()) {
            QString errorCode = QString("LoadStockFromBroker;checkStockKey();recievedKey!=stockKey:;%1;%2")
                    .arg(recievedKey.keyToString(), _stock->key().keyToString());
            throw std::logic_error(errorCode.toUtf8().data());
        }
    }  catch (std::exception &except) {
        logCritical << QString("catch std::exception in LoadStockFromBroker::checkStockKey():;") + QString(except.what());
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
    auto &candles = _stock->getCandles();
    Data::Candle &lastCandle = candles.back();

    QDateTime lastCompleteCandle = _stock->key().prevCandleTime(QDateTime::currentDateTime());
    if (lastCandle.dateTime() >= lastCompleteCandle)
        candles.pop_back();
}

}
