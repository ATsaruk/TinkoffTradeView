#ifndef STOCKVIEWREFERENCE_H
#define STOCKVIEWREFERENCE_H

#include "stockview.h"

#include "Data/Stock/stock.h"

namespace Data {


class StockViewReference : public StockView
{
public:
    StockViewReference(const Stock &baseStock, const QDateTime &begin, const QDateTime &end);

    const std::vector<Candle>::const_iterator begin() const;
    const std::vector<Candle>::const_iterator end() const;

private:
    const Stock &cRef;
};


}

#endif // STOCKVIEWREFERENCE_H
