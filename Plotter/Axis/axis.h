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

    explicit Axis(const AXIS_TYPE axisType);
    virtual ~Axis();

    virtual qreal getScale() = 0;
    virtual qreal getRange() = 0;
    virtual qreal getOffset() = 0;

    virtual void setDataRange(const qreal range) = 0;
    virtual void setDataOffset(const qreal offset) = 0;

    const AXIS_TYPE& getAxisType();

public slots:
    //Изменение размеров окна
    virtual void setSceneRect(const QRectF &rect);

    //Масштабирование delta - число шагов для смещения
    virtual void setMove(const qreal delta) = 0;

    //Масштабирование delta - во сколько раз, anchor - якорь привязки [0..1], 0 - левый край, 0.5 середина, 1 - правый край
    virtual void setScale(const qreal newScale, const qreal anchor) = 0;

signals:
    void scaled();

protected:
    QRectF sceneRect;

private:
    AXIS_TYPE _axisType;
};

}

#endif // AXIS_H
