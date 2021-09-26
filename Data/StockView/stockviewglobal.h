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
    StockViewGlobal(const StockKey &key, const Range &range = Range());

    std::vector<Candle>::const_iterator begin();
    std::vector<Candle>::const_iterator end();

    const std::vector<Candle>::const_iterator begin() const;
    const std::vector<Candle>::const_iterator end() const;

private:
    std::shared_ptr<StockViewReference<QReadLocker>> stock;
};

}

#endif // STOCKVIEWGLOBAL_H
