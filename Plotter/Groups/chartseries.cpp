#include <QThread>

#include "chartseries.h"

namespace Plotter {

ChartSeries::ChartSeries()
{

}

ChartSeries::~ChartSeries()
{

}

void ChartSeries::attachAxis(Axis *axis)
{
    if (axis->getAxisType() == Axis::HORIZONTAL)
        xAxis = axis;
    else
        yAxis = axis;
    connect(axis, &Axis::scaled, this, &ChartSeries::update);
}

const Data::StockKey &ChartSeries::getStockKey()
{
    return candlesData.stockKey;
}

void ChartSeries::update()
{
    isRepaintRequired = true;
}

}
