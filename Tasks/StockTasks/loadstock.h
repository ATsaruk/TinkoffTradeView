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
#include "Data/Stock/stocks.h"

namespace Task {
using namespace Data;


///Команда получения данных по акции
class LoadStock : public IBaseCommand
{
public:
    explicit LoadStock(const StockKey &stockKey, const uint minCandleCount_ = 1);

    void setData(SharedInterface &inputData) override;
    SharedInterface &getResult() override;

protected:
    void exec() override;
    void loadForwardFromBroker();
    void loadBackwardFromBroker();
    void startLoading();
    void finishLoading();
    void receiveResult(QObject *sender);
    bool isLoadFinished();

protected slots:
    //Обработка завершения потока
    virtual void taskFinished() override;

private:
    InterfaceWrapper<Range> range;
    InterfaceWrapper<Stock> stock;
    InterfaceWrapper<Stock> loadedCandles;
    bool forwardLoading = false;
    uint minCandleCount;
    QDateTime endLoadDate;

};

}

#endif // LOADSTOCK_H
