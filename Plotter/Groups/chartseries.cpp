#include <QThread>

#include "chartseries.h"

namespace Plotter {

ChartSeries::ChartSeries(CandlesData *candlesData)
    : xAxis(nullptr), yAxis(nullptr), candlesData(candlesData)
{

}

ChartSeries::~ChartSeries()
{

}

std::shared_ptr<Axis> &ChartSeries::getAxis(const Axis::AXIS_TYPE &type)
{
    if (type == Axis::HORIZONTAL)
        return xAxis;

    return yAxis;
}

void ChartSeries::attachAxis(std::shared_ptr<Axis> axis)
{
    if (axis->getAxisType() == Axis::HORIZONTAL)
        xAxis = axis;
    else
        yAxis = axis;
    connect(axis.get(), &Axis::scaled, this, &ChartSeries::update);
}

const Data::StockKey &ChartSeries::getStockKey()
{
    return candlesData->getStockKey();
}

void ChartSeries::update()
{
    auto axis = dynamic_cast<Axis*>(sender());
    if (axis->getAxisType() == Axis::HORIZONTAL)
        updateScaleByXAxis();
    else
        updateScaleByYAxis();
    isRepaintRequired = true;
}

/* Функция мастабирования оси Х
 * Определяет новые индексы начала и конца интервала отображения свечей
 * Скрывает свечи, которые стали невидны, а новые свечи отображает
 */
void ChartSeries::updateScaleByXAxis()
{
    //Обновляем масштаб по оси oX
    qreal xScale = xAxis->getScale();
    if (candlesData->xScale != xScale) {
        candlesData->clearance = xScale * 0.34;
        if (candlesData->clearance > 2.)
            candlesData->clearance = 2.;
        candlesData->xScale = xScale;

        isUpdatePosRequered = true;
    }

    long long offset = xAxis->getOffset();
    if (candlesData->offsetIndex != offset) {
        candlesData->offsetIndex = offset;
        isUpdatePosRequered = true;
    }

    candlesData->candlesCount = xAxis->getRange();
}

void ChartSeries::updateScaleByYAxis()
{
    qreal yScale = yAxis->getScale();
    if (candlesData->yScale != yScale) {
        candlesData->yScale = yScale;
        isUpdatePosRequered = true;
    }
}

}
