#include <QThread>

#include "candlesdata.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"

namespace Plotter {


CandlesData::CandlesData()
    : _bearPen   (Glo.conf->getValue("ChartPlotter/CandleItem/bearPen", QColor(235, 77, 92))),
      _bullPen   (Glo.conf->getValue("ChartPlotter/CandleItem/bullPen", QColor(83, 185, 135))),
      _bearBrush (Glo.conf->getValue("ChartPlotter/CandleItem/bearBrush", QColor(235, 77, 92))),
      _bullBrush (Glo.conf->getValue("ChartPlotter/CandleItem/bullBrush", QColor(83, 185, 135))),
      _candles(this), _candlesPool(this)
{
    //Когда candlesPool создает новые свечи, сигнализируем об этом!
    connect(&_candlesPool, SIGNAL(newItems(CandlesPool::PairRange)), this, SIGNAL(newItems(CandlesPool::PairRange)));
}

CandlesData::~CandlesData()
{
    _candles.clear();
    _candlesPool.clear();
}

void CandlesData::update(const long long newIndex, const size_t candlesCount)
{
    if (_isDataRequested)
        return;     //Данные уже запрошены, ждем!

    if (_candles.empty()) {      //если список свечей пуст, запрашиваем весь интервал
        requestCandles(Data::Range(QDateTime(), QDateTime::currentDateTime()), candlesCount * 2);
        return;
    }

    ///@todo !!Загружать +50% свечей слева и +50% свечей справа! как только подходим к границе +/- 25%, начинаем дозагрузку!
    ///@todo !!Ограничить запросы на загрузка вперед, когда настоящее время

    //Вычисляем смещение отображаемого индекса свечи и число свечей, которые нужно будет запросить
    long long indexOffset = newIndex - _candles.begin()->getIndex();
    long long requiredCount = abs(indexOffset) + candlesCount - _candles.size();
    if (indexOffset == 0 && requiredCount == 0)
        return; //все ок, действий не требуется

    auto setUnVisible = [](auto &it){it.clear();};

    if (indexOffset == 0) {             //Первая отображаемая свеча остается на месте
        if (requiredCount < 0) {
            auto splitItem = _candles.pop_back(-requiredCount);
            std::for_each(splitItem.first, splitItem.second, setUnVisible);
            _candlesPool.push_back(_candles, splitItem);
        } else {    //requiredCount > 0
            auto lastCandleTime = _candles.getBack()->getCandle()->dateTime();
            Data::Range requiredRange(_stockKey.nextCandleTime(lastCandleTime), QDateTime());
            requestCandles(requiredRange, requiredCount);
        }
    } else if (indexOffset < 0) {       //Первая отображаемая свеча смещается влево
        /* Возможен вариант firstCandle < firstDisplayedCandle && firstCandle + candlesCount > firstDisplayedCandle + displayedCandlesCount
         * Этот вариант означает, что свечи нужно добавить в начало и в конец, дабы не усложнять логику, этот вариант будет проигнорирован
         * И свечи добавим только в начале, после добавления повторный вызов CandlesSeries::updateData(),
         * приведет в ветку if (firstCandle == firstDisplayedCandle) && difCount > 0, и недостающие свечи будут догружены */
        auto splitItem = _candles.pop_back(-indexOffset);
        std::for_each(splitItem.first, splitItem.second, setUnVisible);
        _candlesPool.push_back(_candles, splitItem);

        Data::Range requiredRange(QDateTime(), _candles.begin()->getCandle()->dateTime());
        requestCandles(requiredRange, requiredCount);
    } else { //indexOffset > 0          //Первая отображаемая свеча смещается вправо
        if (requiredCount <= 0) {
            //этот вариант возможен при приближении скролом, интекс смещается вправо, но и количество свечей уменьшается
            auto splitItem = _candles.pop_front(indexOffset);
            std::for_each(splitItem.first, splitItem.second, setUnVisible);
            _candlesPool.push_back(_candles, splitItem);
        } else {
            auto lastCandleTime = _candles.getBack()->getCandle()->dateTime();
            Data::Range requiredRange(_stockKey.nextCandleTime(lastCandleTime), QDateTime());
            requestCandles(requiredRange, requiredCount);
        }
    }
}

void CandlesData::waitForRequestData() const
{
    while (_isDataRequested)
        QThread::msleep(1);  ///@note QThread::msleep(1) может привести к зависанию при очистке CandlesSeries
}

void CandlesData::setStockKey(const Data::StockKey &key)
{
    _stockKey = key;
}

const Data::StockKey &CandlesData::getStockKey() const
{
    return _stockKey;
}

CandleItem *CandlesData::operator [](const long long index)
{
    auto isEqualIndex = [index] (auto &item){ return item.getIndex() == index; };
    auto it = std::find_if(_candles.begin(), _candles.end(), isEqualIndex);
    if (it != _candles.end())
        return &(*it);
    return nullptr;
}

void CandlesData::clear()
{
    //Очищаем список свечей
    _candlesPool.push_back(_candles, _candles.pop_front(_candles.size()) );
}

bool CandlesData::empty() const
{
    return _candles.empty();
}

CandlesPool::Iterator CandlesData::begin()
{
    return _candles.begin();
}

CandlesPool::Iterator CandlesData::end()
{
    return _candles.end();
}

void CandlesData::requestCandles(const Data::Range &range, const size_t requiredCount)
{
    Task::InterfaceWrapper<Data::Range> inData = range;
    auto *command = NEW_TASK <Task::GetStock> (&inData, _stockKey, requiredCount);
    connect(command, &Task::GetStock::finished, this, &CandlesData::recieveCandles);
    _isDataRequested = true;
}

void CandlesData::recieveCandles()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(sender());
    Task::InterfaceWrapper<Data::StockView> stock = task->getResult();

