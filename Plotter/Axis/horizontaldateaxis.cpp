#include <QPainter>

#include "axis.h"
#include "horizontaldateaxis.h"
#include "Core/globals.h"

namespace Plotter {

HorizontalDateAxis::HorizontalDateAxis()
    : Axis(AXIS_TYPE::HORIZONTAL)
{
    axisHeight = Glo.conf->getValue("ChartPlotter/Axis/height", 20);
}

QRectF HorizontalDateAxis::boundingRect() const
{
    QRect rect(0, -axisHeight, sceneRect.width(), -axisHeight);
    return rect;
}

void HorizontalDateAxis::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawLine(0, -axisHeight, sceneRect.width(), -axisHeight);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
