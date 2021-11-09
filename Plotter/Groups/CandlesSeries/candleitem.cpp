#include "candleitem.h"

#include <QPen>
#include <QBrush>
#include <QPainter>

#include "Data/Stock/candle.h"
#include "../ChartData/candlesdata.h"

namespace Plotter {

CandleItem::CandleItem(CandlesData *candlesParams)
    : _params(candlesParams)
{
    this->setVisible(false);    //До инициализации через set(...) свеча не отображается
}

void CandleItem::clear()
{
    _candle = nullptr;
    this->setVisible(false);
}

void CandleItem::set(const int32_t &index, const Data::Candle *candle)
{
    _pos = index;
    this->_candle = const_cast<Data::Candle*> (candle);
    setToolTip(QString("%1").arg(candle->dateTime().toString()));
    this->setVisible(true);
}

int32_t CandleItem::getIndex() const
{
    return _pos;
}

const Data::Candle* CandleItem::getCandle() const
{
    return _candle;
}

void CandleItem::updatePos()
{
    if (_params == nullptr || _candle == nullptr) {
        this->setVisible(false);
        return;
    }

    setX(_params->_xScale * _pos);
    setY(_params->_yScale * _candle->high() * -1.);
}

QRectF CandleItem::boundingRect() const
{
    QRectF rect;
    if (_params == nullptr || _candle == nullptr) {
        const_cast<CandleItem*>(this)->setVisible(false);
        return rect;
    }

    rect.setLeft(0);
    rect.setTop (0);
    rect.setWidth (_params->_xScale);
    rect.setHeight(_params->_yScale * (_candle->high() - _candle->low()));

    return rect;
}

void CandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (_params == nullptr || _candle == nullptr) {
        this->setVisible(false);
        return;
    }

    //Устанавливаем инструменты для рисования
    bool isBearCandle = _candle->open() > _candle->close();

    painter->setPen  ( isBearCandle ? _params->_bearPen   : _params->_bullPen   );
    painter->setBrush( isBearCandle ? _params->_bearBrush : _params->_bullBrush );

    QPolygon body; //Полигон, для отрисовки тела свечи

    //Помещаем координаты точек в полигональную модель
    body << QPoint(0, (_candle->high() - _candle->open()) * _params->_yScale)
         << QPoint(0, (_candle->high() - _candle->close()) * _params->_yScale)
         << QPoint(_params->_xScale - _params->_clearance, (_candle->high() - _candle->close()) * _params->_yScale)
         << QPoint(_params->_xScale - _params->_clearance,  (_candle->high() - _candle->open()) * _params->_yScale);

    //Рисуем тень свечи
    painter->drawLine((_params->_xScale - _params->_clearance) / 2., (_candle->high() - _candle->low()) * _params->_yScale,
                      (_params->_xScale - _params->_clearance) / 2., 0.);

    //Рисуем тело свечи по полигональной модели
    painter->drawPolygon(body);

    Q_UNUSED(option); Q_UNUSED(widget);
}

}
