#include "axis.h"

namespace Plotter {

Axis::Axis(const AXIS_TYPE axisType)
    : _axisType(axisType)
{
}

Axis::~Axis()
{

}

const Axis::AXIS_TYPE& Axis::getAxisType()
{
    return _axisType;
}

//Resize action
void Axis::setSceneRect(const QRectF &rect)
{
    sceneRect = rect;
    emit scaled();
}

}
