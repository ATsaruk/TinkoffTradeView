#include "chartscene.h"

#include <QTimer>

#include "Core/globals.h"
#include "Groups/chartseries.h"

namespace Plotter {


ChartScene::ChartScene(const Data::StockKey &stockKey_)
    : stockKey(stockKey_)
{
    plotTimer = new QTimer(this);
    connect(plotTimer, &QTimer::timeout, this, &ChartScene::drawScene);
    plotInterval = Glo.conf->getValue("ChartPlotter/plotInterval", 5);
}

ChartScene::~ChartScene()
{

}

void ChartScene::setEnable(const bool enable)
{
    if (enable != plotTimer->isActive())
        enable ? plotTimer->start(plotInterval) : plotTimer->stop();
}

void ChartScene::drawScene()
{
    for (auto &it : this->items()) {
        if (ChartSeries *series = dynamic_cast<ChartSeries*>(it) )
            series->repaint();
    }
}

}
