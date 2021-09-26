#include "stocks.h"

#include <unordered_map>

#include "istocks.h"
#include "Core/globals.h"
#include "Tasks/StockTasks/loadstock.h"

namespace Data {

Stocks::Stocks()
{

}

Stocks::~Stocks()
{

}

std::optional<const Candle *> Stocks::getCandle(const StockKey &key, const QDateTime &time)
{
    if (const auto &candles = stocks.find(key); candles != stocks.end())
        return candles->second.find(time);
    return std::nullopt;
}

bool Stocks::checkCandles(const StockKey &key, const Range &range)
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
    connect(command, &Task::LoadStock::finished, this, &Stocks::candlesLoaded);

    return false;
}

Range Stocks::insert(Stock &newCandles)
{
    if (auto stock = stocks.find(newCandles.key()); stock != stocks.end())
        return stock->second.append(newCandles);

    return Range();
}

void Stocks::candlesLoaded()
{
    Task::IBaseTask *task = dynamic_cast<Task::IBaseTask*>(const_cast<QObject*>(sender()));
    Task::InterfaceWrapper<Data::Stock> stock = task->getResult();
    //addCandles( std::move(stock->candles) );
    emit newCandles(stock->key(), stock->range());
}

std::shared_ptr<StockViewReference<QReadLocker>> Stocks::getCandlesForRead(const StockKey &key, const QDateTime &begin, const QDateTime &end) const
{
    ///@todo !!!блокировать mutex в конструкторе StockViewReference!
    if (auto record = stocks.find(key); record != stocks.end())
        return std::make_shared<StockViewReference<QReadLocker>>(record->second, begin, end);

    return nullptr;
}

}
