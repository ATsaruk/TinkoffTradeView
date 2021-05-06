#include <QThread>

#include "chartseries.h"

namespace Plotter {

ChartSeries::ChartSeries()
{

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

void ChartSeries::setScalse()
{
    isChanged = true;
}

}
