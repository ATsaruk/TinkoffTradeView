#include "candlesseries.h"

#include <QThread>

#include "Core/globals.h"
#include "Plotter/Axis/axis.h"

#include "DataBase/Query/stocksquery.h"



namespace Plotter {

CandlesSeries::CandlesSeries(const Data::StockKey &stockKey)
    : candles(data), unused(data)
{
    data->stockKey = stockKey;

    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    drawWait = plotInterval * 3 / 4;
}

CandlesSeries::~CandlesSeries()
{
}

void CandlesSeries::repaint()
{
    updateVisibleCandles();

    if (!isRepaintRequired || candles.empty() || !drawMutex.tryLock(drawWait))
        return;

    updateScaleByXAxis();
    updateScaleByYAxis();
    updateCandlesPos();

    drawMutex.unlock();
    isRepaintRequired = false;
}

/*void CandlesSeries::loadCandlesFinished()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(sender());
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    addCandles( std::move(stock->candles) );
}*/

void CandlesSeries::clear()
{
    //Если в текущий момент запрошены свечные данные, то пока мы их не обработаем, мы не может произвести очистку
    while (isDataRequested)
        QThread::msleep(1);  ///@note QThread::msleep(1) может привести к зависанию при очистке CandlesSeries

    //Удаляем свечи
    for (auto &it : candles)
        if (it.isVisible())
            this->removeFromGroup(&it);

    //Очищаем список свечей
    unused.push_back( candles.pop_front(candles.size()) );  ///@todo !Проверить pop_front(candles.size())
}

void CandlesSeries::updateVisibleCandles()
{
    if (isDataRequested)
        return;     //Данные уже запрошены, ждем!

    long candleLenght = data->stockKey.candleLenght();
    size_t candlesCount = std::ceil(xAxis->getRange()) + 0.5;   //число свечей, которое должны быть на экране
    if (candles.empty()) {      //если список свечей пуст, запрашиваем весь интервал
        requestCandles(Data::Range(QDateTime::currentDateTime(), -candlesCount * candleLenght), candlesCount);
        return;
    }

    ///@todo !Возможно стоит тут же выделять помять под новые элементы

    //Вычисляем смещение отображаемого индекса свечи и число свечей, которые нужно будет запросить
    long long indexOffset = static_cast<long long>(xAxis->getOffset()) - candles.begin()->getIndex();
    long long requiredCount = abs(indexOffset) + candlesCount - candles.size();
    if (indexOffset == 0 && requiredCount == 0)
        return; //все ок, действий не требуется

    if (indexOffset == 0) {             //Первая отображаемая свеча остается на месте
        if (requiredCount < 0) {
            popBackCandles(-requiredCount);
        } else {    //requiredCount > 0
            auto lastCandleTime = candles.last()->getCandle().dateTime();
            Data::Range requiredRange(data->stockKey.nextCandleTime(lastCandleTime), requiredCount * candleLenght);
            requestCandles(requiredRange, requiredCount);
        }
    } else if (indexOffset < 0) {       //Первая отображаемая свеча смещается влево
        /* Возможен вариант firstCandle < firstDisplayedCandle && firstCandle + candlesCount > firstDisplayedCandle + displayedCandlesCount
         * Этот вариант означает, что свечи нужно добавить в начало и в конец, дабы не усложнять логику, этот вариант будет проигнорирован
         * И свечи добавим только в начале, после добавления повторный вызов CandlesSeries::updateData(),
         * приведет в ветку if (firstCandle == firstDisplayedCandle) && difCount > 0, и недостающие свечи будут догружены */
        requiredCount = -indexOffset;
        Data::Range requiredRange(candles.begin()->getCandle().dateTime(), -requiredCount * candleLenght);
        requestCandles(requiredRange, requiredCount);
    } else { //if (firstCandle > firstDisplayedCandle)
        if (requiredCount <= 0) {       //Первая отображаемая свеча смещается вправо
            //этот вариант возможен при приближении скролом, интекс смещается вправо, но и количество свечей уменьшается
            popFrontCandles(indexOffset);
        } else {
            auto lastCandleTime = candles.last()->getCandle().dateTime();
            Data::Range requiredRange(data->stockKey.nextCandleTime(lastCandleTime), requiredCount * candleLenght);
            requestCandles(requiredRange, requiredCount);
        }
    }
}

/* Функция мастабирования оси Х
 * Определяет новые индексы начала и конца интервала отображения свечей
 * Скрывает свечи, которые стали невидны, а новые свечи отображает
 */
void CandlesSeries::updateScaleByXAxis()
{
    //Обновляем масштаб по оси oX
    qreal xScale = xAxis->getScale();
    if (data->xScale != xScale) {
        data->clearance = xScale * 0.34;
        if (data->clearance > 2.)
            data->clearance = 2.;
        data->xScale = xScale;

        isUpdatePosRequered = true;
    }
}

void CandlesSeries::updateScaleByYAxis()
{
    if (data->autoPriceRange)
        updatePriceRange();

    qreal yScale = yAxis->getScale();
    if (data->yScale != yScale) {
        data->yScale = yScale;
        isUpdatePosRequered = true;
    }
}

void CandlesSeries::updatePriceRange()
{
    qreal minPrice = candles.begin()->getCandle().low();
    qreal maxPrice = candles.begin()->getCandle().high();

    for (const auto &it : candles) {   //проверяем только свечи на экране!
        const auto &candle = it.getCandle();
        if (minPrice > candle.low())
            minPrice = candle.low();
        if (maxPrice < candle.high())
            maxPrice = candle.high();
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
    setX(-1 * xAxis->getOffset() * data->xScale);
    this->setY(yAxis->getOffset() * data->yScale);

    //Обновляем положение всех видимых свечей
    for (auto &it : candles)
        it.updatePos();

    isUpdatePosRequered = false;
}

void CandlesSeries::requestCandles(const Data::Range &range, const size_t requiredCount)
{
    Task::InterfaceWrapper<Data::Range> inData = range;
    auto *command = NEW_TASK <Task::GetStock> (&inData, data->stockKey, requiredCount);
    connect(command, &Task::GetStock::finished, this, &CandlesSeries::loadCandlesFinished);
    isDataRequested = true;
}

void CandlesSeries::popFrontCandles(const long long count)
{
    unused.push_back( candles.pop_front(count) );
}

void CandlesSeries::popBackCandles(const long long count)
{
    unused.push_back( candles.pop_back(count) );
}


/* Функция, которая скрывает или отображает свечи, в зависимости от переданного интервала
 * Если первичный ключ first_iterator'а МЕНЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем отображать свечи из интервала [first_iterator..second_iterator]
 * Если первичный ключ first_iterator'а БОЛЬШЕ чем первичный ключ second_iterator'а, это означает,
 * что мы будем скрывать свечи из интервала [second_iterator..first_iterator] (т.к. у нас не reverse_iterator)
 */
/*void CandlesSeries::setCandleVisible(const CandleItems::iterator &first_iterator, const CandleItems::iterator &second_iterator)
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
}*/

//Слот загружает свечи после получения сигнала finished от CommandLoadStock
/*void CandlesSeries::addCandles(Data::Candles &&candles)
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
}*/

/*const QDateTime CandlesSeries::getDateByIndex(const int32_t index)
{
    if (data->timeMap.find(index) != data->timeMap.end())
        return data->timeMap[index];
    return QDateTime();
}*/

void CandlesSeries::loadCandlesFinished()
{

}

}