    if (stock->size() == 0) {
        logCritical << "CandlesData::recieveCandles;recieved empty stock!";
        _isDataRequested = false;
        return;
    }

    auto lastRecievedCandleTime = stock->rbegin()->dateTime();
    if (!_candles.empty() && lastRecievedCandleTime == _candles.begin()->getCandle()->dateTime())
        stock->setEnd(_stockKey.prevCandleTime(stock->range().end()));
        //const_cast<Data::StockReference<QReadLocker>*>(stock.operator->())->setEnd(_stockKey.prevCandleTime(stock->getRange().end()));

    //Получаем stock->size() элементов для вставки из unused candles pool'а
    auto newItems = _candlesPool.pop_front(stock->size());

    //Определяем место вставки, в начало или в конец списка
    if(_candles.empty() || lastRecievedCandleTime <= _candles.begin()->getCandle()->dateTime()) {
        //Новые свечи левее существующих
        long long index = _candles.empty() ? -(stock->size() - 1) : _candles.begin()->getIndex() - stock->size();
        _candles.push_front(_candlesPool, newItems);
        auto curItem = _candles.begin();
        for (auto it = stock->begin(); it != stock->end(); ++it, ++curItem) {
            ///@todo !!отладить перемещение вправо, удалить CandleItem *cnalde
            CandleItem *cnalde; //temp - удалить! сделано для того, что бы заглянуть в list::iterator
            cnalde = &(*curItem);
            cnalde->set(index++, &(*it));
        }
        //std::for_each(stock->rbegin(), stock->rend(), insertCalnde);
    } else {
        //Новые свечи правее существующих
        auto curItem = _candles.end();
        --curItem;
        long long index = curItem->getIndex() + 1;
        _candles.push_back(_candlesPool, newItems);
        for (auto it = stock->begin(); it != stock->end(); ++it) {
            ++curItem;
            curItem->set(index++, &(*it));
        }
    }
    _isDataRequested = false;
    emit updateItems();
}

}
