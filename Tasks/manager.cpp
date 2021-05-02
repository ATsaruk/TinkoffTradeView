#include <QThread>

#include "manager.h"
#include "Core/globals.h"

namespace Task {

Manager::Manager()
    : CustomCommand()
{
    setThread(nullptr);

    //Обнуляем taskThread, если этого не сделать, то при вызове addTask() для новых задач, им будет передан taskThread
    //в качестве родительского потока и они будут создаваться внутри этого потока, а нам нужно что бы каждая задача
    //была отдельным потоком, и обнулением мы лишаем их родительского потока и они создадут свои потоки
    taskThread = nullptr;

    maxTaskCount = QThread::idealThreadCount() * 2;
    logDebug << QString("TaskManager;TaskManager();+constructor!");
}

QString Manager::getName()
{
    return "TaskManager";
}

void Manager::registerTask(IBaseTask *newTask)
{
    QMutexLocker locker(&mutex);

    taskList.enqueue(newTask);
    runNextTask();
}

void Manager::runNextTask()
{
    QMutexLocker locker(&mutex);

    //Запуск задач из очереди, их может быть одновременно не более чем maxTaskCount задач
    while (taskCount < maxTaskCount) {
        if (taskList.empty())
            break;  //Задач больше нет
        ++taskCount;
        CustomCommand::runNextTask();
    }
}

/* Обработка сигнала завершения задачи
 * Уменьшяем счетчик числа запущенных комманд, если есть задачи в очереди, запускаем следующую задачу
 * Если задач в очереди нет и все запущенные команды завершились (taskCount == 0), завершаем выполнение отправкой
 * соответсвующего сигнала
 */
void Manager::taskFinished()
{
    if (taskCount == 0)  //Такого быть не должно! Завершилась задача, а запущенных задач 0!
        throw std::logic_error(QString("%1;taskFinished();task finished but taskCount == 0")
                               .arg(getName()).toStdString());

    QMutexLocker locker(&mutex);

    //Удаляем завершившуюся задачу
    --taskCount;
    if ( IBaseTask *task = dynamic_cast<IBaseTask*>(sender()) ) {
        logDebug << QString("%1;taskFinished();finished: %2;taskCount = %3/%4")
                    .arg(getName(), task->getName()).arg(taskCount).arg(taskList.size());
        task->deleteLater();
    } else
        logCritical << QString("%1;taskFinished();can't get task!;taskCount = %2/%3")
                       .arg(getName()).arg(taskCount).arg(taskList.size());

    if (!taskList.isEmpty() && !isStopRequested)
        runNextTask();    //Запускаем следующую задачу
    else if (taskCount == 0)
        emit finished();  //отправляем сигнал о завершении
}

QThread *Manager::getThread()
{
    return taskThread;
}

}
