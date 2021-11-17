#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "loadstockfrombroker.h"

#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromBroker::LoadStockFromBroker(const Data::StockKey &stockKey, const size_t minCandleCount)
    : IBaseTask("LoadStockFromBroker"),
      _extraRangeLoaded(false),
      _isForwardLoading(false),
      _minCandlesCount(minCandleCount),
      _stock(stockKey)
{
    //_stock.create();
    //_stock->setStockKey(stockKey);
}

LoadStockFromBroker::~LoadStockFromBroker()
{
}

void LoadStockFromBroker::setData(SharedInterface &inputData)
{
    _loadRange = inputData;
}

SharedInterface &LoadStockFromBroker::getResult()
{
    return &_stock;
}

void LoadStockFromBroker::exec()
{
    //подготавливаем подинтервал для загрузки
    if (!initLoadingRange()) {
        finishTask();
        return;
    }

    //получаем доступ к брокеру
    Glo.broker->mutex.lock();
    connect(Glo.broker.data(), &Broker::IBroker::getResopnse, this, &LoadStockFromBroker::onResponse);

    //отправляем запрос на получение свечей
    if (!sendRequest())
        finishTask();
}

bool LoadStockFromBroker::initLoadingRange()
{
    if ( (_loadRange->isValid()) != (_minCandlesCount > 0) ) {
        logCritical << QString("LoadStockFromBroker::exec:;invalid input data!;%1;%2;%3")
                       .arg(_loadRange->begin().toString(), _loadRange->end().toString())
                       .arg(_minCandlesCount);
        return false;
    }

    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(_stock->key().interval());
    auto endLoading = _loadRange->isEndValid() ? _loadRange->end() : QDateTime::currentDateTime();

    //Формируем подинтервал для загрузки, длинна которого не более чем максимально доступный интервал загрузки для
    //данной длительности свечи (интервалы определяет брокер)
    _isForwardLoading = _loadRange->isBeginValid();
    if (_isForwardLoading)
        _subRange.setRange(_loadRange->begin(), maxLoadRange);
    else
        _subRange.setRange(endLoading, -maxLoadRange);

    _subRange.constrain(_loadRange);

    return _subRange.toSec() >= _stock->key().candleLenght();
}

bool LoadStockFromBroker::sendRequest()
{
    if (isStopRequested)
        return false;

    return Glo.broker->loadCandles(_stock->key(), _subRange);
}

void LoadStockFromBroker::onResponse(const QByteArray &answer)
{
    //В соответствии с принципом ленивых вычислений функции будут выполнены по порядку readCandles, getNextLoadRange,
    //sendRequest если хотя бы одна из них false, последующие функцие выполнятся не будут, и будет вызов finishTask()
    if ( !readCandles(answer)
         || !goNextLoadRange()
         || !sendRequest() )
        finishTask();
}

bool LoadStockFromBroker::goNextLoadRange()
{
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(_stock->key().interval());
    _subRange.shift( (_isForwardLoading ? 1 : -1) * maxLoadRange );
    _subRange.constrain(_loadRange);
    return isLoadFinished();
}

bool LoadStockFromBroker::isLoadFinished()
{
    ///@todo !!!ограничить максимальный интервал загрузки Config->parametr
    ///@todo !!!реализовать определение конца загрузки + загрузку доп 2 недельного интервала

    return _subRange.toSec() >= _stock->key().candleLenght();
}

void LoadStockFromBroker::finishTask()
{
    disconnect(Glo.broker.data(), &Broker::IBroker::getResopnse, this, &LoadStockFromBroker::onResponse);
    Glo.broker->mutex.unlock();

    if (_stock->size() > 0)
        _stock->removeIncompleteCandle();

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

    auto candlesArray = payload.value("candles").toArray().toVariantList(); //преобразуется в QVariantList т.к. у него есть реверс итератор!
    //тут нужен будет реверс!!!
    if (_isForwardLoading) {
        auto pushBack = [&](const auto &it) { _stock->insertCandle(_stock->end(), Data::Candle::fromJson(it.toJsonObject())); };
        std::for_each(candlesArray.begin(), candlesArray.end(), pushBack);
    } else {
        auto pushFront = [&](const auto &it) { _stock->insertCandle(_stock->begin(), Data::Candle::fromJson(it.toJsonObject())); };
        std::for_each(candlesArray.rbegin(), candlesArray.rend(), pushFront);
    }

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
