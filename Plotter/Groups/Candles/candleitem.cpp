#include <QPainter>

#include "candleitem.h"
#include "Core/globals.h"

namespace Plotter {

CandleItem::CandleItem(const Data::Candle &_candle)
{
    candle = _candle;

    QColor redPen     = Glo.conf->getValue("ChartPlotter/CandleItem/redPen",     QColor(128,   0, 0)).value<QColor>();
    QColor greenPen   = Glo.conf->getValue("ChartPlotter/CandleItem/greePen",    QColor(  0, 128, 0)).value<QColor>();
    QColor redBrush   = Glo.conf->getValue("ChartPlotter/CandleItem/redBrush",   QColor(196,   0, 0)).value<QColor>();
    QColor greenBrush = Glo.conf->getValue("ChartPlotter/CandleItem/greenBrush", QColor(  0, 196, 0)).value<QColor>();

    //Красный для медвежей свечи, зеленый для бычей свечи
    bool isBearCandle = candle.open > candle.close;
    pen.setColor  ( isBearCandle ? redPen   : greenPen );
    brush.setColor( isBearCandle ? redBrush : greenBrush );
    brush.setStyle(Qt::SolidPattern);

    setToolTip(QString("%1").arg(candle.dateTime.toString()));
}

const Data::Candle &CandleItem::getData()
{
    return candle;
}

void CandleItem::setCandleVerticalScale(const qreal newScale)
{
    if (verticalScale != newScale) {
        verticalScale = newScale;
        setY(-1 * candle.high * verticalScale);
    }
}

void CandleItem::setCandleHorizontalScale(const qreal newWidth)
{
    if (horizontalWidth != newWidth) {
        horizontalClearance = newWidth * 0.34;
        if (horizontalClearance > 2.)
            horizontalClearance = 2.;
        horizontalWidth = newWidth;
    }
}

QRectF CandleItem::boundingRect() const
{
    QRectF rect;
    rect.setLeft(0);
    rect.setTop (0);
    rect.setWidth(horizontalWidth);
    rect.setHeight((candle.high - candle.low) * verticalScale);

    return rect;
}

void CandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Устанавливаем инструменты для рисования
    painter->setPen(pen);
    painter->setBrush(brush);

    QPolygon body; //Полигон, для отрисовки тела свечи

    //Помещаем координаты точек в полигональную модель
    body << QPoint(0, (candle.high - candle.open) * verticalScale)
         << QPoint(0, (candle.high - candle.close) * verticalScale)
         << QPoint(horizontalWidth - horizontalClearance, (candle.high - candle.close) * verticalScale)
         << QPoint(horizontalWidth - horizontalClearance, (candle.high - candle.open) * verticalScale);

    //Рисуем тень свечи
    painter->drawLine((horizontalWidth - horizontalClearance) / 2.,
                      (candle.high - candle.low) * verticalScale,
                      (horizontalWidth - horizontalClearance) / 2.,
                      0.);

    //Рисуем тело свечи по полигональной модели
    painter->drawPolygon(body);

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

}
