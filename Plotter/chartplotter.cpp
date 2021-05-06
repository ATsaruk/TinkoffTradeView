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

    graphicScene = new QGraphicsScene();
    setScene(graphicScene);

    ///temp
    dateAxis = new DateAxis;
    dateAxis->setDataRange(100);
    dateAxis->setDataOffset(-75);
    graphicScene->addItem(dateAxis);

    priceAxis = new Axis(Axis::VERTICAL);
    priceAxis->setDataRange(1500);
    priceAxis->setDataOffset(0);
    ///temp

    plotTimer = new QTimer(this);
    connect(plotTimer, &QTimer::timeout, this, &ChartPlotter::drawScene);
    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    plotTimer->start(plotInterval);
}

ChartPlotter::~ChartPlotter()
{
}

void ChartPlotter::setDrawStockKey(const Data::StockKey &_stockKey)
{
    if (stockKey == _stockKey)
        return;

    stockKey = _stockKey;

    bool isExist = false;
    /*for (auto &it : graphicScene->items()) {
        if (ChartSeries *series = dynamic_cast<ChartSeries*>(it)) {
            bool isVisible = series->getStockKey() == stockKey;
            series->setVisible(isVisible);
            if (isVisible)
                isExist = true;
        }
    }*/

    if (!isExist) {
        auto series = new CandlesSeries(stockKey);
        series->attachAxis(dateAxis);
        series->attachAxis(priceAxis);
        graphicScene->addItem(series);
    }
}

void ChartPlotter::wheelEvent(QWheelEvent *event)
{
    int steps = -0.5 * event->angleDelta().y();

    qreal deltaX = event->position().x() / width();
    qreal deltaY = event->position().y() / height();
    qreal anchorX = deltaX;
    qreal anchorY = 1 - deltaY;

    dateAxis->setScale(steps, anchorX);
    priceAxis->setScale(steps, anchorY);

    event->accept();
}

void ChartPlotter::mouseMoveEvent(QMouseEvent *event)
{
    qreal dx = prevMousePos.x() - event->pos().x();
    qreal dy = event->pos().y() - prevMousePos.y();

    if (pressedButton == Qt::LeftButton) {
        dateAxis->setMove(dx);
        priceAxis->setMove(dy);
    } else if (pressedButton == Qt::MiddleButton) {
        dateAxis->setScale(dx, mouseAnchorPos.x());
        priceAxis->setScale(dy, mouseAnchorPos.y());
    }// else if (pressedButton == Qt::RightButton)

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
    graphicScene->setSceneRect(rec);
    priceAxis->setSceneRect(rec);
    dateAxis->setSceneRect(rec);

    QGraphicsView::resizeEvent(event);
}

void ChartPlotter::drawScene()
{
    for (auto &it : graphicScene->items()) {
        if (ChartSeries *series = dynamic_cast<ChartSeries*>(it) )
            series->repaint();
    }
}

}
