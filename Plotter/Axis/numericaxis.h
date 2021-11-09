#ifndef NUMERICAXIS_H
#define NUMERICAXIS_H

#include "axis.h"

namespace Plotter {


class NumericAxis : public Axis
{
public:
    explicit NumericAxis(qreal range = 1, qreal offset = 0);

    qreal getScale() override;
    qreal getRange() override;
    qreal getOffset() override;

    void setDataRange(const qreal range) override;
    void setDataOffset(const qreal offset) override;

    QRectF boundingRect() const override;

public slots:
    //Масштабирование delta - число шагов для смещения
    void setMove(const qreal delta) override;

    //Масштабирование delta - во сколько раз, anchor - якорь привязки [0..1], 0 - левый край, 0.5 середина, 1 - правый край
    void setScale(const qreal scale, const qreal anchor) override;

protected:
    //Определяем метод для отрисовки
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    qreal _axisWidth;
    qreal _dataRange;
    qreal _dataOffset;
};

}

#endif // NUMERICAXIS_H
