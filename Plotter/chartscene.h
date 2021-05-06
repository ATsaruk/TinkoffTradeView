#ifndef CHARTSCENE_H
#define CHARTSCENE_H

#include <vector>
#include <QGraphicsScene>

#include "Axis/dateaxis.h"
#include "Data/Stock/stockkey.h"

namespace Plotter {


class ChartScene : public QGraphicsScene
{
public:
    explicit ChartScene(const Data::StockKey &stockKey_);
    ~ChartScene();

    void setEnable(const bool enable);

public slots:
    void drawScene();

private:
    uint plotInterval;
    Data::StockKey stockKey;

    std::list<DateAxis*> dateAxis;
    std::list<Axis*> priceAxis;

    //Таймер для перерисовки сцены
    QTimer *plotTimer;
};

}

#endif // CHARTSCENE_H
