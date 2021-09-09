#ifndef PRICEAXIS_H
#define PRICEAXIS_H

#include "numericaxis.h"

namespace Plotter {


class PriceAxis : public NumericAxis
{
public:
    explicit PriceAxis(qreal range = 1, qreal offset = 0);
};

}

#endif // PRICEAXIS_H
