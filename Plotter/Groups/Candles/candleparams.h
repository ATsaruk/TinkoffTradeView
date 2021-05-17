#ifndef CANDLEPARAMS_H
#define CANDLEPARAMS_H

#include <QPen>
#include <QBrush>

namespace Plotter {


struct CandleParams
{
    QPointF scale = QPointF(0., 0.);
    qreal clearance = 0;
    QPen redPen;
    QBrush redBrush;
    QPen greenPen;
    QBrush greenBrush;

    CandleParams();
};

}

#endif // CANDLEPARAMS_H
