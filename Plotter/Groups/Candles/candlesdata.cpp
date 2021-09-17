#include "candlesdata.h"

#include "Core/globals.h"

namespace Plotter {


CandlesData::CandlesData()
{
    bearPen   = Glo.conf->getValue("ChartPlotter/CandleItem/bearPen", QColor(235, 77, 92));
    bearBrush = Glo.conf->getValue("ChartPlotter/CandleItem/bearBrush", QColor(235, 77, 92));
    bullPen   = Glo.conf->getValue("ChartPlotter/CandleItem/bullPen", QColor(83, 185, 135));
    bullBrush = Glo.conf->getValue("ChartPlotter/CandleItem/bullBrush", QColor(83, 185, 135));
}


}
