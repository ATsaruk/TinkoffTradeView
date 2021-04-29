#include <QThread>

#include "chartseries.h"

namespace Plotter {

ChartSeries::ChartSeries()
{
    isScaled = false;
}

ChartSeries::~ChartSeries()
{

}

void ChartSeries::attachAxis(Axis *_axis)
{
    if (_axis->getAxisType() == Axis::HORIZONTAL)
        hAxis = _axis;
    else
        vAxis = _axis;

    connect(_axis, &Axis::scaled, this, &ChartSeries::setScalse);
}

void ChartSeries::setStockKey(const Data::StockKey &stockKey)
{
    while (!clear())
        QThread::msleep(1);

    curStockKey = stockKey;

    updateData();
}

const Data::StockKey &ChartSeries::getStockKey()
{
    return curStockKey;
}

void ChartSeries::setScalse()
{
    isScaled = true;
}

}
