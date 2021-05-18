#include "chartscene.h"

#include <QTimer>

#include "Core/globals.h"
#include "Axis/priceaxis.h"

#include "Tasks/StockTasks/loadstock.h"

namespace Plotter {


ChartScene::ChartScene()
{
    //setItemIndexMethod(QGraphicsScene::NoIndex); //@todo проверить производительность!
    dateAxis = new DateAxis(100, -75);
    addItem(dateAxis);

    curYAxis = nullptr;
}

ChartScene::~ChartScene()
{

}

void ChartScene::showStock(const Data::StockKey &stockKey)
{
    if (stockKey.interval() == Data::StockKey::INTERVAL::ANY)
        throw std::logic_error("ChartScene::showStock(): stockKey.interval can't be ANY!");

    if (this->stockKey == stockKey)
        return;

    for (auto &it : this->items()) {
        if (ChartSeries *series = dynamic_cast<ChartSeries*>(it) )
            if (series->getStockKey() == this->stockKey)
                removeItem(it);  //удаляем series, которые привязаны к this->stockKey (series c INTERVAL::ANY остаются)
    }

    this->stockKey = stockKey;

    if (auto curSeries = getCurCandleSeries(); curSeries)
        addItem(*curSeries);
    else
        createSeries();
}

const Data::StockKey &ChartScene::getStockKey()
{
    return stockKey;
}

void ChartScene::setScale(qreal dx, qreal xAnchor, qreal dy, qreal yAnchor)
{
    dateAxis->setScale(dx, xAnchor);
    curYAxis->setScale(dy, yAnchor);
}

void ChartScene::setMove(qreal dx, qreal dy)
{
    dateAxis->setMove(dx);
    curYAxis->setMove(dy);
}

void ChartScene::drawScene()
{
    for (auto &it : this->items()) {
        if (ChartSeries *series = dynamic_cast<ChartSeries*>(it) )
            series->repaint();
    }
}

void ChartScene::setRect(const QRectF &rect)
{
    setSceneRect(rect);
    dateAxis->setSceneRect(rect);
    for (auto &it : yAxis)
        it->setSceneRect(rect);
}

void ChartScene::createSeries()
{
    PriceAxis *axis = nullptr;
    for (auto &it : yAxis) {
        if (PriceAxis *curAxis = dynamic_cast<PriceAxis*>(it))
            axis = curAxis;
    }

    if (axis == nullptr) {
        axis = new PriceAxis;
        yAxis.push_back(axis);
        addItem(axis);
    }

    curYAxis = axis;

    series.emplace_back(new CandlesSeries(stockKey));
    auto *curSeries = series.back();
    connect(curSeries, &CandlesSeries::requestData, this, &ChartScene::loadData);
    curSeries->attachAxis(axis);
    curSeries->attachAxis(dateAxis);

    addItem(curSeries);
}

std::optional<CandlesSeries *> ChartScene::getCurCandleSeries()
{
    CandlesSeries *curSeries = nullptr;
    for (auto &it : series) {
        if (it->getStockKey() == stockKey)
            curSeries = it;
    }

    if (curSeries == nullptr)
        return std::nullopt;

    return curSeries;
}

void ChartScene::loadData(const Data::Range &loadRange)
{
    uint minCandles = dateAxis->getRange() / 3.;
    Task::InterfaceWrapper<Data::Range> range = loadRange;

    //Загружаем свечи
    auto *command = TaskManager->createTask<Task::LoadStock>(&range, stockKey, minCandles);
    if (auto curSeries = getCurCandleSeries(); curSeries)
        command->connect(*curSeries, SLOT(loadCandlesFinished()));
    else
        throw std::logic_error("ChartScene::loadData(): can't get current candle series!");

    //Загружаем остальные данные...
}

}
