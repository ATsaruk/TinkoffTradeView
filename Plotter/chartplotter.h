#ifndef CHARTPLOTTER_H
#define CHARTPLOTTER_H

#include <QGraphicsView>

#include "Axis/dateaxis.h"
#include "Axis/numericaxis.h"
#include "Groups/chartseries.h"
#include "chartscene.h"

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

    void showStock(const Data::StockKey &stockKey);

protected:
    void wheelEvent( QWheelEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

    //событие изменения размеров окна
    void resizeEvent(QResizeEvent *event) override;

    std::tuple<ChartScene *, bool> getCurScene();

protected slots:
    void drawScene();   //слот отрисовки сцены

private:
    Q_OBJECT

    //Переменные для обработки событий мыши
    QPoint prevMousePos;                //Предыдущее положение мыши
    QPointF mouseAnchorPos;             //Предыдущее положение мыши
    Qt::MouseButtons pressedButton;     //Идентефикаторы нажатых кнопок

    QTimer *plotTimer;                  //Таймер для перерисовки сцены
    std::vector<ChartScene*> scenes;          //сцена для отрисовки
};

}

#endif // CHARTPLOTTER_H
