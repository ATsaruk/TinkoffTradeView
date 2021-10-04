#ifndef SERIESDATA_H
#define SERIESDATA_H

#include <QPen>
#include <QBrush>

#include "candleitem.h"
#include "Data/Stock/candle.h"
#include "Data/Stock/stockkey.h"

class Axis;
class CandleItem;
class ChartSeries;

namespace Plotter {

///Класс содержащий данные для отрисовки свечной информации, а так же саму свечную информацию
class SeriesData
{
public:
    SeriesData();

private:
    long xScale = 0;                //Масштаб по оси Х
    long yScale = 0;                //Масштаб по оси Y
    qreal clearance = 0.;           //Зазор по оси Х между свечами

    bool autoPriceRange = true;     //Режим автомастабирования оси Y по цене //@note возможно его нужно от сюда перенести?

    //Pen и Brush для отрисовки свечи (обычно бычьи свечи - красные, медвежьи свечи - зеленые)
    QPen bearPen, bullPen;
    QBrush bearBrush, bullBrush;

    Data::StockKey stockKey;                //Ключи акции для которой предназначены данные
    std::map<int32_t, QDateTime> timeMap;      //Список позиция свечи + её дата

    friend class Axis;
    friend class CandlesList;
    friend class CandleItem;
    friend class ChartSeries;
    friend class CandlesSeries;
};

}

#endif // SERIESDATA_H
