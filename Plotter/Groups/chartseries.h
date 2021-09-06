#ifndef CHARTSERIES_H
#define CHARTSERIES_H

#include <QGraphicsItemGroup>

#include "Plotter/Axis/axis.h"
#include "Data/Stock/stockkey.h"
#include "Candles/candlesdata.h"

namespace Plotter {


class ChartSeries : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    explicit ChartSeries();
    virtual ~ChartSeries();

    virtual void attachAxis(Axis *axis);
    const Data::StockKey& getStockKey();


public slots:
    virtual void repaint() = 0;

protected:
    Axis *xAxis;     //horizontal axis
    Axis *yAxis;     //vertical axis

    CandlesData candlesData;

    virtual void clear() = 0;
};

}

#endif // CHARTSERIES_H
