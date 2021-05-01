#ifndef INPUTINTERFACES_H
#define INPUTINTERFACES_H

#include "interfases.h"
#include "Data/range.h"
#include "Data/Stock/stockkey.h"

namespace Task {


struct InputStockKeyAndRange : public InputInterface
{
    virtual void setData(const Data::StockKey &stockKey, const Data::Range &range) = 0;
    InterfaceType getInputInterfaceName() override { return InterfaceType::stockKeyAndRange; }
};

}

#endif // INPUTINTERFACES_H
