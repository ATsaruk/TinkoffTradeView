#include <math.h>

#include "Core/globals.h"

#include "numericaxis.h"

namespace Plotter {

NumericAxis::NumericAxis(qreal range, qreal offset)
    : Axis(AXIS_TYPE::VERTICAL),
      _axisWidth(Glo.conf->getValue("ChartPlotter/Axis/width", 50)),
      _dataRange(range), _dataOffset(offset)
{
}

qreal NumericAxis::getScale()
{
    qreal scale = sceneRect.height();
    return scale / _dataRange;
}

qreal NumericAxis::getRange()
{
    return _dataRange;
}

qreal NumericAxis::getOffset()
{
    return _dataOffset;
}

void NumericAxis::setDataRange(const qreal range)
{
    _dataRange = range;
}

void NumericAxis::setDataOffset(const qreal offset)
{
    _dataOffset = offset;
}

QRectF NumericAxis::boundingRect() const
{
    return QRectF(sceneRect.width() - _axisWidth, 0, sceneRect.width(), sceneRect.height());
}

void NumericAxis::setMove(const qreal delta)
{
    _dataOffset += delta / getScale();
    emit scaled();
}

void NumericAxis::setScale(const qreal scale, const qreal anchor)
{
    qreal newScale;
    if (scale > 0)
        newScale = pow(1.002, scale);
    else
        newScale = pow(0.998, -1. * scale);

    qreal dataAnchor = _dataRange * anchor;
    qreal fixedPoint = dataAnchor + _dataOffset;
    _dataOffset = fixedPoint - dataAnchor * newScale;
    _dataRange *= newScale;

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
