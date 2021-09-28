#ifndef ISTOCKS_H
#define ISTOCKS_H

#include <QObject>
#include <QReadLocker>

#include "Tasks/StockTasks/getstock.h"
#include "StockView/stockviewreference.h"

namespace Data {

/** @brief Интерфейс для просмотра свечных данных
  * @todo 1. при получении доступа к свечам должен блокироваться mutex и сам разблокироваться
  * @todo 2. определить механизм загрузки новых свечей
  */

class IStocks : public QObject
{
public:
    IStocks() {}
    virtual ~IStocks() {}

    /** @brief Возвращает доступный диапазон по акции
     *  @param key - ключ акции */
    virtual std::pair<Range, size_t> getRange(const StockKey &key) const = 0;

protected:
    //virtual CandlePtr getCandlesForWrite(const StockKey &key) = 0;
    using SharedStockVewRef = std::shared_ptr<Data::StockViewReference<QReadLocker>>;

    virtual void appedStock(Stock &stock) = 0;
    virtual SharedStockVewRef getCandlesForRead(const StockKey &key, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime()) const = 0;

signals:
    void newCandles(const StockKey &, const Range &);

    friend class Task::GetStock;
    friend class StockViewGlobal;
};

}

#endif // ISTOCKS_H
