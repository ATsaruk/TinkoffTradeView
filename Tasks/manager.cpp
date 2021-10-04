#include <QThread>

#include "manager.h"
#include "Core/globals.h"

namespace Task {

Manager::Manager(QObject *parent)
    : QObject(parent), maxTaskCount(QThread::idealThreadCount() * 2)
{
    logDebug << QString("Task::Manager;Manager();created!");
}

Manager::~Manager()
{
    logDebug << QString("Task::Manager;~Manager();destroyed!");
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

        ++taskCount;

        logDebug << QString("TaskManager;runNextTask();started : %1;tasks: %2/%3")
                    .arg(task->getName()).arg(taskCount).arg(taskCount + taskList.size());

        task->start();
    }
}

/* Обработка сигнала завершения задачи
 * Уменьшяем счетчик числа запущенных комманд, если есть задачи в очереди, запускаем следующую задачу. */
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

    runNextTask();

    task->deleteLater();
}

}
