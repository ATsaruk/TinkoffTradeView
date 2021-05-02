#ifndef INPUTINTERFACES_H
#define INPUTINTERFACES_H

#include "interfases.h"
#include "Data/range.h"
#include "Data/Stock/stockkey.h"

namespace Task {


struct InputRange : public InputInterface
{
    virtual void setData(const Data::Range &range) = 0;
    InterfaceType getInputInterfaceName() override { return InterfaceType::range; }
};

}

#endif // INPUTINTERFACES_H
