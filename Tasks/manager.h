/* Класс для запуска задач/комманд: IBaseTask/CustomCommand
 * Предварительно рекомендую ознакомиться с классами IBaseTask и CustomCommand
 *
 * Есть 2 пути регистрации задачи:
 * 1. Регистрация комманды/задачи для которой создан класс, тут будет реальный пример запуска задачи GetStock:
 *     auto *command = TaskManager::get()->addTask <GetStock> (stockKey, beginDate, endDate, minCandleCount);
 *     или с учетом #define NEW_TASK TaskManager::get()->addTask
 *     auto *command = NEW_TASK <GetStock> (stockKey, beginDate, endDate, minCandleCount);
 *     connect(command, &GetStock::finished, this, &ChartCandlesGroup::addCandles);
 *
 * 2. Если команда сформирована "находу", то первы вариант не подходит, то второй вариант:
 *     auto *command = new CustomCommand("LoadStock");
 *     command->addTask<LoadStockFromDbFunc>(stockKey, beginDate, endDate, minCandleCount);
 *     command->addTask<LoadStocksFromBroker>(stockKey, beginDate, endDate, minCandleCount);
 *     connect(command, &CommandLoadStock::finished, this, &ChartCandlesGroup::addCandles);
 *     TaskManager::get()->registerTask(command);
 *
 * TaskManager при получении задачи сразу пытается её запустить, конечно если текущее количество задач не больше чем
 * maxTaskCount, если больше, то запуск производится про принципу очереди FIFO. */

#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>

#include "ibasecommand.h"

namespace Task {


/** @ingroup Task
  * @brief Класс управляющий задачами
  * @details Класс предназначен для запуска задач/комманд наследников IBaseTask.
  * @see IBaseTask, IBaseCommand */
class Manager : public QObject
{
public:
    explicit Manager(QObject *parent = nullptr);
    ~Manager();

    ///Регистрирует задачу/команду, и если возможно сразу её запускает
    void registerTask(IBaseTask *newTask);

    /** @brief Создаёт новую задачу
      * @param T - класс добавляемой задачи, наследник IFunction
      * @param inputData - входные данные необходимые для функции setData задачи
      * @param args - аргументы необходимые для функции конструктора задачи
      * @return Возвращает ссылку на созданную задачу (например для подключения к сигналу finished)
      * Пример:
      *  С учетом globals.h: #define TaskManager Core::Globals::get().taskManager
      *    TaskManager->createTask <LoadStock> (rangeInterface, stockKey, candleCount);
      *  или с учетом globals.h: #define NEW_TASK Core::Globals::get().taskManager->createTask
      *    NEW_TASK <LoadStock> (rangeInterface, stockKey, candleCount); */
    template<class T, typename... N>
    std::enable_if_t<std::is_base_of_v<IBaseTask, T>, T*>
    createTask(SharedInterface &inputData, N ... args)
    {
        QMutexLocker locker(&_mutex);
        T *newTask = new T(args ...);
        newTask->setData(inputData);
        newTask->setThread(nullptr);
        registerTask(newTask);
        return newTask;
    }

signals:
    ///Сигнал остановки всех задач
    void stopAll();

protected:
    ///запуск следующей задачи в очереди
    void runNextTask();

protected slots:
    ///Обработывет завершение работы задачи
    virtual void taskFinished();

private:
    Q_OBJECT

    QRecursiveMutex _mutex;
    QQueue<IBaseTask*> _taskList;    //очередь задач на запуск

    const uint16_t _maxTaskCount;    //максимальное число одновременно работающих задач
    uint16_t _taskCount = 0;         //текущее кол-во запущенных задач
};

}

#endif // MANAGER_H
