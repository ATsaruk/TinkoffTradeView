#ifndef CANDLEITEM_H
#define CANDLEITEM_H

#include <QPen>
#include <QBrush>
#include <QGraphicsItem>

#include "candlesdata.h"

#include "Plotter/Axis/axis.h"
#include "Data/Stock/candle.h"

namespace Plotter {

class CandlesData;

///Класс отрисовка свечи (calndle)
class CandleItem : public QGraphicsItem
{
public:
    explicit CandleItem(Data::Candle &&candle, CandlesData *candleParams) noexcept;

    const Data::Candle& getData();
    void updateYPos();

protected:
    //Определяем виртуальный метод, который возвращает область, в которой находится треугольник
    QRectF boundingRect() const override;

    //Определяем метод для отрисовки треугольника
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    CandlesData *params;

    Data::Candle candle;        //данные по свечи
};

}

#endif // CANDLEITEM_H
