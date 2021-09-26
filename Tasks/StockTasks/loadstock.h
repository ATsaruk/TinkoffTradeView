/* Класс команды для получения данных по акции
 * Загрузка производится сначала из БД
 * Затем недостающие данные догружаются от брокера
 * Пример использования:
 *   auto *command = new CommandLoadStock;
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
#ifndef LOADSTOCK_H
#define LOADSTOCK_H

#include "Tasks/ibasecommand.h"
#include "Tasks/Interfaces/interfase.h"

#include "Data/range.h"
#include "Data/stocks.h"

namespace Task {


///Команда получения данных по акции
class LoadStock : public IBaseCommand
{
    Q_OBJECT

public:
    explicit LoadStock(const Data::StockKey &stockKey, const uint minCandleCount_ = 1);

    void setData(SharedInterface &inputData) override;
    SharedInterface &getResult() override;

protected:
    void exec() override;                   //запуск задачи
    void createLoadingTasks();              //подготовка интервалов загрузки
    void startNextTask();                   //запрос свечей из подинтервала subRange

protected slots:
    virtual void taskFinished() override;   //обработка завершения задачи

private:
    bool extraRangeLoaded;          //производистся загрузка дополнительного 2х недельного интервала
    uint minCandleCount;            //Минимальное число свечей, которое необходимо загрузить
    Data::Range existedRange;       //интервал на котором данные уже существуют
    InterfaceWrapper<Data::Range> range;            //исходный интервал загрузки
    InterfaceWrapper<Data::Stock> stock;            //список загруженных акций (из БД + от брокера)
};

}

#endif // LOADSTOCK_H
