/* Класс задачи по загрузке свечных данных из базы данных
 * Пример использования:
 *   auto *loadCandlesTask = new TaskLoadStocksFromDb;
 *
 * далее 1 из 4 нужных вариантов:
 *   loadCandlesTask->setData(curStockKey, beginTime, endTime, 100);    //минимум 100 свечей
 *   loadCandlesTask->setData(curStockKey, beginTime, endTime);         //если важен именно временной интервал (но не менее 1 свечи)
 *   loadCandlesTask->setData(curStockKey, beginTime);                  //загрузка с beginTime и до текущего времени (но не менее 1 свечи)
 *   loadCandlesTask->setData(curStockKey);                             //будет загружен maxLoadInterval для данной длинны сечи (но не менее 1 свечи)
 *
 *   taskManager->registerTask(loadCandlesTask);
 *
 * Клас получает и захватывает доступ к свечным данным из класса DataStocks
 * Далее проверяет существуют ли акция с ключем stockKey в списке Data::getStockList()
 * Елси такая акция уже есть, то данные из БД будет загружаться в этот же список, дополняя его
 * Если такой акции нет, то создает новую акцию и добавляет его в Data::stockList и производит загрузку в него
 * После загрузки отправляет сигнал о завершении задачи и создает заявку на удаление данного экземпляра класса
 */

#ifndef LOADSTOCKFROMDB_H
#define LOADSTOCKFROMDB_H

#include "Tasks/ibasetask.h"
#include "Data/range.h"
#include "Data/Stock/stockkey.h"

namespace Task {
using namespace Data;

///Задача загрузки свечей от брокеры за указанный временной интервал
class LoadStockFromDb : public IBaseTask
{
public:
    explicit LoadStockFromDb(QThread *parent = nullptr);
    ~LoadStockFromDb();

    //Задание исходных данных для загрузки
    void setData(const StockKey &stockKey, const Range &range, const uint minCandleCount = 1);

    //Возвращает имя класса владельца
    QString getName() override;

protected:
    uint minCount;        //Минимальное количество свечей, которое должно быть загружено
    Range loadRange;      //Загружаемый итнервал
    StockKey key;         //Ключ акции

    //Запустить задачу
    void exec() override;
};

}

#endif // LOADSTOCKFROMDB_H
