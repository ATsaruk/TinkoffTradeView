#include "candleitem.h"

#include <QPen>
#include <QBrush>
#include <QPainter>

#include "Data/Stock/candle.h"
#include "../ChartData/candlesdata.h"

namespace Plotter {

CandleItem::CandleItem(CandlesData *candlesParams)
    : params(candlesParams)
{
    this->setVisible(false);    //До инициализации через set(...) свеча не отображается
}

void CandleItem::clear()
{
    candle = nullptr;
    this->setVisible(false);
}

void CandleItem::set(const int32_t &index, const Data::Candle *candle)
{
    pos = index;
    this->candle = const_cast<Data::Candle*> (candle);
    setToolTip(QString("%1").arg(candle->dateTime().toString()));
    this->setVisible(true);
}

int32_t CandleItem::getIndex() const
{
    return pos;
}

const Data::Candle* CandleItem::getCandle() const
{
    return candle;
}

void CandleItem::updatePos()
{
    if (params == nullptr || candle == nullptr) {
        this->setVisible(false);
        return;
    }

    setX(params->xScale * pos);
    setY(params->yScale * candle->high() * -1.);
}

QRectF CandleItem::boundingRect() const
{
    QRectF rect;
    if (params == nullptr || candle == nullptr) {
        const_cast<CandleItem*>(this)->setVisible(false);
        return rect;
    }

    rect.setLeft(0);
    rect.setTop (0);
    rect.setWidth (params->xScale);
    rect.setHeight(params->yScale * (candle->high() - candle->low()));

    return rect;
}

void CandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (params == nullptr || candle == nullptr) {
        this->setVisible(false);
        return;
    }

    //Устанавливаем инструменты для рисования
    bool isBearCandle = candle->open() > candle->close();

    painter->setPen  ( isBearCandle ? params->bearPen   : params->bullPen   );
    painter->setBrush( isBearCandle ? params->bearBrush : params->bullBrush );

    QPolygon body; //Полигон, для отрисовки тела свечи

    //Помещаем координаты точек в полигональную модель
    body << QPoint(0, (candle->high() - candle->open()) * params->yScale)
         << QPoint(0, (candle->high() - candle->close()) * params->yScale)
         << QPoint(params->xScale - params->clearance, (candle->high() - candle->close()) * params->yScale)
         << QPoint(params->xScale - params->clearance,  (candle->high() - candle->open()) * params->yScale);

    //Рисуем тень свечи
    painter->drawLine((params->xScale - params->clearance) / 2., (candle->high() - candle->low()) * params->yScale,
                      (params->xScale - params->clearance) / 2., 0.);

    //Рисуем тело свечи по полигональной модели
    painter->drawPolygon(body);

    Q_UNUSED(option); Q_UNUSED(widget);
}

}
