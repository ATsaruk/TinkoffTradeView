#ifndef STOCKVIEWGLOBAL_H
#define STOCKVIEWGLOBAL_H

#include "stockview.h"
#include "stockviewreference.h"

#include "Data/Stock/stockkey.h"

namespace Data {


class StockViewGlobal : public StockView
{
public:
    StockViewGlobal(const StockKey &key, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime());

    const std::vector<Candle>::const_iterator begin() const;
    const std::vector<Candle>::const_iterator end() const;

private:
    std::shared_ptr<StockViewReference> stock;
};

}

#endif // STOCKVIEWGLOBAL_H
