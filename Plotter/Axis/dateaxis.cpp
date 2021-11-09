#include <QPainter>
#include <math.h>

#include "axis.h"
#include "dateaxis.h"
#include "Core/globals.h"

namespace Plotter {

DateAxis::DateAxis(qreal range, qreal offset)
    : Axis(AXIS_TYPE::HORIZONTAL), _axisHeight(Glo.conf->getValue("ChartPlotter/Axis/height", 20)),
      _candlesCount(range), _offsetIndex(offset)
{
}

qreal DateAxis::getScale()
{
    qreal scale = sceneRect.width();
    return scale / _candlesCount;
}

qreal DateAxis::getRange()
{
    return _candlesCount;
}

qreal DateAxis::getOffset()
{
    return _offsetIndex;
}

void DateAxis::setDataRange(qreal range)
{
    _candlesCount = range;
}

void DateAxis::setDataOffset(qreal offset)
{
    _offsetIndex = offset;
}

QRectF DateAxis::boundingRect() const
{
    QRect rect(0, -_axisHeight, sceneRect.width(), -_axisHeight);
    return rect;
}

void DateAxis::setMove(const qreal delta)
{
    _offsetIndex += delta / getScale();
    emit scaled();
}

void DateAxis::setScale(const qreal scale, const qreal anchor)
{
    qreal newScale;
    if (scale > 0)
        newScale = pow(1.002, scale);
    else
        newScale = pow(0.998, -1. * scale);

    qreal dataAnchor = _candlesCount * anchor;
    qreal fixedPoint = dataAnchor + _offsetIndex;
    _offsetIndex = fixedPoint - dataAnchor * newScale;
    _candlesCount *= newScale;

    emit scaled();
}

void DateAxis::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ///@todo сделать подписи даты
    painter->drawLine(0, -_axisHeight, sceneRect.width(), -_axisHeight);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
