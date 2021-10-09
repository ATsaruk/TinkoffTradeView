#include <QThread>

#include "candlesdata.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"

namespace Plotter {


CandlesData::CandlesData()
    : candles(this), candlesPool(this)
{
    bearPen   = Glo.conf->getValue("ChartPlotter/CandleItem/bearPen", QColor(235, 77, 92));
    bearBrush = Glo.conf->getValue("ChartPlotter/CandleItem/bearBrush", QColor(235, 77, 92));
    bullPen   = Glo.conf->getValue("ChartPlotter/CandleItem/bullPen", QColor(83, 185, 135));
    bullBrush = Glo.conf->getValue("ChartPlotter/CandleItem/bullBrush", QColor(83, 185, 135));

    //Когда candlesPool создает новые свечи, сигнализируем об этом!
    connect(&candlesPool, SIGNAL(newItems(CandlesPool::PairRange)), this, SIGNAL(newItems(CandlesPool::PairRange)));
}

CandlesData::~CandlesData()
{
    candles.clear();
    candlesPool.clear();
}

void CandlesData::update(const long long newIndex, const size_t candlesCount)
{
    if (isDataRequested)
        return;     //Данные уже запрошены, ждем!

    long candleLenght = stockKey.candleLenght();
    if (candles.empty()) {      //если список свечей пуст, запрашиваем весь интервал
        requestCandles(Data::Range(QDateTime::currentDateTime(), -candlesCount * candleLenght), candlesCount);
        return;
    }

    //Вычисляем смещение отображаемого индекса свечи и число свечей, которые нужно будет запросить
    long long indexOffset = newIndex - candles.begin()->getIndex();
    long long requiredCount = abs(indexOffset) + candlesCount - candles.size();
    if (indexOffset == 0 && requiredCount == 0)
        return; //все ок, действий не требуется

    auto setUnVisible = [](auto &it){it.clear();};

    if (indexOffset == 0) {             //Первая отображаемая свеча остается на месте
        if (requiredCount < 0) {
            auto splitItem = candles.pop_back(-requiredCount);
            std::for_each(splitItem.first, splitItem.second, setUnVisible);
            candlesPool.push_back(candles, splitItem);
        } else {    //requiredCount > 0
            auto lastCandleTime = candles.getBack()->getCandle()->dateTime();
            Data::Range requiredRange(stockKey.nextCandleTime(lastCandleTime), requiredCount * candleLenght);
            requestCandles(requiredRange, requiredCount);
        }
    } else if (indexOffset < 0) {       //Первая отображаемая свеча смещается влево
        /* Возможен вариант firstCandle < firstDisplayedCandle && firstCandle + candlesCount > firstDisplayedCandle + displayedCandlesCount
         * Этот вариант означает, что свечи нужно добавить в начало и в конец, дабы не усложнять логику, этот вариант будет проигнорирован
         * И свечи добавим только в начале, после добавления повторный вызов CandlesSeries::updateData(),
         * приведет в ветку if (firstCandle == firstDisplayedCandle) && difCount > 0, и недостающие свечи будут догружены */
        auto splitItem = candles.pop_back(-indexOffset);
        std::for_each(splitItem.first, splitItem.second, setUnVisible);
        candlesPool.push_back(candles, splitItem);

        Data::Range requiredRange(candles.begin()->getCandle()->dateTime(), -requiredCount * candleLenght);
        requestCandles(requiredRange, requiredCount);
    } else { //indexOffset > 0          //Первая отображаемая свеча смещается вправо
        if (requiredCount <= 0) {
            //этот вариант возможен при приближении скролом, интекс смещается вправо, но и количество свечей уменьшается
            auto splitItem = candles.pop_front(indexOffset);
            std::for_each(splitItem.first, splitItem.second, setUnVisible);
            candlesPool.push_back(candles, splitItem);
        } else {
            auto lastCandleTime = candles.getBack()->getCandle()->dateTime();
            Data::Range requiredRange(stockKey.nextCandleTime(lastCandleTime), requiredCount * candleLenght);
            requestCandles(requiredRange, requiredCount);
        }
    }
}

void CandlesData::waitForRequestData() const
{
    while (isDataRequested)
        QThread::msleep(1);  ///@note QThread::msleep(1) может привести к зависанию при очистке CandlesSeries
}

void CandlesData::setStockKey(const Data::StockKey &key)
{
    stockKey = key;
}

const Data::StockKey &CandlesData::getStockKey() const
{
    return stockKey;
}

CandleItem *CandlesData::operator [](const long long index)
{
    auto isEqualIndex = [index] (auto &item){ return item.getIndex() == index; };
    auto it = std::find_if(candles.begin(), candles.end(), isEqualIndex);
    if (it != candles.end())
        return &(*it);
    return nullptr;
}

void CandlesData::clear()
{
    //Очищаем список свечей
    candlesPool.push_back(candles, candles.pop_front(candles.size()) );
}

bool CandlesData::empty() const
{
    return candles.empty();
}

CandlesPool::Iterator CandlesData::begin()
{
    return candles.begin();
}

CandlesPool::Iterator CandlesData::end()
{
    return candles.end();
}

void CandlesData::requestCandles(const Data::Range &range, const size_t requiredCount)
{
    Task::InterfaceWrapper<Data::Range> inData = range;
    auto *command = NEW_TASK <Task::GetStock> (&inData, stockKey, requiredCount);
    connect(command, &Task::GetStock::finished, this, &CandlesData::recieveCandles);
    isDataRequested = true;
}

void CandlesData::recieveCandles()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(sender());
    Task::InterfaceWrapper<Task::GetStock::SharedStockVewRef> stock = task->getResult();

    if (stock->empty()) {
        logCritical << "CandlesData::recieveCandles;recieved empty stock!";
        isDataRequested = false;
        return;
    }

    auto lastRecievedCandleTime = stock->rbegin()->dateTime();
    if (!candles.empty() && lastRecievedCandleTime == candles.begin()->getCandle()->dateTime())
        const_cast<Data::StockReference<QReadLocker>*>(stock.operator->())->setEnd(stockKey.prevCandleTime(stock->getRange().getEnd()));

    //Получаем stock->size() элементов для вставки из unused candles pool'а
    auto newItems = candlesPool.pop_front(stock->size());

    ///@todo !!отладить перемещение вправо, удалить CandleItem *cnalde
    CandleItem *cnalde; //temp - удалить! сделано для того, что бы заглянуть в list::iterator

    //Определяем место вставки, в начало или в конец списка
    if(candles.empty() || lastRecievedCandleTime <= candles.begin()->getCandle()->dateTime()) {
        //Новые свечи левее существующих
        long long index = candles.empty() ? -(stock->size() - 1) : candles.begin()->getIndex() - stock->size();
        candles.push_front(candlesPool, newItems);
        auto curItem = candles.begin();
        for (auto it = stock->begin(); it != stock->end(); ++it, ++curItem) {
            cnalde = &(*curItem);
            cnalde->set(index++, &(*it));
        }
        //std::for_each(stock->rbegin(), stock->rend(), insertCalnde);
    } else {
        //Новые свечи правее существующих
        auto curItem = candles.end();
        --curItem;
        long long index = curItem->getIndex() + 1;
        candles.push_back(candlesPool, newItems);
        for (auto it = stock->begin(); it != stock->end(); ++it) {
            ++curItem;
            curItem->set(index++, &(*it));
        }
    }
    isDataRequested = false;
    emit updateItems();
}

}
