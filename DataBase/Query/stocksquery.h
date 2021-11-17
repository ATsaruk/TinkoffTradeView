#ifndef STOCKSQUERY_H
#define STOCKSQUERY_H

#include "Data/range.h"
#include "Data/stocks.h"
#include "DataBase/idatabase.h"

namespace DB {

using namespace Data;

/** @ingroup DataBase
  * @brief Класс запросов свечной информации из БД */
class StocksQuery
{
public:
    explicit StocksQuery();

    /** @brief Сохраняет список свечей в БД
      * @param stock - акция, свечи которой будут сохранены */
    static void insertCandles(const Stock &stock);

    /** @brief Загружает свечи из базы данных (таблица stocks) в структуру candles
      * @param stock - акция, в которую будут добавлены загруженные свечи
      * @param begin - дата свечи (>=) с которой начнется загрузки из БД
      * @param end - дата свечи (<=) до которой будет идти загрузка из БД
      * @param candleCount - минимальное число свечей, которое должно быть загружено
      * @warning если указано candleCount, то по факту дата начала будет проигнорирована и будет загружено candleCount
      *          свечей, дата последней свечи будет <= end */
    static void loadCandles(Stock &stock, const Range &range, const uint loadCandlesCount = 0);
};

}

#endif // STOCKSQUERY_H
