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

    connect(&plotTimer, &QTimer::timeout, this, &ChartPlotter::drawScene);
    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
    plotTimer.start(plotInterval);
}

ChartPlotter::~ChartPlotter()
{

}

/** @brief ChartPlotter::showStock Отображает акцию с ключем stockKey
 *  @param stockKey - ключ акции
 *
 *  Ищет существующую сцену с ключем stockKey, если не находит создает её
 */
void ChartPlotter::showStock(const Data::StockKey &stockKey)
{
    auto isSameStockKey = [&stockKey](const auto &it){ return it->getStockKey().figi() == stockKey.figi(); };
    auto findedIterator = std::find_if(scenes.begin(), scenes.end(), isSameStockKey);

    if (findedIterator == scenes.end()) {
        //сцены для акции с ключом stockKey не найдено
        scenes.emplace_back(new ChartScene(stockKey));
        setScene( scenes.back().get()     );
    } else
        setScene( (*findedIterator).get() );
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
    ///@bug при масштабировашии окна справа некорректно обновляется граница, где рисуются свечи!
    QRectF rec(0, -event->size().height(), event->size().width(), event->size().height());

    setSceneRect(rec);
    if (auto [curScene, ok] = getCurScene(); ok)
        curScene->setRect(rec);

    QGraphicsView::resizeEvent(event);
}

std::tuple<ChartScene*, bool> ChartPlotter::getCurScene() const
{
    ChartScene *currentScene = dynamic_cast<ChartScene*>(scene());
    bool ok = currentScene != nullptr;
    return std::make_tuple(currentScene, ok);
}

void ChartPlotter::drawScene() const
{
    if (auto [curScene, ok] = getCurScene(); ok)
        curScene->drawScene();
}

}
