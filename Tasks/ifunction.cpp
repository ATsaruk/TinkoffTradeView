#include <QTypeInfo>

#include "ifunction.h"

#include "Core/globals.h"

namespace Task {


IFunction::IFunction(QString name)
{
    functionName = name;
    logDebug << QString("%1;%1();created!").arg(functionName);
}

IFunction::~IFunction()
{
    logDebug << QString("%1;~%1;destroyed!").arg(functionName);
}

QString IFunction::getName()
{
    return functionName;
}

bool IFunction::isFunction()
{
    return isFunc;
}


}
