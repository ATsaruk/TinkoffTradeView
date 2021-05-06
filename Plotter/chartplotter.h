#ifndef CHARTPLOTTER_H
#define CHARTPLOTTER_H

#include <QGraphicsView>

#include "Axis/axis.h"
#include "Axis/dateaxis.h"
#include "Groups/chartseries.h"

class QTimer;
class QWheelEvent;
class QMouseEvent;

namespace Plotter {


///Класс рисования графика акций
class ChartPlotter : public QGraphicsView
{
public:
    explicit ChartPlotter(QWidget *parent = nullptr);
    ~ChartPlotter();

    void setDrawStockKey( const Data::StockKey &_stockKey );

protected:
    void wheelEvent( QWheelEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

    //событие изменения размеров окна
    void resizeEvent(QResizeEvent *event) override;

protected slots:
    void drawScene();   //слот отрисовки сцены

private:
    Q_OBJECT

    //Переменные для обработки событий мыши
    QPoint prevMousePos;                //Предыдущее положение мыши
    QPointF mouseAnchorPos;             //Предыдущее положение мыши
    Qt::MouseButtons pressedButton;     //Идентефикаторы нажатых кнопок

    //temp
    DateAxis *dateAxis;
    Axis *priceAxis;
    //temp

    QTimer *plotTimer;                  //Таймер для перерисовки сцены
    QGraphicsScene *graphicScene;       //сцена для отрисовки

    //Данные для рисования
    Data::StockKey stockKey;
};

}

#endif // CHARTPLOTTER_H
