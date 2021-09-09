#include "candlesdata.h"

#include "Core/globals.h"

namespace Plotter {


CandlesData::CandlesData()
{
    redPen     = Glo.conf->getValue("ChartPlotter/CandleItem/redPen", QColor(235, 77, 92));
    redBrush   = Glo.conf->getValue("ChartPlotter/CandleItem/redBrush", QColor(235, 77, 92));
    greenPen   = Glo.conf->getValue("ChartPlotter/CandleItem/greePen", QColor(83, 185, 135));
    greenBrush = Glo.conf->getValue("ChartPlotter/CandleItem/greenBrush", QColor(83, 185, 135));
}


}
