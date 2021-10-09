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

bool StockView::empty() const
{
    return !range.isValid();
}

size_t StockView::size() const
{
    return std::distance(begin(), end());
}

std::pair<const Candle*, bool> StockView::at(size_t index) const
{
    if (index < size()) {
        const auto &candle = *(begin() + index);
        return std::make_pair(&candle, true);
    }
    return std::make_pair(&nullVector.front(), false);
}

void StockView::setBegin(const QDateTime &time)
{
    range.setBegin(time);
}

void StockView::setEnd(const QDateTime &time)
{
    range.setEnd(time);
}

}
