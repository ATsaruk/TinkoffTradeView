#ifndef LOADSTOCKFROMDBFUNC_H
#define LOADSTOCKFROMDBFUNC_H

#include "Tasks/ifunction.h"

#include "Data/range.h"
#include "Data/Stock/stock.h"

namespace Task {


/** @ingroup Commands
  * @brief Функция загрузки свечей из БД за указанный временной интервал
  * @details Порядок действий:\n
  * п.1 Проверяет корректность исходных данных;\n
  * п.2 Получает свечную информацию за заданный интервал;\n
  * п.3 Если получено свечей меньше чем minCandlesCount, то запрашивает недостающее число свечей слева от запрошенного
  * интервала. */
class LoadStockFromDbFunc : public IFunction
{
public:
    /** @brief Конструктор, сохраняет переданные исходные данные
      * @param stockKey - ключ акции, для которой будет происходить загрузка
      * @param minCandlesCount - минимальное число свечей, которое необходимо загрузить */
    explicit LoadStockFromDbFunc(const Data::StockKey &stockKey, const uint minCandlesCount = 1);

    ///Сохраняет диапазон, в котором будет происходить загрузка (Data::Range)
    void setData(SharedInterface &inputData) override;

    void exec() override;

    ///Возвращает загруженные свечи (Data::Stock)
    SharedInterface &getResult() override;

private:
    uint _minCandlesCount;                       //минимальное число свечей, которое нужно загрузить
    InterfaceWrapper<Data::Range> _loadRange;    //интервал для загрузки
    InterfaceWrapper<Data::Stock> _stock;        //загруженные свечи
};

}

#endif // LOADSTOCKFROMDBFUNC_H
