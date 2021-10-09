#ifndef CHARTPLOTTER_H
#define CHARTPLOTTER_H

#include <QTimer>
#include <QGraphicsView>

#include "chartscene.h"

class QWheelEvent;
class QMouseEvent;

namespace Plotter {


///Класс рисования графика акций
class ChartPlotter : public QGraphicsView
{
public:
    explicit ChartPlotter(QWidget *parent = nullptr);
    ~ChartPlotter();

    //Отображает акцию с ключем stockKey
    void showStock(const Data::StockKey &stockKey);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    //событие изменения размеров окна
    void resizeEvent(QResizeEvent *event) override;

    //Возвращает текущую сцену + признак того, что эта сцена является ChartScene
    std::tuple<ChartScene*, bool> getCurScene() const;

protected slots:
    void drawScene() const;   //слот отрисовки сцены

private:
    Q_OBJECT

    //Переменные для обработки событий мыши
    QPoint prevMousePos;                //Предыдущее положение мыши
    QPointF mouseAnchorPos;             //Предыдущее положение мыши
    Qt::MouseButtons pressedButton;     //Идентефикаторы нажатых кнопок

    QTimer plotTimer;                   //Таймер для перерисовки сцены
    std::vector<std::shared_ptr<ChartScene>> scenes;    //список сцен для отрисовки
};

}

#endif // CHARTPLOTTER_H
