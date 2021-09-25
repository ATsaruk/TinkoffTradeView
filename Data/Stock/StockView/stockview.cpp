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
    return std::size(*this);    //число элементов от [this->begin...this->end)
}

std::pair<const Candle &, bool> StockView::operator [](size_t index) const
{
    for (const auto &it : *this)    //перебираем все свечи от [this->begin...this->end) (не включая end)
        if (index-- == 0)           //можно было создать переменную и инкрементировать её, это тоже самое
            return std::make_pair(it, true);

    return std::make_pair(nullVector.front(), false);
}

}
