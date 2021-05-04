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

#include <QObject>

#include "ibasecommand.h"

namespace Task {


/** @brief Класс управляющий задачами
  * @warning Класс нарушает принцип Барбары Лисков, его нельзя использоваться в качестве Task или Command
  * @details Класс предназначен для запуска других задач/комманд. Что бы задачу можно было запустить с помощью
  * addTask<TaskClass>(args...), у задачи должна быть реализована функция setData(...).
  */
class Manager : public QObject
{
public:
    explicit Manager(QObject *parent = nullptr);
    ~Manager();

    //Регистрирует задачу
    void registerTask(IBaseTask *newTask);

    /* Создаёт новую задачу
    *  T - класс добавляемой задачи, наследник IFunction
    *  inputData - входные данные необходимые для функции setData задачи
    *  N...args - аргументы необходимые для функции конструктора задачи
    *  Возвращает ссылку на созданную задачу (например для подключения к сигналу finished)
    *  Пример:
    *    Task::Manager::get()->createTask <LoadStock> (rangeInterface, stockKey, candleCount);
    */
    template<class T, typename... N>
    std::enable_if_t<std::is_base_of_v<IBaseTask, T>, T*>
    createTask(SharedInterface &inputData, N ... args)
    {
        QMutexLocker locker(&mutex);
        T *newTask = new T(args ...);
        newTask->setData(inputData);
        newTask->setThread(nullptr);
        registerTask(newTask);
        return newTask;
    }

signals:
    void stopAll();

protected:
    void runNextTask();

protected slots:
    //Обработывет завершение работы задачи
    virtual void taskFinished();

private:
    Q_OBJECT

    QRecursiveMutex mutex;
    QQueue<IBaseTask*> taskList;  //очередь задач на запуск

    const uint16_t maxTaskCount;
    uint16_t taskCount = 0;   //общее кол-во запущенных задач
};

}

#endif // MANAGER_H
