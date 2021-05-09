/* CustomCommand(nullptr...
 * Если в качестве QThread *parent передается nullptr, это означает что создается корневая комманда
 * например:
 *   auto *newCommand = new CustomCommand("MyCommand");
 * Данную команду можно зарегистрировать у TaskManager, но нельзя передать другой комманде, если мы хотим создать комманду, которая будет запущена в другой комманде, то:
 *   auto *rootCommand = new CustomCommand("MyRootCommand");
 *
 *   auto *newCommand1 = new CustomCommand(rootCommand->getThread(), "MyCommand1");
 *   newCommand1->setData(someData...);
 *   rootCommand->registerTask(newCommand1);
 *
 *   auto *newCommand2 = new CustomCommand(rootCommand->getThread(), "MyCommand2");
 *   newCommand2->setData(someOtherData...);
 *   rootCommand->registerTask(newCommand2);
 *
 *   ...
 *
 *   TaskManager::get()->registerTask(rootCommand);
 */
#include "ibasecommand.h"
#include "Core/globals.h"

namespace Task {


IBaseCommand::IBaseCommand(const QString commandName)
    : IBaseTask(commandName)
{

}

IBaseCommand::~IBaseCommand()
{
    emit stopAll();
}

void IBaseCommand::setData(SharedInterface &inputData)
{
    assert(!taskList.isEmpty() && "IBaseCommand::setData(): logical error!");
    taskList.front()->setData(inputData);
}

SharedInterface &IBaseCommand::getResult()
{
    assert(lastCompleteTask && "IBaseCommand::getResult(): logical error");
    return lastCompleteTask->getResult();
}

void IBaseCommand::registerFunc(IFunction *newTask)
{
    if (!newTask->isFunction())
        dynamic_cast<IBaseTask*>(newTask)->setThread(taskThread);
    taskList.enqueue(newTask);
}

void IBaseCommand::connect(QObject *receiver, const char *method)
{
    QObject::connect(this, SIGNAL(finished()), receiver, method, Qt::BlockingQueuedConnection);
}

void IBaseCommand::runNextTask(IFunction *previousTask)
{
    lastCompleteTask.reset(previousTask);

    if (taskList.isEmpty() || isStopRequested) {
        emit finished();
        return;
    }

    IFunction *currentTask = taskList.dequeue();

    if (lastCompleteTask)
        currentTask->setData(previousTask->getResult());

    if (currentTask->isFunction()) {
        currentTask->exec();
        runNextTask(currentTask);
    } else
        startTask( dynamic_cast<IBaseTask*>(currentTask) );
}

void IBaseCommand::startTask(IBaseTask *task)
{
    QObject::connect(task, &IBaseTask::finished,  this, &IBaseCommand::taskFinished);
    QObject::connect(this, &IBaseCommand::stopAll, task, &IBaseTask::stop);

    logDebug << QString("%1;runNextTask();started : %2;tasksLeft: %3")
                .arg(getName(), task->getName()).arg(taskList.size());

    task->start();
}

void IBaseCommand::taskFinished()
{
    auto finishedTask = dynamic_cast<IBaseTask*>(sender());

    assert(finishedTask != nullptr && QString("%1;taskFinished();can't get task!;tasksLeft: %2")
            .arg(getName()).arg(taskList.size()).toStdString().data());

    logDebug << QString("%1;taskFinished();finished: %2;tasksLeft: %3")
                .arg(getName(), finishedTask->getName()).arg(taskList.size());

    //Запускаем следующую или завершаем выполнение
    runNextTask(finishedTask);
}

void IBaseCommand::exec()
{
    runNextTask();
}

void IBaseCommand::stop()
{
    IBaseTask::stop();
    emit stopAll();
}

}
