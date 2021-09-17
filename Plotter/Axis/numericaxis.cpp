#include <math.h>

#include "Core/globals.h"

#include "numericaxis.h"

namespace Plotter {

NumericAxis::NumericAxis(qreal range, qreal offset)
    : Axis(AXIS_TYPE::VERTICAL)
{
    axisWidth = Glo.conf->getValue("ChartPlotter/Axis/width", 50);
    setDataRange(range);
    setDataOffset(offset);
}

qreal NumericAxis::getScale()
{
    qreal scale = sceneRect.height();
    return scale / dataRange;
}

qreal NumericAxis::getRange()
{
    return dataRange;
}

qreal NumericAxis::getOffset()
{
    return dataOffset;
}

void NumericAxis::setDataRange(const qreal range)
{
    dataRange = range;
}

void NumericAxis::setDataOffset(const qreal offset)
{
    dataOffset = offset;
}

QRectF NumericAxis::boundingRect() const
{
    return QRectF(sceneRect.width() - axisWidth, 0, sceneRect.width(), sceneRect.height());
}

void NumericAxis::setMove(const qreal delta)
{
    dataOffset += delta / getScale();
    emit scaled();
}

void NumericAxis::setScale(const qreal scale, const qreal anchor)
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

void NumericAxis::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ///@todo сделать подписи цены

    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
