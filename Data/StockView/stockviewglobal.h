#ifndef STOCKVIEWGLOBAL_H
#define STOCKVIEWGLOBAL_H

#include "stockview.h"
#include "stockreference.h"

#include "Data/Stock/stockkey.h"

namespace Data {

/** @ingroup StockView
  * @brief Обертка, предоставляющая доступ к свечам из Glo.stocks
  * @warning Если временной интервал range сформированный в конструкторе не валидный (!range.isValid()), то функции,
  * возвращающие итераторы будут возвращать nullVector.end() */
class StockViewGlobal : public StockView
{
public:
    /** @brief Формирует указатель на запрашиваемую акции и временные границы range
      * @param key - ключ акции
      * @param begin - дата начала интервала, если не задано, то используется дата первой доступной свечи
      * @param end - дата конца интервала, если не задано, то используется дата последней доступной свечи
      * @param minCandlesCount - минимальное число свечей, которое должно содержаться в диапазоне range, если не задано,
      *                          то range будет сформирован только с учетом заданных [begin .. end]
      *
      * Подробнее про правила формирования range, смотри:
      * @see Data::Stocks::getCandlesForRead */
    StockViewGlobal(const StockKey &key, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime(), const size_t minCandlesCount = 0);

    ///Возвращает const итератор на элемент, дата которого не меньше чем time
    ConstDequeIt lower_bound(const QDateTime &time) const override;

    ///Возвращает const итератор на элемент, дата которого больше чем time
    ConstDequeIt upper_bound(const QDateTime &time) const override;

    ///итератор на первую свечу, время которой не меньше (lower_bound), чем range.getBegin()
    ConstDequeIt begin() const override;
    ///итератор на свечу, время которой больше (upper_bound), чем range.getEnd()
    ConstDequeIt end() const override;

private:
    QSharedPointer<const StockReference<QReadLocker>> stock;     //указатель на StockViewReference(Glo.stocks[key])
};

}

#endif // STOCKVIEWGLOBAL_H
