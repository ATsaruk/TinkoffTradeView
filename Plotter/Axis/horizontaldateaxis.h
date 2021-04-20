#ifndef HORIZONTALDATEAXIS_H
#define HORIZONTALDATEAXIS_H

#include "axis.h"

namespace Plotter {


class HorizontalDateAxis : public Axis
{
public:
    HorizontalDateAxis();

    //Определяем виртуальный метод, который возвращает область, в которой находится треугольник
    virtual QRectF boundingRect() const override;

protected:
    int axisHeight;
    //Определяем метод для отрисовки треугольника
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

}

#endif // HORIZONTALDATEAXIS_H
