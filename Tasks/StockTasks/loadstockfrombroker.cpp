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

    if (_stock->size() > 0) {
        _stock->sort();
        _stock->removeIncompleteCandle();
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

    auto pushBack = [&](const auto &it){ _stock->insertCandle(_stock->end(), Data::Candle::fromJson(it.toObject())); };
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

}
