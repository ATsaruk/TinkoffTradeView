#include <QPainter>
#include <type_traits>

#include "candleitem.h"
#include "Core/globals.h"

namespace Plotter {

CandleItem::CandleItem(Data::Candle &&candle, CandlesData *candleParams) noexcept
{
    params = candleParams;
    this->candle = std::move(candle);

    setToolTip(QString("%1").arg(candle.dateTime.toString()));
}

const Data::Candle &CandleItem::getData()
{
    return candle;
}

void CandleItem::updateYPos()
{
    setY(-1 * candle.high * params->yScale);
}

QRectF CandleItem::boundingRect() const
{
    QRectF rect;
    rect.setLeft(0);
    rect.setTop (0);
    rect.setWidth(params->xScale);
    rect.setHeight((candle.high - candle.low) * params->yScale);

    return rect;
}

void CandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Устанавливаем инструменты для рисования
    bool isBearCandle = candle.open > candle.close;

    painter->setPen  ( isBearCandle ? params->redPen   : params->greenPen   );
    painter->setBrush( isBearCandle ? params->redBrush : params->greenBrush );

    QPolygon body; //Полигон, для отрисовки тела свечи

    //Помещаем координаты точек в полигональную модель
    body << QPoint(0, (candle.high - candle.open) * params->yScale)
         << QPoint(0, (candle.high - candle.close) * params->yScale)
         << QPoint(params->xScale - params->clearance, (candle.high - candle.close) * params->yScale)
         << QPoint(params->xScale - params->clearance,  (candle.high - candle.open) * params->yScale);

    //Рисуем тень свечи
    painter->drawLine((params->xScale - params->clearance) / 2., (candle.high - candle.low) * params->yScale,
                      (params->xScale - params->clearance) / 2., 0.);

    //Рисуем тело свечи по полигональной модели
    painter->drawPolygon(body);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
