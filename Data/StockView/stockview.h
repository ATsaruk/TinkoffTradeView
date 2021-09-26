#ifndef STOCKVIEW_H
#define STOCKVIEW_H

#include "vector"
#include "Data/range.h"
#include "Data/Stock/candle.h"

namespace Data {


class StockView
{
public:
    StockView();
    virtual ~StockView();

    const Range getRange() const;
    size_t size() const;

    std::pair<const Candle*, bool> operator [](size_t index);
    std::pair<const Candle&, bool> operator [](size_t index) const;

    virtual std::vector<Candle>::const_iterator begin() = 0;
    virtual std::vector<Candle>::const_iterator end() = 0;

    virtual const std::vector<Candle>::const_iterator begin() const = 0;
    virtual const std::vector<Candle>::const_iterator end() const = 0;

protected:
    Range range;
    std::vector<Candle> nullVector;
};


}

#endif // STOCKVIEW_H
