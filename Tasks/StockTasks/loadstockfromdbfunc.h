#ifndef LOADSTOCKFROMDBFUNC_H
#define LOADSTOCKFROMDBFUNC_H

#include "Tasks/ifunction.h"

#include "Data/range.h"
#include "Data/Stock/stocks.h"

namespace Task {


class LoadStockFromDbFunc : public IFunction
{
public:
    explicit LoadStockFromDbFunc(const Data::StockKey &stockKey, const uint minCandlesCount_ = 1);

    void exec() override;
    void setData(SharedInterface &inputData) override;
    SharedInterface &getResult() override;

protected:
    void loadByCount();

private:
    uint minCandlesCount;
    InterfaceWrapper<Data::Range> loadRange;
    InterfaceWrapper<Data::Stock> stock;
};

}

#endif // LOADSTOCKFROMDBFUNC_H
