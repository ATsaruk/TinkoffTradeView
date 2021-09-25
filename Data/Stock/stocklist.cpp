#include "stocklist.h"

#include <unordered_map>

#include "istocks.h"
#include "Core/globals.h"
#include "Tasks/StockTasks/loadstock.h"
#include "StockView/stockviewreference.h"

namespace Data {

StockList::StockList()
{

}

StockList::~StockList()
{

}

std::optional<const Candle *> StockList::getCandle(const StockKey &key, const QDateTime &time)
{
    if (const auto &candles = stocks.find(key); candles != stocks.end())
        return candles->second.find(time);
    return std::nullopt;
}

bool StockList::checkCandles(const StockKey &key, const Range &range)
{
    //Проверяем наличие свечей в запрашиваемом диапазоне
    Range existedRange;
    if (const auto &candles = stocks.find(key); candles != stocks.end()) {
        existedRange = candles->second.range();
        if (existedRange.contains(range))
            return true;
    }

    //Свечей недостаточно, инициализируем загрузку
    Task::InterfaceWrapper<Range> loadRange = range.remove(existedRange);
    auto *command = TaskManager->createTask<Task::LoadStock>(&loadRange, key);
    connect(command, &Task::LoadStock::finished, this, &StockList::candlesLoaded);

    return false;
}

Range StockList::insert(Stock &newCandles)
{
    if (auto stock = stocks.find(newCandles.key()); stock != stocks.end())
        return stock->second.append(newCandles);

    return Range();
}

void StockList::candlesLoaded()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(const_cast<QObject*>(sender()));
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    //addCandles( std::move(stock->candles) );
    emit newCandles(stock->key(), stock->range());
}

std::shared_ptr<StockViewReference> StockList::getCandlesForRead(const StockKey &key, const QDateTime &begin, const QDateTime &end) const
{
    ///@todo !!!блокировать mutex в конструкторе StockViewReference!
    if (const auto &record = stocks.find(key); record != stocks.end())
        return std::make_shared<StockViewReference>(record->second, begin, end);

    return nullptr;
}

}
