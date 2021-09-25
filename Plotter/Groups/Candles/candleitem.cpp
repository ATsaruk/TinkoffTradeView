#include <QPen>
#include <QBrush>
#include <QPainter>
//#include <type_traits>

#include "candleitem.h"
#include "Core/globals.h"

namespace Plotter {

CandleItem::CandleItem(const int32_t &index, Data::Candle *candle, std::shared_ptr<CandlesData> candleParams) noexcept
    : pos(index), data(candle), params(candleParams)
{
    setToolTip(QString("%1").arg(data->dateTime().toString()));
}

void CandleItem::setCandle(Data::Candle *candle)
{
    data = candle;
}

const int32_t &CandleItem::getIndex() const
{
    return pos;
}

void CandleItem::updatePos()
{
    setX(params->xScale * pos);
    setY(params->yScale * data->high() * -1.);
}

QRectF CandleItem::boundingRect() const
{
    QRectF rect;
    rect.setLeft(0);
    rect.setTop (0);
    rect.setWidth (params->xScale);
    rect.setHeight(params->yScale * (data->high() - data->low()));

    return rect;
}

void CandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Устанавливаем инструменты для рисования
    bool isBearCandle = data->open() > data->close();

    painter->setPen  ( isBearCandle ? params->bearPen   : params->bullPen   );
    painter->setBrush( isBearCandle ? params->bearBrush : params->bullBrush );

    QPolygon body; //Полигон, для отрисовки тела свечи

    //Помещаем координаты точек в полигональную модель
    body << QPoint(0, (data->high() - data->open()) * params->yScale)
         << QPoint(0, (data->high() - data->close()) * params->yScale)
         << QPoint(params->xScale - params->clearance, (data->high() - data->close()) * params->yScale)
         << QPoint(params->xScale - params->clearance,  (data->high() - data->open()) * params->yScale);

    //Рисуем тень свечи
    painter->drawLine((params->xScale - params->clearance) / 2., (data->high() - data->low()) * params->yScale,
                      (params->xScale - params->clearance) / 2., 0.);

    //Рисуем тело свечи по полигональной модели
    painter->drawPolygon(body);

    Q_UNUSED(option); Q_UNUSED(widget);
}

}
