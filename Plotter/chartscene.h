#ifndef CHARTSCENE_H
#define CHARTSCENE_H

#include <vector>
#include <QGraphicsScene>

#include "Axis/dateaxis.h"
#include "Groups/CandlesSeries/candlesseries.h"

namespace Plotter {


class ChartScene : public QGraphicsScene
{
public:
    explicit ChartScene(const Data::StockKey &stockKey);
    ~ChartScene();

    const Data::StockKey& getStockKey() const;

    void setScale(const qreal dx, const qreal xAnchor, const qreal dy, const qreal yAnchor);
    void setMove(const qreal dx, const qreal dy);

public slots:
    void drawScene() const;
    void setRect(const QRectF &rect);

protected:
    ///Создает новую серию свечей с ключем key
    void createCandleSeries();

    ///Возвращает ось даты (ось Х)
    DateAxis* getDateAxis() const;

private:
    CandlesData candlesData;
    std::shared_ptr<CandlesSeries> candles;
};

}

#endif // CHARTSCENE_H
