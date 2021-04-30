/* Класс для запуска задач/комманд: IBaseTask/CustomCommand
 * Предварительно рекомендую ознакомиться с классами IBaseTask и CustomCommand
 *
 * Есть 2 пути регистрации задачи:
 * 1. Регистрация комманды/задачи для которой создан класс, тут будет реальный пример запуска задачи CommanLoadStock:
 *     auto *command = TaskManager::get()->addTask <CommanLoadStock> (stockKey, beginDate, endDate, minCandleCount);
 *     или с учетом #define NEW_TASK TaskManager::get()->addTask
 *     auto *command = NEW_TASK <CommanLoadStock> (stockKey, beginDate, endDate, minCandleCount);
 *     connect(command, &CommandLoadStock::finished, this, &ChartCandlesGroup::addCandles);
 *
 * 2. Если команда сформирована "находу", то первы вариант не подходит, то второй вариант:
 *     auto *command = new CustomCommand("LoadStock");
 *     command->addTask<TaskLoadStockFromDb>(stockKey, beginDate, endDate, minCandleCount);
 *     command->addTask<TaskLoadStocksFromBroker>(stockKey, beginDate, endDate, minCandleCount);
 *     connect(command, &CommandLoadStock::finished, this, &ChartCandlesGroup::addCandles);
 *     TaskManager::get()->registerTask(command);
 *
 * TaskManager при получении задачи сразу пытается её запустить, исключением являются ресурсоемкие задачи heavyTasks
 * ресурсоемких задач одновременно может быть запущенно не более getMaxExecTask() (функция вернет количество ядер в системе)
 * При завершении ресурсоемкой задачи будет запущена следующая задача.
 */
#ifndef MANAGER_H
#define MANAGER_H

#include "customcommand.h"

namespace Task {


/** @brief Класс управляющий задачами
  * @warning Класс нарушает принцип Барбары Лисков, его нельзя использоваться в качестве Task или Command
  * @details Класс предназначен для запуска других задач/комманд. Что бы задачу можно было запустить с помощью
  * addTask<TaskClass>(args...), у задачи должна быть реализована функция setData(...).
  */
class Manager final : public CustomCommand
{
    Q_OBJECT

public:
    explicit Manager(QThread *parent);

    //Возвращает имя задачи
    QString getName() override;

    //Регистрация новой заявки
    void registerTask(IBaseTask *newTask) override;

    //Запуск очередной задачи
    virtual void runNextTask() override;

protected slots:
    //Обработка завершения работы потока
    virtual void taskFinished() override;

private:
    QRecursiveMutex mutex;
    uint16_t taskCount = 0;   //общее кол-во запущенных задач
    uint16_t maxTaskCount;

    //Функция перенесена в секцию private, что бы нельзя было зарегестрировать задачу класса Manager
    QThread* getThread() override;
};

}

#endif // MANAGER_H
