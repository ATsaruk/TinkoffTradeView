#ifndef CHARTPLOTTER_H
#define CHARTPLOTTER_H

#include <QObject>
#include <QtCharts>

#include "Data/Stock/stockkey.h"
#include "CandlesLoader/dbcandlesloader.h"

namespace Plotter {


///Класс рисования графика акций
class ChartPlotter : public QObject
{
public:
    explicit ChartPlotter(QObject *parent = nullptr);
    ~ChartPlotter();

    void setStockKey(Data::StockKey &&stockKey);

    QChartView *getWidget();

protected slots:
    void updateCandles();

private:
    Q_OBJECT

    Data::StockKey key;
    DbCandlesLoader dbLoader;

    QChart *chart;
    QChartView *chartView;
    QHCandlestickModelMapper *mapper;
};

}

#endif // CHARTPLOTTER_H
