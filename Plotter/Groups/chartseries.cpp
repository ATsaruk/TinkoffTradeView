#include <QThread>

#include "chartseries.h"

namespace Plotter {

ChartSeries::ChartSeries()
    : xAxis(nullptr), yAxis(nullptr), candlesData(new CandlesData)
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
    return candlesData->stockKey;
}

void ChartSeries::update()
{
    isRepaintRequired = true;
}

}
