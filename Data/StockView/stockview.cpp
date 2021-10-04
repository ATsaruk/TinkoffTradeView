#include "stockview.h"

namespace Data {


StockView::StockView()
{
    nullVector.emplace_back(QDateTime(), 0., 0., 0., 0., 0);  //null element для operator []
}

StockView::~StockView()
{

}

const Range StockView::getRange() const
{
    return range;
}

size_t StockView::size() const
{
    return std::count_if(begin(), end(), [](const auto &it){return true;Q_UNUSED(it)});
}

std::pair<const Candle*, bool> StockView::operator [](size_t index)
{
    if (index < size()) {
        auto &candle = *(begin() + index);
        return std::make_pair(&candle, true);
    }
    return std::make_pair(&nullVector.front(), false);
}

std::pair<const Candle&, bool> StockView::operator [](size_t index) const
{
    if (index < size()) {
        const auto &candle = *(begin() + index);
        return std::make_pair(candle, true);
    }
    return std::make_pair(nullVector.front(), false);
}

}
