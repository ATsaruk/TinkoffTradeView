#ifndef CANDLEITEM_H
#define CANDLEITEM_H

#include <QGraphicsItem>

#include "candlesdata.h"
#include "Data/Stock/candle.h"

namespace Plotter {

class CandlesData;

///Класс для отрисовки свечи
class CandleItem : public QGraphicsItem
{
public:
    /** @param candleIndex - позиция свечи на экране, а так же это индекс свечи в массиве candleParams.data
      * @param candleParams - структура с данными для отрисовки (масштаб, цвет, свечные данные) */
    explicit CandleItem(const int32_t &index, Data::Candle *candle, std::shared_ptr<CandlesData> candleParams) noexcept;

    void setCandle(Data::Candle *candle);

    ///Возвращает индекс свечи
    const int32_t &getIndex() const;

    ///Обновляет позицию свечи по осям Х, Y в соотвествии с текущими значениями params: x, y scale
    void updatePos();

protected:
    ///Определяем виртуальный метод, который возвращает область, в которой рисуется свеча
    QRectF boundingRect() const override;

    ///Определяем метод для отрисовки свечи
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int32_t pos;
    Data::Candle *data;                     //свечные данные
    std::shared_ptr<CandlesData> params;    //структура с параметрами для рисования
};

}

#endif // CANDLEITEM_H
