/** @ingroup Task
  * @defgroup Commands */

#ifndef GETSTOCK_H
#define GETSTOCK_H

#include "Tasks/ibasecommand.h"
#include "Tasks/Interfaces/interfase.h"

#include "Data/range.h"
#include "Data/StockView/stockreference.h"

namespace Task {

/** @ingroup Commands
  * @brief Команда получения свечных данных по акции
  *
  * Сначала проверяются уже загруженные свечи (в Glo.stocks);\n
  * Далее производится попытка загрузить недостающие свечи из БД;\n
  * Полученные из БД свечи помещаются в Glo.stocks;\n
  * Оставшиеся свечи запрашиваются у брокера;\n
  * Полученные у брокера свечи сохраняются в БД и в Glo.stocks.\n
  * Пример использования: @code
  *   InterfaceWrapper<Data::Range> range (begin, end);
  *   auto *command = new NEW_TASK<GetStock>(range, stockKey);
  *   connect(command, &Task::GetStock::finished, this, &ThisClass::onFinished); @endcode */
class GetStock : public IBaseCommand
{
    Q_OBJECT

public:
    using SharedStockVewRef = const Data::StockReference<QReadLocker>;

    /** @brief Конструктор, сохраняет начальные данные
      * @param stockKey - ключ акции для загрузки
      * @param minCandleCount - минимальное число свечей, которое должны быть загружно */
    explicit GetStock(const Data::StockKey &stockKey, const size_t minCandleCount = 1);

    ///Получение диапазона, в котором должна вестись загрузка (Data::Range)
    void setData(SharedInterface &inputData) override;

    ///Возвращает запрашиваемые свечи (SharedStockVewRef)
    SharedInterface &getResult() override;

protected:
    ///Запускает задачу
    void exec() override;

    /** @brief Проверяет условие завершения загрузки
      * @param loadFromBrockerComplete - признак того, что загрузка от брокера уже производилась
      * @return Возвращает TRUE - если все запрашиваемые свечи загружены, FALSE - требуется продолжение загрузки */
    bool isEnoughCandles(const bool loadFromBrockerComplete);

    ///Загружет недостающие свечи из БД
    /** @brief Загружет недостающие свечи из БД
      * @return TRUE - если загружено достаточно свечей, FALSE - требует загрузка от брокера */
    bool loadFromDb();

    ///Создает задачи для загрузки недостающих свечей от брокера
    void startLoading();

    /** @brief Продолжает загрузку от брокера
      * @details Если после основной загрузки от брокера, все равно недостаточно свечей, то запускает загрузку
      * дополнительного 2 недельного интервала. */
    void startNextTask();

    ///Создает задачи для загрузки дополнительного 2 недельного интервала
    void createExtraRangeTasks();

    ///завершает задачу
    void finishTask();

protected slots:
    ///Обрабатка сигнала о завершении текущей задачи загрузки от брокера
    void taskFinished() override;

private:
    bool extraRangeLoaded;      //производистся загрузка дополнительного 2х недельного интервала
    size_t loadedCount;         //Число загруженный свечей
    size_t minCandleCount;      //минимальное число свечей, которое нужно загрузить
    Data::StockKey key;

    InterfaceWrapper<Data::Range> range;            //исходный интервал загрузки
    InterfaceWrapper<Data::Range> subRange;         //подинтервал для загрузки
    InterfaceWrapper<SharedStockVewRef> stock;      //список загруженных акций (из БД + от брокера)
};

}

#endif // GETSTOCK_H
