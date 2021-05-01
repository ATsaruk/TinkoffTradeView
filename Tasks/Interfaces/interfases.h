#ifndef INPUTINTERFASE_H
#define INPUTINTERFASE_H

#include <QString>

namespace Task {


enum class InterfaceType : uint8_t
{
    empty,
    stockKeyAndRange,
    candlesList
};


class InputInterface
{
public:
    virtual InterfaceType getInputInterfaceName() { return InterfaceType::empty; }
};

class OutputInterface
{
public:
    virtual InterfaceType getOutputInterfaceName() { return InterfaceType::empty; }
};

}

#endif // INPUTINTERFASE_H
