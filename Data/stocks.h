/** @defgroup Data
  * @brief Модуль хранящий данные
  * @author Царюк А.В.
  * @date Сентябрь 2021 года */

#ifndef STOCKS_H
#define STOCKS_H

#include <unordered_map>

#include "Stock/stock.h"
#include "Stock/stockview.h"
#include "Tasks/StockTasks/getstock.h"

namespace Data {


/** @ingroup Data
  * @brief Класс хранящий свечные данные для акции с заданным ключем
  *
  * Данный класс предназначен только для дружественных классов.
  * Для доступа к свечной информации служит задача @see Tasks::GetStcok */
class Stocks
{
public:
    explicit Stocks();
    ~Stocks();

protected:
    using SharedStockVewRef = QSharedPointer<Data::StockView>;

    /** @brief Возвращает доступный диапазон по акции и количество доступных свечей
      * @param key - ключ акции */
    std::pair<Range, size_t> getRange(const StockKey &key) const;

    /** @brief Сохраняет данные по акции
      * @param stock - акция, данные которой нужно сохранить
      * @warning функция использую move симантику, и "забирает" новые свечи из массива stock->candles
      *
      * 1. Если в Glo.stocks отсутсвует акция с ключем stock.key(), то сохраняет всю акцию целиком, "забирая" все
      *    свечи из исходной акции stock
      * 2. Если в Glo.stocks уже есть акция с ключем stock.key(), то "забирает" только новые свечи */
    void appedStock(Stock &stock);

    /** @brief Возвращает свечи из запрошенного интервала
      * @param[IN] key - ключ акции
      * @param[IN] range - запрашиваемый интервал
      * @param[IN] minCandlesCount - минимальное число свечей, которое должен содержать результирующий массив
      * @return Возвращает обертку над акцией с ключем key, обертка ограничивает массив запрошенным интервалом/числом свечей
      *
      * Формирует диапазон range, удовлетворяющий заданным параметрам и возвращает обертку SharedStockVewRef для акции
      * с ключем key.
      * Правила формирования диапазона:
      * 1. Дата последней свечи будет соотвествовать upper_bound, если такой свечи нет будет возвращет candles.end() для
      *    того, что бы корректно работали stl алгоритмы
      * 2. Дата первой свечи будет соотвествовать lower_bound в том случае, если minCandlesCount = 0, т.е. нет
      *    дополнительного условия на минимальное количество свечей.
      * 3. Если minCandlesCount > 0, то датой первой свечи будет свеча с номером last_candle_index - minCandlesCount,
      *    где last_candle_index это индекс свечи с датой последней свечи. */
    SharedStockVewRef getCandlesForRead(const StockKey &key,
                                        const Range &range = Range(),
                                        const size_t minCandlesCount = 0);

private:
    std::unordered_map<StockKey, QSharedPointer<Stock>> _stocks; /// Список акций

    friend class Task::GetStock;
};

}

#endif // STOCKS_H
