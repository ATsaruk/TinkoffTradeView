#ifndef INPUTSTOCKKEY_RANGE_H
#define INPUTSTOCKKEY_RANGE_H

#include "Data/range.h"
#include "Data/Stock/stockkey.h"

namespace Task {


class InputStockKey_Range
{
public:
    virtual void setData(const Data::StockKey &stockKey, const Data::Range &range) = 0;
};

}

#endif // INPUTSTOCKKEY_RANGE_H
