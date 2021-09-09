#include "axis.h"

namespace Plotter {

Axis::Axis(const AXIS_TYPE _axisType)
{
    axisType = _axisType;
}

Axis::~Axis()
{

}

const Axis::AXIS_TYPE& Axis::getAxisType()
{
    return axisType;
}

//Resize action
void Axis::setSceneRect(const QRectF &_rect)
{
    sceneRect = _rect;
    emit scaled();
}

}
