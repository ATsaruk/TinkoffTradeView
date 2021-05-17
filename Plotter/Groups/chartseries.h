#ifndef CHARTSERIES_H
#define CHARTSERIES_H

#include <QGraphicsItemGroup>

#include "Data/Stock/stockkey.h"
#include "Plotter/Axis/axis.h"

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
    virtual void setScalse();

signals:
    void changed();

protected:
    bool isRepaintRequired = false;
    Axis *xAxis;     //horizontal axis
    Axis *yAxis;     //vertical axis
    Data::StockKey stockKey;

    virtual void clear() = 0;
};

}

#endif // CHARTSERIES_H
