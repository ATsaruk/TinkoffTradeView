#ifndef OUTPUTINTERFACES_H
#define OUTPUTINTERFACES_H

#include "interfases.h"
#include "Data/Stock/candle.h"

namespace Task {


struct OutputCandles : public OutputInterface {
    virtual Data::Candles& getResult() = 0;
    InterfaceType getOutputInterfaceName() override { return InterfaceType::candlesList; }
};

}

#endif // OUTPUTINTERFACES_H
