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

    connect(axis, &Axis::scaled, this, &ChartSeries::setScalse);
}

const Data::StockKey &ChartSeries::getStockKey()
{
    return stockKey;
}

void ChartSeries::setScalse()
{
    isRepaintRequired = true;
}

}
