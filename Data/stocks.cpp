#include "stocks.h"

namespace Data {

Stocks::Stocks()
{

}

Stocks::~Stocks()
{

}

std::pair<Range, size_t> Stocks::getRange(const StockKey &key) const
{
    if (auto record = _stocks.find(key); record != _stocks.end()) {
        auto mutex = const_cast<QReadWriteLock*>(&record->second->mutex);
        QReadLocker locker(mutex);
        return std::make_pair(record->second->range(), record->second->size());
    }

    return std::make_pair(Range(), 0);
}

void Stocks::appedStock(Stock &stock)
{
    if (auto record = _stocks.find(stock.key()); record != _stocks.end()) {
        QReadLocker locker(&record->second->mutex);
        record->second->append(stock);
    } else
        _stocks[stock.key()] = QSharedPointer<Stock>::create(std::move(stock));
}

Stocks::SharedStockVewRef Stocks::getCandlesForRead(const StockKey &key, const Range &range, const size_t minCandlesCount)
{
    /*for ( const auto &it : stocks )
        if (it.first == key)
            return SharedStockVewRef::create(it.second, begin, end, minCandlesCount);*/
    if (_stocks.find(key) == _stocks.end())
        return nullptr;

    return SharedStockVewRef::create(_stocks[key], range, minCandlesCount);
}

}
