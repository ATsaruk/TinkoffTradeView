#include "chartplotter.h"
#include "Core/global.h"

namespace Plotter {


ChartPlotter::ChartPlotter(QObject *parent) : QObject(parent)
{
    chart = new QChart();
    chart->setTheme(QChart::ChartThemeBrownSand);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chartView = new QChartView(chart);
    chartView->setBackgroundBrush(QColor(243, 236, 224));
    chartView->setRenderHint(QPainter::Antialiasing);

    connect(&dbLoader, &DbCandlesLoader::candlesRecieved, this, &ChartPlotter::updateCandles);
}

ChartPlotter::~ChartPlotter()
{

}

void ChartPlotter::setStockKey(Data::StockKey &&stockKey)
{
    chart->setTitle("Apple");
    key = std::move(stockKey);
    Data::Range range;
    range.setRange(QDateTime::currentDateTime(), -48 * 60 * 60);
    //dbLoader.setStockKey(key, std::move(range));
}

QChartView *ChartPlotter::getWidget()
{
    return chartView;
}

void ChartPlotter::updateCandles()
{
    QReadLocker locker(&Glo.stocks->rwMutex);
    const Data::Candles &candles = Glo.stocks->getStock(key);

    QCandlestickSeries *acmeSeries = new QCandlestickSeries();
    acmeSeries->setName("Acme Ltd");
    acmeSeries->setIncreasingColor(QColor(Qt::green));
    acmeSeries->setDecreasingColor(QColor(Qt::red));

    QStringList categories;

    for (const auto &it : candles) {
        qreal timestamp = it.dateTime.toTime_t();
        QCandlestickSet *set = new QCandlestickSet(timestamp);
        set->setOpen(it.open);
        set->setHigh(it.high);
        set->setLow(it.low);
        set->setClose(it.close);

        acmeSeries->append(set);
        categories << it.dateTime.toString();
    }

    chart->addSeries(acmeSeries);
    chart->createDefaultAxes();

    QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis *>(chart->axes(Qt::Horizontal).at(0));
    axisX->setCategories(categories);
    QValueAxis *axisY = qobject_cast<QValueAxis *>(chart->axes(Qt::Vertical).at(0));
    axisY->setMax(axisY->max() * 1.01);
    axisY->setMin(axisY->min() * 0.99);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

}
