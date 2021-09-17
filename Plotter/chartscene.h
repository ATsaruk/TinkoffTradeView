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
    void createCandleSeries(const Data::StockKey &key);

    ///Возвращает серию свечей с ключем this->stockKey
    std::pair<std::shared_ptr<CandlesSeries>, bool> getCurCandleSeries() const;

    ///Возвращает ось даты (ось Х)
    DateAxis* getDateAxis() const;

protected slots:
    ///Создает задачу на загрузку данных из диапазона loadRange
    void loadData(const Data::Range &loadRange) const;

private:
    Data::StockKey stockKey;
    std::vector<std::shared_ptr<CandlesSeries>> series;
};

}

#endif // CHARTSCENE_H
