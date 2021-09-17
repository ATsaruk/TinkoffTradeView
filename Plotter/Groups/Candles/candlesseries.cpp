#include "candlesseries.h"

#include <QThread>

#include "Core/globals.h"
#include "Plotter/Axis/axis.h"

#include "DataBase/Query/stocksquery.h"



namespace Plotter {

CandlesSeries::CandlesSeries(const Data::StockKey &stockKey)
{
    candlesData->stockKey = stockKey;

    beginCandle = candleItems.end();
    endCandle = candleItems.end();

    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    drawWait = plotInterval * 3 / 4;
}

CandlesSeries::~CandlesSeries()
{

}

void CandlesSeries::repaint()
{
    updateData();

    if (!isRepaintRequired || candleItems.empty() || !drawMutex.tryLock(drawWait))
        return;

    updateScaleByXAxis();
    updateScaleByYAxis();
    updateCandlesPos();

    drawMutex.unlock();
    isRepaintRequired = false;
}

void CandlesSeries::loadCandlesFinished()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(sender());
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    addCandles( std::move(stock->candles) );
}

void CandlesSeries::clear()
{
    //Если в текущий момент запрошены свечные данные, то пока мы их не обработаем, мы не может произвести очистку
    while (isDataRequested)
        QThread::msleep(1);  ///@note QThread::msleep(1) может привести к зависанию при очистке CandlesSeries

    //Удаляем свечи
    for (auto &it : candleItems)
        if (it.second->isVisible())
            this->removeFromGroup(it.second.get());

    //Очищаем список свечей
    candleItems.clear();

    //Сбрасываем отображаемый интервал
    beginCandle = candleItems.end();
    endCandle = candleItems.end();
}

/* Функция мастабирования оси Х
 * Определяет новые индексы начала и конца интервала отображения свечей
 * Скрывает свечи, которые стали невидны, а новые свечи отображает
 */
void CandlesSeries::updateScaleByXAxis()
{
    //Определяем новый индекс начали интервала отображения свечей
    int32_t firstDisplayedIntex = xAxis->getOffset();
    auto &&newBeginCandle = candleItems.find(firstDisplayedIntex);
    //Если элемент с индексом firstDisplayedIntex не найден, то будем отображать с первого элемента
    if (newBeginCandle == candleItems.end())
        newBeginCandle = candleItems.begin();

    //Определяем новый индекс конца интервала отображения свечей
    int32_t lastDisplayedIndex = firstDisplayedIntex + xAxis->getRange();
    auto &&newEndCandle = candleItems.find(lastDisplayedIndex);

    setCandleVisible(newBeginCandle, beginCandle);
    beginCandle = newBeginCandle;

    setCandleVisible(endCandle, newEndCandle);
    endCandle = newEndCandle;

    //Обновляем масштаб по оси oX
    qreal xScale = xAxis->getScale();
    if (candlesData->xScale != xScale) {
        candlesData->clearance = xScale * 0.34;
        if (candlesData->clearance > 2.)
            candlesData->clearance = 2.;
        candlesData->xScale = xScale;

        isUpdatePosRequered = true;
    }
}

void CandlesSeries::updateScaleByYAxis()
{
    if (candlesData->autoPriceRange)
        updatePriceRange();

    qreal yScale = yAxis->getScale();
    if (candlesData->yScale != yScale) {
        candlesData->yScale = yScale;
        isUpdatePosRequered = true;
    }
}

