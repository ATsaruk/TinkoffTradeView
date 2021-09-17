#include <QPainter>
#include <math.h>

#include "axis.h"
#include "dateaxis.h"
#include "Core/globals.h"

namespace Plotter {

DateAxis::DateAxis(qreal range, qreal offset)
    : Axis(AXIS_TYPE::HORIZONTAL)
{
    axisHeight = Glo.conf->getValue("ChartPlotter/Axis/height", 20);
    setDataRange(range);
    setDataOffset(offset);
}

qreal DateAxis::getScale()
{
    qreal scale = sceneRect.width();
    return scale / candlesCount;
}

qreal DateAxis::getRange()
{
    return candlesCount;
}

qreal DateAxis::getOffset()
{
    return offsetIndex;
}

void DateAxis::setDataRange(qreal range)
{
    candlesCount = range;
}

void DateAxis::setDataOffset(qreal offset)
{
    offsetIndex = offset;
}

QRectF DateAxis::boundingRect() const
{
    QRect rect(0, -axisHeight, sceneRect.width(), -axisHeight);
    return rect;
}

void DateAxis::setMove(const qreal delta)
{
    offsetIndex += delta / getScale();
    emit scaled();
}

void DateAxis::setScale(const qreal scale, const qreal anchor)
{
    qreal newScale;
    if (scale > 0)
        newScale = pow(1.002, scale);
    else
        newScale = pow(0.998, -1. * scale);

    qreal dataAnchor = candlesCount * anchor;
    qreal fixedPoint = dataAnchor + offsetIndex;
    offsetIndex = fixedPoint - dataAnchor * newScale;
    candlesCount *= newScale;

    emit scaled();
}

void DateAxis::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ///@todo сделать подписи даты
    painter->drawLine(0, -axisHeight, sceneRect.width(), -axisHeight);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
