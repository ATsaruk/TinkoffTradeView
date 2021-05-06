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

    void attachAxis(Axis *_axis);

public slots:
    virtual void repaint() = 0;
    virtual void setScalse();

signals:
    void changed();

protected:
    bool isChanged = false;
    Axis *hAxis;     //horizontal axis
    Axis *vAxis;     //vertical axis

    virtual void updateData() = 0;
    virtual void clear() = 0;
};

}

#endif // CHARTSERIES_H
