/* Класс команды для получения данных по акции
 * Загрузка производится сначала из БД
 * Затем недостающие данные догружаются от брокера
 * Пример использования:
 *   auto *command = new GetStock;
 *
 * далее 1 из 4 нужных вариантов:
 *   command->setData(curStockKey, beginTime, endTime, 100);    //минимум 100 свечей
 *   command->setData(curStockKey, beginTime, endTime);         //если важен именно временной интервал (но не менее 1 свечи)
 *   command->setData(curStockKey, beginTime);                  //загрузка с beginTime и до текущего времени (но не менее 1 свечи)
 *   command->setData(curStockKey);                             //будет загружен maxLoadInterval для данной длинны сечи (но не менее 1 свечи)
 *
 *   connect(command, &CommandLoadStock::finished, this, &thisClass::addCandles);
 *   TaskManager::get()->registerTask(command);
 *
 * Если в указанном временном интервале будет свечей будет меньше запрошенного количества (minCandleCount), то временной интервал
 * будет смещаться влево пока не будет полученно нужно числ свечей, но не более чем на 2 недели
 */
#ifndef GETSTOCK_H
#define GETSTOCK_H

#include "Tasks/ibasecommand.h"
#include "Tasks/Interfaces/interfase.h"

#include "Data/range.h"
#include "Data/StockView/stockviewreference.h"

namespace Task {

///Команда получения данных по акции
class GetStock : public IBaseCommand
{
    Q_OBJECT

public:
    using SharedStockVewRef = std::shared_ptr<Data::StockViewReference<QReadLocker>>;

    explicit GetStock(const Data::StockKey &stockKey, const size_t minCandleCount_ = 1);

    void setData(SharedInterface &inputData) override;
    SharedInterface &getResult() override;

protected:
    void exec() override;                   //запуск задачи
    bool removeRange(const Data::Range &existed, const size_t count); //убирает существующий диапазон
    std::pair<Data::Range, size_t> loadFromDb();               //загружет недостающие свечи из БД
    void createLoadingTasks();              //подготовка интервалов загрузки
    void startNextTask();                   //запрос свечей из подинтервала subRange
    void createExtraRangeTasks();           //подготовка доп. 2 недельного интервала загрузки
    void finishTask();                      //завершает задачу

protected slots:
    virtual void taskFinished() override;   //обработка завершения задачи

private:
    bool extraRangeLoaded;      //производистся загрузка дополнительного 2х недельного интервала
    size_t candlesLeft;         //Число свечей, которое осталось загрузить
    Data::StockKey key;

    InterfaceWrapper<Data::Range> range;                //исходный интервал загрузки
    InterfaceWrapper<Data::Range> subRange;             //подинтервал для загрузки
    InterfaceWrapper<SharedStockVewRef> stock; //список загруженных акций (из БД + от брокера)
};

}

#endif // GETSTOCK_H
