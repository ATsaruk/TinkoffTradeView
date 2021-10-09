#include "candlesseries.h"

#include <QThread>

#include "Core/globals.h"

namespace Plotter {

CandlesSeries::CandlesSeries(CandlesData *candlesData)
    : ChartSeries(candlesData)
{
    //При обновлении списка свечей, устанавливаем признак о необходимости перерисовки
    connect(candlesData, &CandlesData::updateItems, this, &CandlesSeries::askForRepaint);

    //При создании новых свечей, добавляем их в группу для рисованияы
    connect(candlesData, &CandlesData::newItems, this, &CandlesSeries::appendCandles);

    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    drawWait = plotInterval * 3 / 4;
}

CandlesSeries::~CandlesSeries()
{
    //Отвязываем свечи от ChartSeries, что бы ChartSeries в деструкторе их не удалил, т.к. памят почитстит деструктор std::list'а
    for (auto &it : *candlesData)
        removeFromGroup(&it);

    //Отвязываем незадействованные в данный момент свечи candlesPool от ChartSeries, по той же причине
    for (auto &it : candlesData->candlesPool)
        removeFromGroup(&it);
}

void CandlesSeries::repaint()
{
    updateVisibleCandles();

    if (!isRepaintRequired || candlesData->empty() || !drawMutex.tryLock(drawWait))
        return;

    updatePriceRange();
    updateCandlesPos();

    drawMutex.unlock();
    isRepaintRequired = false;
}

void CandlesSeries::clear()
{
    //Если в текущий момент запрошены свечные данные, то пока мы их не обработаем, мы не может произвести очистку
    candlesData->waitForRequestData();

    //Удаляем свечи
    auto removeVisibleItems = [this](auto &it) mutable { if (it.isVisible()) this->removeFromGroup(&it); };
    std::for_each (candlesData->begin(), candlesData->end(), removeVisibleItems);

    candlesData->clear();
}

void CandlesSeries::updateVisibleCandles()
{
    size_t candlesCount = std::ceil(xAxis->getRange()) + 0.5;   //число свечей, которое должны быть на экране
    long long newIndex = static_cast<long long>(xAxis->getOffset());
    candlesData->update(newIndex, candlesCount);
}


void CandlesSeries::updatePriceRange()
{
    if (!candlesData->autoPriceRange)
        return;

    qreal minPrice = candlesData->begin()->getCandle()->low();
    qreal maxPrice = candlesData->begin()->getCandle()->high();

    for (const auto &it : *candlesData) {   //проверяем только свечи на экране!
        const auto &candle = it.getCandle();
        if (minPrice > candle->low())
            minPrice = candle->low();
        if (maxPrice < candle->high())
            maxPrice = candle->high();
    }

    qreal priceRange = maxPrice - minPrice;
    qreal clearence = priceRange * 0.1; // + 10% от диапазона цен зазор верху и снизу
    qreal newRange = priceRange + clearence * 2.; // * 2. т.к. зазор сверху + зазор снизу
    qreal newOffset = minPrice - clearence;  // минус зазор снизу

    yAxis->setDataRange(newRange);
    yAxis->setDataOffset(newOffset);
    candlesData->yScale = yAxis->getScale();

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
    for (auto &it : *candlesData)
        it.updatePos();

    isUpdatePosRequered = false;
}

void CandlesSeries::askForRepaint()
{
    isRepaintRequired = true;
    isUpdatePosRequered = true;
}

void CandlesSeries::appendCandles(CandlesPool::PairRange range)
{
    auto& [firstIt, lastIt] = range;
    auto appendNewCandles = [&](auto &it) mutable { this->addToGroup(&it); };
    std::for_each(firstIt, lastIt, appendNewCandles);
}

}
