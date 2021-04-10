#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>

#include "chartplotter.h"
#include "Core/global.h"
#include "Groups/Candles/stockitem.h"

namespace Plotter {

ChartPlotter::ChartPlotter(QWidget *parent) : QGraphicsView(parent)
{
    //Минимальные размеры виджета
    setMinimumWidth(100);
    setMinimumHeight(100);

    setViewportUpdateMode(SmartViewportUpdate);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   //отключим скроллбар по вертикали
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //отключим скроллбар по горизонтали
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);    //растягиваем содержимое по виджету
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);   //Включаем сглаживание


    graphicScene = new QGraphicsScene();   // Инициализируем сцену для отрисовки
    setScene(graphicScene);

    horizontalAxis = new HorizontalDateAxis;
    horizontalAxis->setDataRange(100);
    horizontalAxis->setDataOffset(-75);
    graphicScene->addItem(horizontalAxis);

    verticalAxis = new Axis(Axis::VERTICAL);
    verticalAxis->setDataRange(1500);
    verticalAxis->setDataOffset(0);

    //Инициализируем Таймер
    plotTimer = new QTimer(this);
    //Подключаем СЛОТ для отрисовки к таймеру
    connect(plotTimer, SIGNAL(timeout()), this, SLOT(drawScene()));
    //Запускаем таймар для начальной отрисовки
    uint plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", QVariant(5)).toUInt();
    plotTimer->start(plotInterval);
}

ChartPlotter::~ChartPlotter()
{
    delete horizontalAxis;
    delete verticalAxis;
    for (auto &it : items)
        delete it.second;
}

void ChartPlotter::setDrawStockKey(const Data::StockKey &_stockKey)
{
    if (curStockKey == _stockKey)
        return;

    curStockKey = _stockKey;

    //Все группы с ключем curStockKey делаем видимыми, остальные скрываем
    for (auto &it : items)
        it.second->setVisible(it.second->getStockKey() == curStockKey);

    //Если группы для данной акции нет, добавляем ее
    QString stringKey = curStockKey.keyToString();
    if (items.find(stringKey) == items.end()) {
        StockItem *newGroup = new StockItem;
        newGroup->setAxis(horizontalAxis);
        newGroup->setAxis(verticalAxis);
        newGroup->setStockKey(curStockKey);
        graphicScene->addItem(newGroup);
        items[stringKey] = newGroup;
    }
}

void ChartPlotter::wheelEvent(QWheelEvent *event)
{
    int steps = -0.5 * event->angleDelta().y();

    qreal deltaX = event->position().x() / width();
    qreal deltaY = event->position().y() / height();
    qreal anchorX = deltaX;
    qreal anchorY = 1 - deltaY;

    horizontalAxis->setScale(steps, anchorX);
    verticalAxis->setScale(steps, anchorY);

    event->accept();
}

void ChartPlotter::mouseMoveEvent(QMouseEvent *event)
{
    qreal dx = prevMousePos.x() - event->pos().x();
    qreal dy = event->pos().y() - prevMousePos.y();

    if (pressedButton == Qt::LeftButton) {
        horizontalAxis->setMove(dx);
        verticalAxis->setMove(dy);
    } else if (pressedButton == Qt::MiddleButton) {
        horizontalAxis->setScale(dx, mouseAnchorPos.x());
        verticalAxis->setScale(dy, mouseAnchorPos.y());
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
    verticalAxis->setSceneRect(rec);
    horizontalAxis->setSceneRect(rec);

    QGraphicsView::resizeEvent(event);  //запускаем событие родителького класса
}


void ChartPlotter::drawScene()
{
    for (const auto &it : items)
        it.second->repaint();
}

}
