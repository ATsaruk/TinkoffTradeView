#include "stocks.h"

#include <unordered_map>

#include "istocks.h"
#include "Core/globals.h"

namespace Data {

Stocks::Stocks()
{

}

Stocks::~Stocks()
{

}

std::pair<Range, size_t> Stocks::getRange(const StockKey &key) const
{
    if (auto record = stocks.find(key); record != stocks.end()) {
        auto mutex = const_cast<QReadWriteLock*>(&record->second.mutex);
        QReadLocker locker(mutex);
        return std::make_pair(record->second.range(), record->second.count());
    }

    return std::make_pair(Range(), 0);
}

void Stocks::candlesLoaded()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(const_cast<QObject*>(sender()));
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    emit newCandles(stock->key(), stock->range());
}

void Stocks::appedStock(Stock &stock)
{
    if (auto record = stocks.find(stock.key()); record != stocks.end()) {
        auto mutex = const_cast<QReadWriteLock*>(&record->second.mutex);
        QReadLocker locker(mutex);
        record->second.append(stock);
    } else
        stocks[stock.key()] = std::move(stock);
}

std::shared_ptr<StockViewReference<QReadLocker>> Stocks::getCandlesForRead(const StockKey &key, const QDateTime &begin, const QDateTime &end) const
{
    if (auto record = stocks.find(key); record != stocks.end())
        return std::make_shared<StockViewReference<QReadLocker>>(record->second, begin, end);

    return nullptr;
}

}
