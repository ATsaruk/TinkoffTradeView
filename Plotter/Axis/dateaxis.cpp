#include <QPainter>

#include "axis.h"
#include "dateaxis.h"
#include "Core/globals.h"

namespace Plotter {

DateAxis::DateAxis()
    : Axis(AXIS_TYPE::HORIZONTAL)
{
    axisHeight = Glo.conf->getValue("ChartPlotter/Axis/height", 20);
}

QRectF DateAxis::boundingRect() const
{
    QRect rect(0, -axisHeight, sceneRect.width(), -axisHeight);
    return rect;
}

void DateAxis::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawLine(0, -axisHeight, sceneRect.width(), -axisHeight);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
