#ifndef CANDLESDATA_H
#define CANDLESDATA_H

#include <QPen>
#include <QBrush>

#include "candleitem.h"
#include "Data/Stock/stockkey.h"

class Axis;
class CandleItem;
class ChartSeries;

namespace Plotter {


class CandlesData
{
public:
    CandlesData();

private:
    long xScale = 0;
    long yScale = 0;
    qreal clearance = 0.;

    bool autoPriceRange = true;

    QPen redPen;
    QPen greenPen;
    QBrush redBrush;
    QBrush greenBrush;

    Data::StockKey stockKey;

    friend class Axis;
    friend class CandleItem;
    friend class ChartSeries;
    friend class CandlesSeries;
};

}

#endif // CANDLESDATA_H
