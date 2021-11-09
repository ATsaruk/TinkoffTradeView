#include "chartscene.h"

#include <QTimer>

#include "Core/globals.h"
#include "Axis/priceaxis.h"

namespace Plotter {


ChartScene::ChartScene(const Data::StockKey &stockKey)
{
    _candlesData.setStockKey(stockKey);
    createCandleSeries();

    setItemIndexMethod(QGraphicsScene::NoIndex); //NoIndex меньше лаг отрисовки при перемещении по экрану большого кол-ва свечей
}

ChartScene::~ChartScene()
{

}

const Data::StockKey &ChartScene::getStockKey() const
{
    return _candlesData.getStockKey();
}

void ChartScene::setScale(const qreal dx, const qreal xAnchor, const qreal dy, const qreal yAnchor)
{
    for (const auto &it : items()) {
        if (auto *axis = dynamic_cast<Axis*>(it); axis) {
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
        if (auto *axis = dynamic_cast<Axis*>(it); axis) {
            if (axis->getAxisType() == Axis::HORIZONTAL)
                axis->setMove(dx);
            if (axis->getAxisType() == Axis::VERTICAL)
                axis->setMove(dy);
        }
    }
}

void ChartScene::drawScene() const
{
    for (const auto &it : items())
        if (auto *series = dynamic_cast<ChartSeries*>(it); series)
            series->repaint();
}

void ChartScene::setRect(const QRectF &rect)
{
    for (const auto &it : items())
        if (auto *axis = dynamic_cast<Axis*>(it); axis)
            axis->setSceneRect(rect);
    setSceneRect(rect);
}

void ChartScene::createCandleSeries()
{
    _candles = std::make_shared<CandlesSeries>(&_candlesData);

    _candles->attachAxis(std::make_shared<DateAxis>(100, -99));  //[-99 ... 0] = 100 candles
    //Создаем новую ось Y (для каждой серии свечей будет своя ось цены, что бы их можно было масштабировать независимо друг от друга)
    _candles->attachAxis(std::make_shared<PriceAxis>());

    addItem(_candles.get());
    addItem(_candles->getAxis(Axis::HORIZONTAL).get());
    addItem(_candles->getAxis(Axis::VERTICAL).get());
}

DateAxis* ChartScene::getDateAxis() const
{
    for (const auto &it : items())
        if (auto *axis = dynamic_cast<DateAxis*>(it); axis)
            return axis;

    logCritical << "ChartScene::getDateAxis(); Can't find DateAxis!";
    throw std::logic_error("ChartScene::getDateAxis(); Can't find DateAxis!");
}

}
