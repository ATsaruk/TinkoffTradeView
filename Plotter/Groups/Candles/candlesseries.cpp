#include "candlesseries.h"

#include "Core/globals.h"
#include "Plotter/Axis/axis.h"
#include "Tasks/StockTasks/loadstock.h"

#include "DataBase/Query/stocksquery.h"


#include <QThread>


namespace Plotter {

CandlesSeries::CandlesSeries(const Data::StockKey &stockKey)
{
    curStockKey = stockKey;

    beginCandle = candleItems.end();
    endCandle = candleItems.end();

    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    drawWait = plotInterval * 3 / 4;
}

CandlesSeries::~CandlesSeries()
{
    for (auto &it : candleItems)
        delete it.second;
}

void CandlesSeries::repaint()
{
    if (!isChanged)
        return;

    if (!drawMutex.tryLock(drawWait))
        return;

    assert(!candleItems.empty() && "CandlesSeries::updateScale() candleItems can't be empty with isChanged == true!");

    updateData();

    scaleByXAxis();

    if (autoPriceRange)
        updatePriceRange();

    scaleByYAxis();

    emit changed();
    isChanged = false;

    drawMutex.unlock();
}

const Data::StockKey &CandlesSeries::getStockKey()
{
    return curStockKey;
}

void CandlesSeries::updateData()
{
    if (isDataRequested)
        return;

    long displayedCandlesCount = hAxis->getRange();
    long intervalSec = curStockKey.time();

    Data::Range range;;
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

void CandlesSeries::clear()
{
    //Если в текущий момент запрошены свечные данные, то пока мы их не обработаем, мы не может произвести очистку
    while (isDataRequested)
        QThread::msleep(1);  ///@todo проверить не приведет ли к зависанию!

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
}

/* Функция мастабирования оси Х
 * Определяет новые индексы начала и конца интервала отображения свечей
 * Свечи, которые стали невидны скрывает, новые свечи отображает
 * И обновляем масштаб всех видимых свечей
 */
void CandlesSeries::scaleByXAxis()
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

void CandlesSeries::scaleByYAxis()
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
void CandlesSeries::setCandleVisible(const std::map<long, CandleItem*>::iterator &first_iterator, const std::map<long, CandleItem*>::iterator &second_iterator)
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

void CandlesSeries::updatePriceRange()
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

const QDateTime CandlesSeries::getDateByIndex(const long index)
{
    if (candleItems.find(index) != candleItems.end())
        return candleItems[index]->getData().dateTime;
    return QDateTime();
}

void CandlesSeries::loadData(const Data::Range &loadRange)
{
    isDataRequested = true;

    uint minCandles = hAxis->getRange() / 3.;
    Task::InterfaceWrapper<Data::Range> range = loadRange;
    auto *command = TaskManager->createTask<Task::LoadStock>(*range, curStockKey, minCandles);
    command->connect(this, SLOT(loadCandlesFinished()));
}

//Добавление 1 свечи, поиск нового индекса
void CandlesSeries::addCandle(Data::Candle &&candleData)
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

    CandleItem *cndl = new CandleItem(std::forward<Data::Candle>(candleData));
    cndl->setCandleHorizontalScale(hAxis->getScale());
    cndl->setX(index * hAxis->getScale());
    if (index >= hAxis->getOffset() &&
            index <= hAxis->getOffset() + hAxis->getRange() )
        //Добавляем новые объекты для отрисовки
        addToGroup(cndl);
    candleItems[index] = cndl;
}

//Слот загружает свечи после получения сигнала finished от CommandLoadStock
void CandlesSeries::addCandles(Data::Candles &&candles)
{
    bool isCandlesAdded = false;
    ///@todo переделать с использование стандартных алгритмов
    for (auto it = candles.rbegin(); it != candles.rend(); ++it) {
        bool isExist = false;

        if (!candleItems.empty()) {
            QDateTime curBeginInterval = candleItems.begin()->second->getData().dateTime;
            QDateTime curEndInterval = candleItems.rbegin()->second->getData().dateTime;
            if (curBeginInterval <= it->dateTime && it->dateTime <= curEndInterval)
                isExist = true;
        }

        if (!isExist) {
            addCandle(std::move(*it));
            isCandlesAdded = true;
        }
    }

    if (isCandlesAdded) {
        isChanged = true;

        //Если beginCandle == candleItems.end() это означает, инициализация интервала для отрисовки
        if (beginCandle == candleItems.end())
            beginCandle = candleItems.begin();
    }

    isDataRequested = false;
}
/*
void CandlesSeries::addCandles(Data::Candles &&candles)
{
    bool isCandlesAdded = false;
    ///@todo переделать с использование стандартных алгритмов
    for (auto it = candles.rbegin(); it != candles.rend(); ++it) {
        bool isExist = false;

        if (!candleItems.empty()) {
            QDateTime curBeginInterval = candleItems.begin()->second->getData().dateTime;
            QDateTime curEndInterval = candleItems.rbegin()->second->getData().dateTime;
            if (curBeginInterval <= it->dateTime && it->dateTime <= curEndInterval)
                isExist = true;
        }

        if (!isExist) {
            addCandle(std::move(*it));
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
*/

void CandlesSeries::loadCandlesFinished()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(sender());
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    addCandles(std::move(stock().candles));
    //isDataChanged = true;
}

}
