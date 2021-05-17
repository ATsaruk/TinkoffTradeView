#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>

#include "chartplotter.h"
#include "Core/globals.h"
#include "Groups/Candles/candlesseries.h"

namespace Plotter {

ChartPlotter::ChartPlotter(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(100, 100);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    plotTimer = new QTimer(this);
    connect(plotTimer, &QTimer::timeout, this, &ChartPlotter::drawScene);
    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    plotTimer->start(plotInterval);
}

ChartPlotter::~ChartPlotter()
{
    for (auto &it : scenes)
        delete it;
}

void ChartPlotter::showStock(const Data::StockKey &stockKey)
{
    ChartScene *existedScene = nullptr;
    for (auto &it : scenes) {
        if (it->getStockKey().figi() == stockKey.figi())
            existedScene = it;
    }

    if (existedScene == nullptr) {
        existedScene = new ChartScene;
        scenes.push_back(existedScene);
    }

    existedScene->showStock(stockKey);
    setScene(existedScene);
}

void ChartPlotter::wheelEvent(QWheelEvent *event)
{
    int steps = -0.5 * event->angleDelta().y();

    qreal deltaX = event->position().x() / width();
    qreal deltaY = event->position().y() / height();
    qreal anchorX = deltaX;
    qreal anchorY = 1 - deltaY;

    if (auto [curScene, ok] = getCurScene(); ok)
        curScene->setScale(steps, anchorX, steps, anchorY);

    event->accept();
}

void ChartPlotter::mouseMoveEvent(QMouseEvent *event)
{
    qreal dx = prevMousePos.x() - event->pos().x();
    qreal dy = event->pos().y() - prevMousePos.y();

    if (auto [curScene, ok] = getCurScene(); ok) {
        if (pressedButton == Qt::LeftButton)
            curScene->setMove(dx, dy);
        else if (pressedButton == Qt::MiddleButton)
            curScene->setScale(dx, mouseAnchorPos.x(), dy, mouseAnchorPos.y());
        // else if (pressedButton == Qt::RightButton)
    }

    prevMousePos = event->pos();
    event->accept();
}

void ChartPlotter::mousePressEvent(QMouseEvent *event)
{
    pressedButton = event->buttons();
    prevMousePos = event->pos();

    qreal wdth = width();
    qreal hght = height();
    qreal anchorX = prevMousePos.x() / wdth;
    qreal anchorY = (hght - prevMousePos.y()) / hght;
    mouseAnchorPos.setX(anchorX);
    mouseAnchorPos.setY(anchorY);
}

void ChartPlotter::mouseReleaseEvent(QMouseEvent *event)
{
    pressedButton = event->buttons();
}

void ChartPlotter::resizeEvent(QResizeEvent *event)
{
    QRectF rec(0, -height(), width(), height());

    setSceneRect(rec);
    if (auto [curScene, ok] = getCurScene(); ok)
        curScene->setRect(rec);

    QGraphicsView::resizeEvent(event);
}

std::tuple<ChartScene*, bool> ChartPlotter::getCurScene()
{
    ChartScene *currentScene = dynamic_cast<ChartScene*>(scene());
    bool ok = currentScene != nullptr;
    return std::make_tuple(currentScene, ok);
}

void ChartPlotter::drawScene()
{
    if (auto [curScene, ok] = getCurScene(); ok)
        curScene->drawScene();
}

}
