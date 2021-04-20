#ifndef CHARTGROUP_H
#define CHARTGROUP_H

#include <QGraphicsItemGroup>

#include "Data/Stock/stockkey.h"
#include "Plotter/Axis/axis.h"

namespace Plotter {


class ChartGroup : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    ChartGroup();
    virtual ~ChartGroup();

    void setAxis(Axis *_axis);

    void setStockKey(const Data::StockKey &stockKey);
    const Data::StockKey& getStockKey();

public slots:
    virtual void repaint() = 0;
    virtual void setScalse();

signals:
    void changed();

protected:
    bool isScaled;
    Axis *hAxis;     //horizontal axis
    Axis *vAxis;     //vertical axis
    Data::StockKey curStockKey;

    virtual void updateData() = 0;
    virtual bool clear() = 0;
};

}

#endif // CHARTGROUP_H
