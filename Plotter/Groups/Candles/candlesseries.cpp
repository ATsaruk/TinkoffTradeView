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
        QThread::msleep(1);  ///@note QThread::msleep(1) может привести к зависанию при очистке CandlesSeries

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
    int32_t lastDisplayedIndex = firstDisplayedIntex + xAxis->getRange();
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

        qreal yOffset = yAxis->getOffset();
        this->setY(yOffset * candlesData.yScale);

        for (auto it = beginCandle; it != endCandle; ++it)
            it->second->updateYPos();
    }
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

    qreal priceRange = maxPrice - minPrice;
    qreal clearence = priceRange * 0.1; // + 10% от диапазона цен зазор верху и снизу
    qreal newRange = priceRange + clearence * 2.; // * 2. т.к. зазор сверху + зазор снизу
    qreal newOffset = minPrice - clearence;  // минус зазор снизу

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

//Слот загружает свечи после получения сигнала finished от CommandLoadStock
void CandlesSeries::addCandles(Data::Candles &&candles)
{
    //1. Удалям существующие (повторяющиеся) свечи
    auto isExisted = [&] (const auto &it) {
        if (candleItems.empty())
            return false;
        QDateTime curBeginInterval = candleItems.begin()->second->getData().dateTime;
        QDateTime curEndInterval = candleItems.rbegin()->second->getData().dateTime;
        return (curBeginInterval <= it.dateTime && it.dateTime <= curEndInterval);  //если it в диапазоне, то он existed
    };

    std::remove_if(candles.rbegin(), candles.rend(), isExisted);
    isRepaintRequired = !candles.empty();

    //2. Добавляем новые свечи
    int32_t increment, index;
    auto insertCalnde = [&] (auto &it) {
        candleItems[index] = new CandleItem(std::move(it), &candlesData);
        candleItems[index]->setX(index * xAxis->getScale());
        if (index >= xAxis->getOffset() && index <= xAxis->getOffset() + xAxis->getRange() )
            addToGroup(candleItems[index]);   //Добавляем новые объекты для отрисовки
        index += increment;
    };

    //Определяем место вставки, в начало или в конец списка
    if(candleItems.empty() || candles.rbegin()->dateTime < candleItems.begin()->second->getData().dateTime) {
        //Новые свечи левее существующих
        increment = -1; //свече добавляем с конца списка и уменьшаем индекс
        index = candleItems.empty() ? 0 : candleItems.begin()->first - 1;
        std::for_each(candles.rbegin(), candles.rend(), insertCalnde);
    } else {
        //Новые свечи правее существующих
        increment = 1;  //свечи добавляем с начала списка и увеличиваем индекс
        index = candleItems.rbegin()->first + 1;
        std::for_each(candles.begin(), candles.end(), insertCalnde);
    }

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
