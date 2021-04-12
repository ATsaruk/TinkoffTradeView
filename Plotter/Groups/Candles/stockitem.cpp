#include "stockitem.h"

#include "Core/global.h"
#include "Plotter/Axis/axis.h"
#include "Tasks/Commands/loadstock.h"

namespace Plotter {

StockItem::StockItem()
{
    isDataRequested = false;
    isDataChanged = false;
    autoPriceRange = true;
    beginCandle = candleItems.end();
    endCandle = candleItems.end();

    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", QVariant(10)).toUInt();
    drawWait = 3 * plotInterval / 4;
    //connect(Glo.stocks, &DataStocks::dataChanged, this, &ChartCandlesGroup::dataChanged);
}

StockItem::~StockItem()
{
    for (auto &it : candleItems)
        delete it.second;
}

void StockItem::repaint()
{
    if (drawMutex.tryLock(drawWait)) {
        if (isDataChanged) {
            addCandles();
            isDataChanged = false;
        }
        if (isScaled)
            updateScale();
    }
    drawMutex.unlock();
}

void StockItem::updateData()
{
    if (isDataRequested)
        return;

    long displayedCandlesCount = hAxis->getRange();
    long intervalSec = curStockKey.intervalToSec();

    Data::DateRange range;;
    if (candleItems.empty()) {
        range.setRange(QDateTime::currentDateTime(), -displayedCandlesCount * intervalSec * 2);
        loadData(range);
    } else {
        long offset = hAxis->getOffset();
        if (offset < candleItems.begin()->first) {
            range.setRange(candleItems.begin()->second->getData().dateTime, -displayedCandlesCount * intervalSec);
            loadData(range);
        }
    }
}

bool StockItem::clear()
{
    //Если в текущий момент запрошены свечные данные, то пока мы их не обработаем, мы не может произвести очистку
    if (isDataRequested)
        return false;

    //Удаляем свечи
    for (auto &it : candleItems) {
        if (it.second->isVisible())
            this->removeFromGroup(it.second);
        delete it.second;
    }

    //Очищаем список свечей
    candleItems.clear();

    //Сбрасываем отображаемый интервал
    beginCandle = candleItems.end();
    endCandle = candleItems.end();

    return true;
}

void StockItem::updateScale()
{
    isScaled = false;

    if (candleItems.empty())
        return;

    updateData();

    scaleByXAxis();

    if (autoPriceRange)
        updatePriceRange();

    scaleByYAxis();

    emit changed();
}

/* Функция мастабирования оси Х
 * Определяет новые индексы начала и конца интервала отображения свечей
 * Свечи, которые стали невидны скрывает, новые свечи отображает
 * И обновляем масштаб всех видимых свечей
 */
void StockItem::scaleByXAxis()
{
    ///Определяем новый индекс начали интервала отображения свечей
    long firstDisplayedIntex = hAxis->getOffset();
    auto newBeginCandle = candleItems.find(firstDisplayedIntex);
    //Если элемент с индексом firstDisplayedIntex не найден, то будем отображать с первого элемента
    if (newBeginCandle == candleItems.end())
        newBeginCandle = candleItems.begin();

    ///Определяем новый индекс конца интервала отображения свечей
    // +1 т.к. чтобы отображать часть свечи, которая невлезла целиком на экран
    long lastDisplayedIndex = firstDisplayedIntex + hAxis->getRange() + 1;
    auto newEndCandle = candleItems.find(lastDisplayedIndex);

    setCandleVisible(newBeginCandle, beginCandle);
    beginCandle = newBeginCandle;

    setCandleVisible(endCandle, newEndCandle);
    endCandle = newEndCandle;

    //Обновляем масштаб и положение видимых свечей по оси oX
    qreal xScale = hAxis->getScale();
    for (auto it = newBeginCandle; it != newEndCandle; ++it) {
        it->second->setCandleHorizontalScale(xScale);
        it->second->setX(it->first * xScale);
    }

    //Задаем положение на экране всей QGraphicsItemGroup
    setX(-1 * firstDisplayedIntex * xScale);
}

void StockItem::scaleByYAxis()
{
    qreal yScale = vAxis->getScale();
    for (auto it = beginCandle; it != endCandle; ++it)
        it->second->setCandleVerticalScale(yScale);

    qreal yOffset = vAxis->getOffset();
    this->setY(yOffset * yScale);
}

/* Функция, которая скрывает или отображает свечи, в зависимости от переданного интервала
 * Если первичный ключ first_iterator'а МЕНЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем отображать свечи из интервала [first_iterator..second_iterator]
 * Если первичный ключ first_iterator'а БОЛЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем скрывать свечи из интервала [second_iterator..first_iterator] (т.к. у нас не reverse_iterator)
 */
void StockItem::setCandleVisible(const std::map<long, CandleItem*>::iterator &first_iterator, const std::map<long, CandleItem*>::iterator &second_iterator)
{
    //Принцип определения параметра visible описан выше
    bool visible  = first_iterator->first < second_iterator->first;
    //Определяем порядок движения (от меньшего значения к большему)
    auto begin = visible ? first_iterator : second_iterator;
    auto end   = visible ? second_iterator : first_iterator;

    qreal xScale = hAxis->getScale();
    for (auto it = begin; it != end; ++it) {
        it->second->setVisible(visible);

        if (visible) { //Для видимых свечей
            //Обновляем масштаб свечи по оси оY
            it->second->setCandleVerticalScale(xScale);

            //Добавляем в группу QGraphicsItemGroup, что бы элемент обрабатывался
            this->addToGroup(it->second);
        }
        else {  //Для скрываемых свечей
            //Удаляем из группы QGraphicsItemGroup, что бы элемент больше не обрабатывался
            this->removeFromGroup(it->second);
        }
    }
}

void StockItem::updatePriceRange()
{
    qreal minPrice = beginCandle->second->getData().low;
    qreal maxPrice = beginCandle->second->getData().high;

    for (auto it = beginCandle; it != endCandle; ++it) {   //проверять только свечи на экране!
        const Data::Candle& data = it->second->getData();
        if (minPrice > data.low)
            minPrice = data.low;
        if (maxPrice < data.high)
            maxPrice = data.high;
    }

    qreal priceRange = maxPrice - minPrice;
    qreal newRange = priceRange + priceRange * 0.2; // + 20% что бы на графике сверху и снизу быз "зазор"
    qreal newOffset = minPrice - priceRange * 0.1;  // - 10% половина от расширения диапазона
    vAxis->setDataRange(newRange);
    vAxis->setDataOffset(newOffset);

    //autoPriceRange = false;   //while settings autoPriceRange off is absent
}

const QDateTime StockItem::getDateByIndex(const long index)
{
    if (candleItems.find(index) != candleItems.end())
        return candleItems[index]->getData().dateTime;
    return QDateTime();
}

void StockItem::loadData(const Data::DateRange &range)
{
    isDataRequested = true;
    uint candleCount = hAxis->getRange() / 3.;
    auto *command = NEW_TASK<Task::LoadStock>(curStockKey, range, candleCount);
    connect(command, &Task::IBaseTask::finished, this, &StockItem::dataChanged);
    //connect(command, &Task::IBaseTask::finished, this, &StockItem::loadTaskFinished);
}

//Добавление 1 свечи, поиск нового индекса
void StockItem::addCandle(const Data::Candle &candleData)
{
    long index = 0.;
    if (!candleItems.empty()) {
        if (candleItems.begin()->second->getData().dateTime > candleData.dateTime)
            index = candleItems.begin()->first - 1;
        if (candleItems.rbegin()->second->getData().dateTime < candleData.dateTime)
            index = candleItems.rbegin()->first + 1;
    }

    if(candleItems.find(index) != candleItems.end())
        return; //Свеча уже существует

    CandleItem *cndl = new CandleItem(candleData);
    cndl->setCandleHorizontalScale(hAxis->getScale());
    cndl->setX(index * hAxis->getScale());
    if (index >= hAxis->getOffset() &&
            index <= hAxis->getOffset() + hAxis->getRange() )
        //Добавляем новые объекты для отрисовки
        addToGroup(cndl);
    candleItems[index] = cndl;
}

//Слот загружает свечи после получения сигнала finished от CommandLoadStock
void StockItem::addCandles()
{
    bool isCandlesAdded = false;
    QReadLocker lock(&Glo.stocks->rwMutex);
    const Data::Candles &candlesData = Glo.stocks->getStock(curStockKey);
    for (auto it = candlesData.rbegin(); it != candlesData.rend(); ++it) {
        bool isExist = false;

        if (!candleItems.empty()) {
            QDateTime curBeginInterval = candleItems.begin()->second->getData().dateTime;
            QDateTime curEndInterval = candleItems.rbegin()->second->getData().dateTime;
            if (curBeginInterval <= it->dateTime && it->dateTime <= curEndInterval)
                isExist = true;
        }

        if (!isExist) {
            addCandle(*it);
            isCandlesAdded = true;
        }
    }

    if (isCandlesAdded) {
        isScaled = true;

        //Если beginCandle == candleItems.end() это означает, инициализация интервала для отрисовки
        if (beginCandle == candleItems.end())
            beginCandle = candleItems.begin();
    }

    isDataRequested = false;
}

void StockItem::dataChanged()
{
    addCandles();
    //isDataChanged = true;
}

void StockItem::loadTaskFinished()
{
    isDataRequested = false;
}

}
