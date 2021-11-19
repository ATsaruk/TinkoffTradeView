#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "loadstockfrombroker.h"

#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

constexpr long secInTwoWeek = 14 * 24 * 60 * 60;

LoadStockFromBroker::LoadStockFromBroker(const Data::StockKey &stockKey, const size_t minCandleCount)
    : IBaseTask("LoadStockFromBroker"),
      _extraRangeLoaded(false),
      _isForwardLoading(false),
      _minCandlesCount(minCandleCount),
      _maxTotalLoadTime(stockKey.candleLenght() * Glo.conf->getValue("Tinkoff/maxLoadCount", 2000)),
      _stock(stockKey)
{
    //_stock.create(stockKey);
    if (_maxTotalLoadTime < secInTwoWeek)
        _maxTotalLoadTime = secInTwoWeek;   //интервал должен быть не меньше 2 недель, что бы загрузка успешно прошла через новогодние каникулы
}

LoadStockFromBroker::~LoadStockFromBroker()
{
}

void LoadStockFromBroker::setData(SharedInterface &inputData)
{
    //InterfaceWrapper<Data::Range> inputRange = inputData;
    _loadRange = Data::Range(inputData->get<Data::Range>());   //создаем копию переданного интервала, т.к. нам возможно его нужно будет редактировать
}

SharedInterface &LoadStockFromBroker::getResult()
{
    return &_stock;
}

void LoadStockFromBroker::exec()
{
    //получаем доступ к брокеру
    Glo.broker->mutex.lock();
    connect(Glo.broker.data(), &Broker::IBroker::getResopnse, this, &LoadStockFromBroker::onResponse);

    if ( !initLoadingRange()    //подготавливаем подинтервал для загрузки
         || !sendRequest() ) {  //отправляем запрос на получение свечей
        finishTask();
    }
}

bool LoadStockFromBroker::initLoadingRange()
{
    if ( (_loadRange->isValid()) != (_minCandlesCount == 0) ) {
        logCritical << QString("LoadStockFromBroker::exec:;invalid input data!;%1;%2;%3")
                       .arg(_loadRange->start().toString(), _loadRange->end().toString())
                       .arg(_minCandlesCount);
        return false;
    }

    _isForwardLoading = _loadRange->isStartValid();

    //Ограничивам окончание загрузки последней завершенной свечей
    if ( auto timeEndLastFullCandle = _stock->key().startCandleTime(QDateTime::currentDateTime());
         _loadRange->isEndNull() || _loadRange->end() > timeEndLastFullCandle )
        _loadRange->end() = timeEndLastFullCandle;

    //Формируем подинтервал для загрузки, длинна которого не более чем максимально доступный интервал загрузки для
    //данной длительности свечи (интервалы определяет брокер)
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(_stock->key().interval());
    if (_isForwardLoading)
        _subRange.setRange(_loadRange->start(), maxLoadRange);
    else
        _subRange.setRange(_loadRange->end(), -maxLoadRange);

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
    return !isLoadFinished();   //если загрузка не завершена, возращаем true (успешно перешли к след. подинтервалу загрузки)
}

bool LoadStockFromBroker::isLoadFinished()
{
    if (_subRange.toSec() < _stock->key().candleLenght())
        return true;        //запрашиваемый интервал загружен!

    if (_minCandlesCount > 0) {
        //Загрузка производится по количеству свечей
        if (_stock->size() >= _minCandlesCount)
            return true;    //загружено достаточно!

        /* Ограничиваем максимальную длинну загрузки, но только при загрузке по количеству, когда происходи загрузка
         * заданного диапазона, то загружаем весь заданный диапазон, это важно! */
        size_t curTotalLoadTime = _isForwardLoading
                ? _loadRange->start().secsTo(_subRange.start())
                : _subRange.end().secsTo(_loadRange->end());

        return curTotalLoadTime > _maxTotalLoadTime;
    }

    return false;
}

void LoadStockFromBroker::finishTask()
{
    disconnect(Glo.broker.data(), &Broker::IBroker::getResopnse, this, &LoadStockFromBroker::onResponse);
    Glo.broker->mutex.unlock();

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
