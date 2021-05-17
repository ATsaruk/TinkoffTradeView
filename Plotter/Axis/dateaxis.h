#ifndef DATEAXIS_H
#define DATEAXIS_H

#include "axis.h"

namespace Plotter {


class DateAxis : public Axis
{
public:
    explicit DateAxis(qreal range = 1, qreal offset = 0);

    qreal getScale() override;
    qreal getRange() override;
    qreal getOffset() override;

    void setDataRange(qreal range) override;
    void setDataOffset(qreal offset) override;

    //Определяем виртуальный метод, который возвращает область, в которой находится треугольник
    virtual QRectF boundingRect() const override;

public slots:
    //Масштабирование delta - число шагов для смещения
    void setMove(const qreal delta) override;

    //Масштабирование delta - во сколько раз, anchor - якорь привязки [0..1], 0 - левый край, 0.5 середина, 1 - правый край
    void setScale(const qreal scale, const qreal anchor) override;

protected:
    //Определяем метод для отрисовки треугольника
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    qreal axisHeight;
    qreal candlesCount;
    qreal offsetIndex;
};

}

#endif // DATEAXIS_H
