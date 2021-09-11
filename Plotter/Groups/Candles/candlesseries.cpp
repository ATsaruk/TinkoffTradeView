#include "candlesseries.h"

#include "Core/globals.h"
#include "Plotter/Axis/axis.h"

#include "DataBase/Query/stocksquery.h"


#include <QThread>


namespace Plotter {

CandlesSeries::CandlesSeries(const Data::StockKey &stockKey)
{
    candlesData.stockKey = stockKey;

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
    updateData();

    if (!isRepaintRequired || candleItems.empty())
        return;

    if (!drawMutex.tryLock(drawWait))
        return;

    scaleByXAxis();

    if (candlesData.autoPriceRange)
        updatePriceRange();

    scaleByYAxis();

    isRepaintRequired = false;

    drawMutex.unlock();
}

void CandlesSeries::loadCandlesFinished()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(sender());
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    addCandles(std::move(stock->candles));
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

void CandlesSeries::updateData()
{
    if (isDataRequested)
        return;

    long intervalSec = candlesData.stockKey.intervalToSec(); //Возвращает опцион


    Data::Range range;
    int32_t displayedCandlesCount = xAxis->getRange();
    if (candleItems.empty()) {
        range.setRange(QDateTime::currentDateTime(), -displayedCandlesCount * intervalSec * 2);
        isDataRequested = true;
        emit requestData(range);
    } else {
        int32_t offset = xAxis->getOffset();
        if (offset < candleItems.begin()->first) {
            range.setRange(candleItems.begin()->second->getData().dateTime, -displayedCandlesCount * intervalSec);
            isDataRequested = true;
            emit requestData(range);
        }
    }
}

/* Функция мастабирования оси Х
 * Определяет новые индексы начала и конца интервала отображения свечей
 * Свечи, которые стали невидны скрывает, новые свечи отображает
 * И обновляем масштаб всех видимых свечей
 */
void CandlesSeries::scaleByXAxis()
{
    ///Определяем новый индекс начали интервала отображения свечей
    int32_t firstDisplayedIntex = xAxis->getOffset();
    auto &&newBeginCandle = candleItems.find(firstDisplayedIntex);
    //Если элемент с индексом firstDisplayedIntex не найден, то будем отображать с первого элемента
    if (newBeginCandle == candleItems.end())
        newBeginCandle = candleItems.begin();

    ///Определяем новый индекс конца интервала отображения свечей
    // +1 т.к. чтобы отображать часть свечи, которая невлезла целиком на экран ??????? не актуально???
    int32_t lastDisplayedIndex = firstDisplayedIntex + xAxis->getRange();// + 1; ///@todo !проверить не будет ли наложений при большом количестве свечей
    auto &&newEndCandle = candleItems.find(lastDisplayedIndex);

    setCandleVisible(newBeginCandle, beginCandle);
    beginCandle = newBeginCandle;

    setCandleVisible(endCandle, newEndCandle);
    endCandle = newEndCandle;

    //Обновляем масштаб и положение видимых свечей по оси oX
    qreal xScale = xAxis->getScale();
    setXScale(xScale);

    //Задаем положение на экране всей QGraphicsItemGroup
    setX(-1 * firstDisplayedIntex * xScale);
}

void CandlesSeries::scaleByYAxis()
{
    qreal yScale = yAxis->getScale();
    if (candlesData.yScale != yScale) {
        candlesData.yScale = yScale;

        for (auto it = beginCandle; it != endCandle; ++it)
            it->second->updateYPos();
    }

    qreal yOffset = yAxis->getOffset();
    this->setY(yOffset * yScale);
}

/* Функция, которая скрывает или отображает свечи, в зависимости от переданного интервала
 * Если первичный ключ first_iterator'а МЕНЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем отображать свечи из интервала [first_iterator..second_iterator]
 * Если первичный ключ first_iterator'а БОЛЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем скрывать свечи из интервала [second_iterator..first_iterator] (т.к. у нас не reverse_iterator)
 */
void CandlesSeries::setCandleVisible(const std::map<int32_t, CandleItem*>::iterator &first_iterator, const std::map<int32_t, CandleItem*>::iterator &second_iterator)
{
    //Принцип определения параметра visible описан выше
    bool visible  = first_iterator->first < second_iterator->first;
    //Определяем порядок движения (от меньшего значения к большему)
    auto &begin = visible ? first_iterator : second_iterator;
    auto &end   = visible ? second_iterator : first_iterator;

    for (auto it = begin; it != end; ++it) {
        it->second->setVisible(visible);

        if (visible) { //Для видимых свечей
            //Обновляем масштаб свечи по оси оY
            it->second->updateYPos();

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

    ///@todo !!проблемы с масштабированием! разобратся с поведением range и offset для priceAxis
    qreal priceRange = maxPrice - minPrice;
    qreal newRange = priceRange;// * 1.2; // + 20% что бы на графике сверху и снизу быз "зазор"
    qreal newOffset = minPrice;// - priceRange * 0.2;  // - 10% половина от расширения диапазона
    yAxis->setDataRange(newRange);
    yAxis->setDataOffset(newOffset);

    //autoPriceRange = false;   //while settings autoPriceRange off is absent
}

const QDateTime CandlesSeries::getDateByIndex(const int32_t index)
{
    if (candleItems.find(index) != candleItems.end())
        return candleItems[index]->getData().dateTime;
    return QDateTime();
}

//Добавление 1 свечи, поиск нового индекса
void CandlesSeries::addCandle(Data::Candle &&candleData)
{
    int32_t index = 0.;
    if (!candleItems.empty()) {
        if (candleItems.begin()->second->getData().dateTime > candleData.dateTime)
            index = candleItems.begin()->first - 1;
        if (candleItems.rbegin()->second->getData().dateTime < candleData.dateTime)
            index = candleItems.rbegin()->first + 1;
    }

    if(candleItems.find(index) != candleItems.end())
        return; //Свеча уже существует

    CandleItem *cndl = new CandleItem(std::move(candleData), &candlesData);
    cndl->setX(index * xAxis->getScale());
    if (index >= xAxis->getOffset() &&
            index <= xAxis->getOffset() + xAxis->getRange() )
        //Добавляем новые объекты для отрисовки
        addToGroup(cndl);
    candleItems[index] = cndl;
}

//Слот загружает свечи после получения сигнала finished от CommandLoadStock
void CandlesSeries::addCandles(Data::Candles &&candles)
{
    ///@todo переделать с использование стандартных алгритмов
    for (auto it = candles.rbegin(); it != candles.rend(); ++it) {
        bool isExisted = false;

        if (!candleItems.empty()) {
            QDateTime curBeginInterval = candleItems.begin()->second->getData().dateTime;
            QDateTime curEndInterval = candleItems.rbegin()->second->getData().dateTime;
            if (curBeginInterval <= it->dateTime && it->dateTime <= curEndInterval)
                isExisted = true;
        }

        if (!isExisted) {
            addCandle(std::move(*it));
            isRepaintRequired = true;
        }
    }

    if (beginCandle == candleItems.end() && !candleItems.empty())
        beginCandle = candleItems.begin();

    isDataRequested = false;
}

void CandlesSeries::setXScale(qreal scale)
{
    if (candlesData.xScale != scale) {
        candlesData.clearance = scale * 0.34;
        if (candlesData.clearance > 2.)
            candlesData.clearance = 2.;
        candlesData.xScale = scale;
    }

    for (auto it = beginCandle; it != endCandle; ++it)
        it->second->setX(it->first * scale);
}

}
