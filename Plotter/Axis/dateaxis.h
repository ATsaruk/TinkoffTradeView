#ifndef DATEAXIS_H
#define DATEAXIS_H

#include "axis.h"

namespace Plotter {


class DateAxis : public Axis
{
public:
    explicit DateAxis();

    //Определяем виртуальный метод, который возвращает область, в которой находится треугольник
    virtual QRectF boundingRect() const override;

protected:
    int axisHeight;
    //Определяем метод для отрисовки треугольника
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

}

#endif // DATEAXIS_H
