#ifndef AXIS_H
#define AXIS_H

#include <QGraphicsItem>

namespace Plotter {


class Axis : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    enum AXIS_TYPE : uint8_t { HORIZONTAL, VERTICAL };

    explicit Axis(const AXIS_TYPE _axisType);
    virtual ~Axis();

    const AXIS_TYPE& getAxisType();

    virtual qreal getScale();
    virtual qreal getRange();
    virtual qreal getOffset();

    virtual void setDataRange(const qreal range);
    virtual void setDataOffset(const qreal offset);

    //Определяем виртуальный метод, который возвращает область, в которой находится треугольник
    virtual QRectF boundingRect() const override {return QRectF();}// = 0;

public slots:
    //Изменение размеров окна
    virtual void setSceneRect(const QRectF &_rect);

    //Масштабирование delta - число шагов для смещения
    virtual void setMove(const qreal delta);

    //Масштабирование delta - во сколько раз, anchor - якорь привязки [0..1], 0 - левый край, 0.5 середина, 1 - правый край
    virtual void setScale(const qreal newScale, const qreal anchor);

signals:
    void scaled();

protected:
    qreal  dataRange;
    qreal  dataOffset;
    QRectF sceneRect;

    //Определяем метод для отрисовки треугольника
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {Q_UNUSED(painter);Q_UNUSED(option);Q_UNUSED(widget);}// = 0;

private:
    AXIS_TYPE axisType;
};

}

#endif // AXIS_H
