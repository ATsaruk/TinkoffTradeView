#include <QThread>

#include "manager.h"
#include "Core/globals.h"

namespace Task {

Manager::Manager(QObject *parent)
    : QObject(parent), maxTaskCount(QThread::idealThreadCount() * 2)
{
}

Manager::~Manager()
{
    emit stopAll();
}

void Manager::registerTask(IBaseTask *newTask)
{
    QMutexLocker locker(&mutex);

    taskList.enqueue(newTask);
    runNextTask();
}

void Manager::runNextTask()
{
    //Запуск задач из очереди, их может быть одновременно не более чем maxTaskCount задач
    while (taskCount < maxTaskCount) {
        if (taskList.empty())
            break;  //Задач больше нет

        IBaseTask *task = taskList.dequeue();
        connect(task, &IBaseTask::finished,  this, &Manager::taskFinished);
        connect(this, &Manager::stopAll, task, &IBaseTask::stop);

        task->start();
        ++taskCount;

        logDebug << QString("TaskManager;runNextTask();started : %1;tasks: %2/%3")
                    .arg(task->getName()).arg(taskList.size()).arg(taskCount + taskList.size());
    }
}

/* Обработка сигнала завершения задачи
 * Уменьшяем счетчик числа запущенных комманд, если есть задачи в очереди, запускаем следующую задачу
 * Если задач в очереди нет и все запущенные команды завершились (taskCount == 0), завершаем выполнение отправкой
 * соответсвующего сигнала
 */
void Manager::taskFinished()
{
    QMutexLocker locker(&mutex);

    assert(taskCount > 0 && "TaskManager;taskFinished();task finished but taskCount == 0");
    --taskCount;

    IBaseTask *task = dynamic_cast<IBaseTask*>(sender());

    assert(task != nullptr && QString("TaskManager;taskFinished();can't get task!;tasks: %1/%2")
            .arg(taskCount).arg(taskCount + taskList.size()).toStdString().data());

    logDebug << QString("TaskManager;taskFinished();finished: %1;tasks: %2/%3")
                    .arg(task->getName()).arg(taskCount).arg(taskCount + taskList.size());

    task->deleteLater();

    runNextTask();
}

}
