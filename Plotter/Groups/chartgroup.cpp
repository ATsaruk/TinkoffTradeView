#include <QThread>

#include "chartgroup.h"

namespace Plotter {

ChartGroup::ChartGroup()
{
    isScaled = false;
}

ChartGroup::~ChartGroup()
{

}

void ChartGroup::setAxis(Axis *_axis)
{
    if (_axis->getAxisType() == Axis::HORIZONTAL)
        hAxis = _axis;
    else
        vAxis = _axis;

    connect(_axis, &Axis::scaled, this, &ChartGroup::setScalse);
}

void ChartGroup::setStockKey(const Data::StockKey &stockKey)
{
    while (!clear())
        QThread::msleep(1);

    curStockKey = stockKey;

    updateData();
}

const Data::StockKey &ChartGroup::getStockKey()
{
    return curStockKey;
}

void ChartGroup::setScalse()
{
    isScaled = true;
}

}
