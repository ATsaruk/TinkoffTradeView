#include "chartscene.h"

#include <QTimer>

#include "Core/globals.h"
#include "Axis/priceaxis.h"

#include "Tasks/StockTasks/getstock.h"

namespace Plotter {


ChartScene::ChartScene(const Data::StockKey &stockKey)
{
    this->stockKey = stockKey;
    createCandleSeries(stockKey);

    setItemIndexMethod(QGraphicsScene::NoIndex); //NoIndex меньше лаг отрисовки при перемещении по экрану большого кол-ва свечей
}

ChartScene::~ChartScene()
{

}

const Data::StockKey &ChartScene::getStockKey() const
{
    return stockKey;
}

void ChartScene::setScale(const qreal dx, const qreal xAnchor, const qreal dy, const qreal yAnchor)
{
    for (const auto &it : items()) {
        if (Axis *axis = dynamic_cast<Axis*>(it); axis != nullptr) {
            if (axis->getAxisType() == Axis::HORIZONTAL)
                axis->setScale(dx, xAnchor);
            if (axis->getAxisType() == Axis::VERTICAL)
                axis->setScale(dy, yAnchor);
        }
    }
}

void ChartScene::setMove(const qreal dx, const qreal dy)
{
    for (const auto &it : items()) {
        if (Axis *axis = dynamic_cast<Axis*>(it); axis != nullptr) {
            if (axis->getAxisType() == Axis::HORIZONTAL)
                axis->setMove(dx);
            if (axis->getAxisType() == Axis::VERTICAL)
                axis->setMove(dy);
        }
    }
}

void ChartScene::drawScene() const
{
    for (const auto &it : items()) {
        if (ChartSeries *series = dynamic_cast<ChartSeries*>(it) )
            series->repaint();
    }
}

void ChartScene::setRect(const QRectF &rect)
{
    setSceneRect(rect);
    for (const auto &it : items())
        if (Axis *axis = dynamic_cast<Axis*>(it); axis != nullptr)
            axis->setSceneRect(rect);
}

void ChartScene::createCandleSeries(const Data::StockKey &key)
{
    auto *newSeries = new CandlesSeries(key);

    if (auto [curSeries, ok] = getCurCandleSeries(); ok) {
        //Если существует основная серия свечей, то прикрепляемся к её оси Х
        newSeries->attachAxis(curSeries->getAxis(Axis::HORIZONTAL));
    } else {
        //Создаем новую ось Х
        newSeries->attachAxis(std::make_shared<DateAxis>(100, -75));
    }

    //Создаем новую ось Y (для каждой серии свечей будет своя ось цены, что бы их можно было масштабировать независимо друг от друга)
    newSeries->attachAxis(std::make_shared<PriceAxis>());

    addItem(newSeries);
    addItem(newSeries->getAxis(Axis::HORIZONTAL).get());
    addItem(newSeries->getAxis(Axis::VERTICAL).get());

    series.emplace_back(newSeries);
}

std::pair<std::shared_ptr<CandlesSeries>, bool> ChartScene::getCurCandleSeries() const
{
    for (auto &it : series)
        if (it->getStockKey() == stockKey)
            return std::make_pair(it, true);

    return std::make_pair(nullptr, false);
}

DateAxis* ChartScene::getDateAxis() const
{
    for (const auto &it : items())
        if (DateAxis *axis = dynamic_cast<DateAxis*>(it); axis != nullptr)
            return axis;

    logCritical << "ChartScene::getDateAxis(); Can't find DateAxis!";
    throw std::logic_error("ChartScene::getDateAxis(); Can't find DateAxis!");
}

}
