#include <QPen>
#include <QBrush>
#include <QPainter>
//#include <type_traits>

#include "candleitem.h"
#include "Core/globals.h"

namespace Plotter {

CandleItem::CandleItem(const int32_t candleIndex, std::shared_ptr<CandlesData> candleParams) noexcept
    : index(candleIndex), params(candleParams)
{
    setToolTip(QString("%1").arg(params->data[index].dateTime.toString()));
}

const int32_t &CandleItem::getIndex()
{
    return index;
}

void CandleItem::updatePos()
{
    setX(params->xScale * index);
    setY(params->yScale * params->data[index].high * -1.);
}

QRectF CandleItem::boundingRect() const
{
    QRectF rect;
    rect.setLeft(0);
    rect.setTop (0);
    rect.setWidth (params->xScale);
    rect.setHeight(params->yScale * (params->data[index].high - params->data[index].low));

    return rect;
}

void CandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Устанавливаем инструменты для рисования
    bool isBearCandle = params->data[index].open > params->data[index].close;

    painter->setPen  ( isBearCandle ? params->bearPen   : params->bullPen   );
    painter->setBrush( isBearCandle ? params->bearBrush : params->bullBrush );

    QPolygon body; //Полигон, для отрисовки тела свечи

    //Помещаем координаты точек в полигональную модель
    body << QPoint(0, (params->data[index].high - params->data[index].open) * params->yScale)
         << QPoint(0, (params->data[index].high - params->data[index].close) * params->yScale)
         << QPoint(params->xScale - params->clearance, (params->data[index].high - params->data[index].close) * params->yScale)
         << QPoint(params->xScale - params->clearance,  (params->data[index].high - params->data[index].open) * params->yScale);

    //Рисуем тень свечи
    painter->drawLine((params->xScale - params->clearance) / 2., (params->data[index].high - params->data[index].low) * params->yScale,
                      (params->xScale - params->clearance) / 2., 0.);

    //Рисуем тело свечи по полигональной модели
    painter->drawPolygon(body);

    Q_UNUSED(option); Q_UNUSED(widget);
}

}
