#include <math.h>

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

qreal Axis::getScale()
{
    qreal scale = ( axisType == HORIZONTAL ? sceneRect.width() : sceneRect.height() );
    return scale / dataRange;
}

qreal Axis::getRange()
{
    return dataRange;
}

qreal Axis::getOffset()
{
    return dataOffset;
}

void Axis::setDataRange(const qreal range)
{
    dataRange = range;
}

void Axis::setDataOffset(const qreal offset)
{
    dataOffset = offset;
}

void Axis::setMove(const qreal delta)
{
    dataOffset += delta / getScale();
    emit scaled();
}

//Resize action
void Axis::setSceneRect(const QRectF &_rect)
{
    sceneRect = _rect;
    emit scaled();
}

void Axis::setScale(const qreal scale, const qreal anchor)
{
    qreal newScale;
    if (scale > 0)
        newScale = pow(1.002, scale);
    else
        newScale = pow(0.998, -1. * scale);

    qreal dataAnchor = dataRange * anchor;
    qreal fixedPoint = dataAnchor + dataOffset;
    dataOffset = fixedPoint - dataAnchor * newScale;
    dataRange *= newScale;

    emit scaled();
}

}