void CandlesSeries::updatePriceRange()
{
    qreal minPrice = candlesData->data[beginCandle->first].low;
    qreal maxPrice = candlesData->data[beginCandle->first].high;

    for (auto it = beginCandle; it != endCandle; ++it) {   //проверяем только свечи на экране!
        const Data::Candle& data = candlesData->data[it->first];
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

void CandlesSeries::updateCandlesPos()
{
    if (!isUpdatePosRequered)
        return;

    //Задаем положение на экране всей QGraphicsItemGroup
    setX(-1 * xAxis->getOffset() * candlesData->xScale);
    this->setY(yAxis->getOffset() * candlesData->yScale);

    //Обновляем положение всех видимых свечей
    for (auto it = beginCandle; it != endCandle; ++it)
        it->second->updatePos();

    isUpdatePosRequered = false;
}

void CandlesSeries::updateData()
{
    if (isDataRequested)
        return;     //Данные уже запрошены, ждем!

    long intervalSec = candlesData->stockKey.intervalToSec();

    int32_t displayedCandlesCount = xAxis->getRange();
    if (candleItems.empty()) {
        Data::Range range(QDateTime::currentDateTime(), -displayedCandlesCount * intervalSec * 2);
        emit requestData(range);
        isDataRequested = true;
    } else {
        int32_t offset = xAxis->getOffset();
        if (offset < candleItems.begin()->first) {
            Data::Range range(candlesData->data.begin()->second.dateTime, -displayedCandlesCount * intervalSec);
            emit requestData(range);
            isDataRequested = true;
        }
    }
}

/* Функция, которая скрывает или отображает свечи, в зависимости от переданного интервала
 * Если первичный ключ first_iterator'а МЕНЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем отображать свечи из интервала [first_iterator..second_iterator]
 * Если первичный ключ first_iterator'а БОЛЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем скрывать свечи из интервала [second_iterator..first_iterator] (т.к. у нас не reverse_iterator)
 */
void CandlesSeries::setCandleVisible(const CandleItems::iterator &first_iterator, const CandleItems::iterator &second_iterator)
{
    //Принцип определения параметра visible описан выше
    bool visible  = first_iterator->first < second_iterator->first;

    //Определяем порядок движения (от меньшего значения к большему)
    auto &begin = visible ? first_iterator : second_iterator;
    auto &end   = visible ? second_iterator : first_iterator;

    for (auto it = begin; it != end; ++it) {
        it->second->setVisible(visible);

        //Видимые свечи добавляем в группу QGraphicsItemGroup, не видимые соответственно удаляем.
        if (visible)
            this->addToGroup(it->second.get());
        else
            this->removeFromGroup(it->second.get());
    }
}

//Слот загружает свечи после получения сигнала finished от CommandLoadStock
void CandlesSeries::addCandles(Data::Candles &&candles)
{
    removeExistedCandles(candles);

    //Лямбда для добавления свечи
    int32_t increment, index;
    auto insertCalnde = [&] (auto &it) {
        candlesData->data[index] = std::move(it);
        candleItems[index] = std::make_shared<CandleItem>(index, candlesData);
        candleItems[index]->setX(index * xAxis->getScale());
        if (index >= xAxis->getOffset() && index <= xAxis->getOffset() + xAxis->getRange() ) {
            addToGroup(candleItems[index].get());   //Добавляем новые объекты для отрисовки
            isRepaintRequired = true;
        }
        index += increment;
    };

    //Определяем место вставки, в начало или в конец списка
    if(candleItems.empty() || candles.rbegin()->dateTime < candlesData->data.begin()->second.dateTime) {
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

//Эта функция служит скорее для отладки, если тут появились повторяющиеся свечи, значит некорректно формируется граница
//для загрузки свечей и нужно разбираться с той функцией!
/// @note после отладки функцию можно удалить, а можно и не удалять :)
void CandlesSeries::removeExistedCandles(Data::Candles &candles)
{
    auto isExisted = [&] (const auto &it) {
        if (candlesData->data.empty())
            return false;
        QDateTime curBeginInterval = candlesData->data.begin()->second.dateTime;
        QDateTime curEndInterval = candlesData->data.rbegin()->second.dateTime;
        return (curBeginInterval <= it.dateTime && it.dateTime <= curEndInterval);  //если it в диапазоне, то он existed
    };

    const auto &start_erase = std::remove_if(candles.begin(), candles.end(), isExisted);
    if (uint32_t count = std::distance(start_erase, candles.end()); count > 0)
        logWarning << QString("CandlesSeries::addCandles:;removed %1 existed candles!").arg(count);
    candles.erase(start_erase, candles.end());
}

///@todo !!после переделки candleData->data на std::map<int32_t, QDateTime> эту функцию нужно удалить!
const QDateTime CandlesSeries::getDateByIndex(const int32_t index)
{
    if (candlesData->data.find(index) != candlesData->data.end())
        return candlesData->data[index].dateTime;
    return QDateTime();
}

}
