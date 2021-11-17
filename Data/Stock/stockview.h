#ifndef STOCKVIEW_H
#define STOCKVIEW_H

#include "stock.h"

namespace Data {

class StockView : protected Stock
{
public:
    /** @brief Формирует временной интервал range, в котором будут доступны свечи
      * @param baseStock - акция, к которой предоставляется доступ
      * @param targetRange - интервал, в котором будут доступны свечи
      * @param minCandlesCount - минимальное количество доступных свечей
      * @warning класс бросает исключения в следующих случаях:\n
      * 1. если передать валидный targetRange и minCandlesCount > 0\n
      * 2. если передать невалидный targetRange и minCandlesCount == 0\n
      * класс ожидает или конкретный дипазон, с 0 minCandlesCount, если же minCandlesCount > 0, то или targetRange.begin()
      * или targetRange.end() должен быть null() что бы было понятно в какую сторону расширять интервал!\n
      * если и begin() и end() is null() то, диапазон будет сформирован из minCandlesCount свечей с конца доступных
      * свечей Stock::_candles */
    explicit StockView(QSharedPointer<Stock> &stock, const Range &targetRange = Range(), const size_t minCandlesCount = 0);

    ///Конструктор копирования
    StockView(const StockView &other);

    ///Возвращает ключ акции
    const StockKey &key() const override;

    ///Возвращает диапазон хранимых свечей
    Range range() const override;

    ///Возвращает количество хранимых свечей
    size_t size() const override;

    ///см. Stock::isEnoughCandles
    bool isEnoughCandles(Range range, const size_t minCandleCount, const bool ignoreRightBorder = false) const override;

    ///итератор на первую свечу, время которой не меньше (lower_bound), чем range.getBegin()
    const DequeIt begin() const override;

    ///итератор на свечу, время которой больше (upper_bound), чем range.getEnd()
    const DequeIt end() const override;

    ///реверс итератор на первую свечу
    const ReverseDequeIt rbegin() const override;

    ///реверс итератор на последнюю свечу
    const ReverseDequeIt rend() const override;

    ///изменить дату начала просматриваемого интервала
    void setBegin(const QDateTime &time);

    ///изменить дату конца просматриваемого интервала
    void setEnd(const QDateTime &time);

protected:
    ///Возвращает const итератор на элемент, дата которого не меньше чем time
    DequeIt lower_bound(const QDateTime &time) const;

    ///Возвращает const итератор на элемент, дата которого больше чем time
    DequeIt upper_bound(const QDateTime &time) const;

private:
    QReadLocker locker; //локер, блокирующий доступ к акции на чтение
    Range _range;       //хранит ограничивающий интервал, т.е. интервал, в которым доступны свечи через данный интерфейс
};

}

#endif // STOCKVIEW_H
