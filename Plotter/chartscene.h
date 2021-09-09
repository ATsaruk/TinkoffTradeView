#ifndef CHARTSCENE_H
#define CHARTSCENE_H

#include <vector>
#include <QGraphicsScene>

#include "Axis/dateaxis.h"
#include "Data/Stock/stockkey.h"
#include "Groups/Candles/candlesseries.h"

namespace Plotter {


class ChartScene : public QGraphicsScene
{
public:
    explicit ChartScene();
    ~ChartScene();

    void showStock(const Data::StockKey &stockKey);
    const Data::StockKey &getStockKey();

    void setScale(qreal dx, qreal xAnchor, qreal dy, qreal yAnchor);
    void setMove(qreal dx, qreal dy);

public slots:
    void drawScene();
    void setRect(const QRectF &rect);

protected:
    void createSeries();
    std::optional<CandlesSeries *> getCurCandleSeries();

protected slots:
    void loadData(const Data::Range &loadRange);

private:
    Data::StockKey stockKey;
    std::vector<CandlesSeries *> series;

    DateAxis *dateAxis;
    Axis *curYAxis;
    std::list<Axis*> yAxis;
};

}

#endif // CHARTSCENE_H
