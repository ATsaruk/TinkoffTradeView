#ifndef OUTPUTINTERFACES_H
#define OUTPUTINTERFACES_H

#include "interfases.h"
#include "Data/Stock/stocks.h"

namespace Task {


struct OutputCandles : public OutputInterface {
    virtual Data::Stock& getResult() = 0;
    InterfaceType getOutputInterfaceName() override { return InterfaceType::candles; }
};

}

#endif // OUTPUTINTERFACES_H
