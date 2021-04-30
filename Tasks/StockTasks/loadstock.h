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

#include "Tasks/customcommand.h"
#include "Tasks/Interfaces/OutputCandles.h"
#include "Tasks/Interfaces/InputStockKey_Range.h"

#include "Data/range.h"
#include "Data/Stock/candle.h"
#include "Data/Stock/stockkey.h"

namespace Task {
using namespace Data;


///Команда получения данных по акции
class LoadStock : public CustomCommand, public InputStockKey_Range, public OutputCandles
{
public:
    explicit LoadStock(QThread *parent = nullptr);

    //Задать данные для запуска задачь
    void setData(const StockKey &stockKey, const Range &range = Range()) override;

    void setMinCandleCount(const uint minCandleCount_);

    //Возвращает имя задачи
    QString getName() override;

    Candles& getCandles() override;

protected:
    void exec() override;

    void loadFromBroker(const Range &range);

protected slots:
    //Обработка завершения потока
    virtual void taskFinished() override;

private:
    Candles candles;
    StockKey key;         //Ключ акции
    Range loadRange;      //Загружаемый итнервал
    uint minCandleCount = 1;
};

}

#endif // LOADSTOCK_H
