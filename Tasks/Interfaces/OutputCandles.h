#ifndef OUTPUTCANDLES_H
#define OUTPUTCANDLES_H

#include "Data/Stock/candle.h"

namespace Task {


class OutputCandles
{
public:
    virtual Data::Candles& getCandles() = 0;
};

}

#endif // OUTPUTCANDLES_H
