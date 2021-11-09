#ifndef IBASECOMMAND_H
#define IBASECOMMAND_H

#include <QQueue>
#include <QRecursiveMutex>

#include "ibasetask.h"
#include "Tasks/Interfaces/interfase.h"


#include <QMetaMethod>


namespace Task {

/** @ingroup Task
  * @brief Класс комманды, которая может в себе содержать другие задачи/комманды
  * @see IBaseTask */
class IBaseCommand : public IBaseTask
{
    Q_OBJECT

public:
    explicit IBaseCommand(const QString &commandName);
    ~IBaseCommand();

    void setData(SharedInterface &inputData) override;
    SharedInterface &getResult() override;

    //Реристрирует задачу
    virtual void registerTask(IFunction *newTask) final;

    /* Создаёт и запускает функцию
    *  T - класс добавляемой задачи, наследник IFunction
    *  N...args - аргументы необходимые для функции конструктора задачи
    *  Возвращает ссылку на созданную задачу (например для подключения к сигналу finished)
    *  Пример:
    *    TaskManager::get()->addTask <LoadStock> (rangeInterface, stockKey, candleCount);
    */
    template <class T, typename... N>
    std::enable_if_t<std::is_base_of_v<IFunction, T>, T*>
    execFunc(SharedInterface &inputData, N ... args)
    {
        T *newFunc = new T(args ...);
        assert(newFunc->isFunction() && "Can't exec task! use createTask!");
        newFunc->setData(inputData);
        newFunc->exec();
        newFunc->deleteLater();
        return newFunc;
    }

    /* Создаёт новую задачу
    *  T - класс добавляемой задачи, наследник IFunction
    *  N...args - аргументы необходимые для функции конструктора задачи
    *  Возвращает ссылку на созданную задачу (например для подключения к сигналу finished)
    *  Пример:
    *    TaskManager::get()->addTask <LoadStock> (rangeInterface, stockKey, candleCount);
    */
    template <class T, typename... N>
    std::enable_if_t<std::is_base_of_v<IFunction, T>, T*>
    createTask(N ... args)
    {
        T *newTask = new T(args ...);
        newTask->setThread(taskThread);
        registerTask(newTask);
        return newTask;
    }

signals:
    //Сигнал остановки всех задач
    void stopAll();

protected:
    //Основной цикл выполнения задачи
    void exec() override;

    //Остановить задату
    void stop() override final;

    //Запуск очередной задачи
    virtual void runNextTask(IFunction *previousTask = nullptr) final;

    virtual void startTask(IBaseTask *task) final;

protected slots:
    //Обработка завершения потока
    virtual void taskFinished();

protected:
    //QRecursiveMutex mutex;
    QQueue<IFunction*> taskList;  //очередь задач на запуск

private:
    QScopedPointer<IFunction> _lastCompleteTask;
};

}

#endif // IBASECOMMAND_H
