#ifndef CANDLEITEM_H
#define CANDLEITEM_H

#include <QPen>
#include <QBrush>
#include <QGraphicsItem>

#include "Data/Stock/candle.h"
#include "Plotter/Axis/axis.h"

namespace Plotter {

///Класс отрисовка свечи (calndle)
class CandleItem : public QGraphicsItem
{
public:
    explicit CandleItem(const Data::Candle &_candle);

    const Data::Candle& getData();

    void setCandleVerticalScale(const qreal newScale);
    void setCandleHorizontalScale(const qreal newWidth);

protected:
    //Определяем виртуальный метод, который возвращает область, в которой находится треугольник
    QRectF boundingRect() const override;

    //Определяем метод для отрисовки треугольника
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    qreal horizontalWidth;      //Ширина свечи
    qreal horizontalClearance;  //Зазор между свечами
    qreal verticalScale;        //масштабирование по оси Y

    QPen pen;                   //ручка для очертания свечи
    QBrush brush;               //кисть для тела свечи

    Data::Candle candle;        //данные по свечи
};

}

#endif // CANDLEITEM_H
