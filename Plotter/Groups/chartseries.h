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

    virtual std::shared_ptr<Axis>& getAxis(const Axis::AXIS_TYPE &type);
    virtual void attachAxis(std::shared_ptr<Axis> axis);
    const Data::StockKey& getStockKey();


public slots:
    virtual void repaint() = 0;

protected:
    bool isRepaintRequired = false;

    std::shared_ptr<Axis> xAxis;  //horizontal axis
    std::shared_ptr<Axis> yAxis = nullptr;  //vertical axis

    std::shared_ptr<CandlesData> candlesData;

    virtual void clear() = 0;

protected slots:
    virtual void update();
};

}

#endif // CHARTSERIES_H
