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


IBaseCommand::IBaseCommand(const QString commandName_)
    : IBaseTask()
{
    commandName = commandName_;
    logDebug << QString("%1;%1();+constructor!").arg(commandName);
}

IBaseCommand::~IBaseCommand()
{
    if (lastTask != nullptr)
        lastTask->deleteLater();
    emit stopAll();
    logDebug << QString("%1;~%1;-destructor!").arg(commandName);
}

QString IBaseCommand::getName()
{
    return commandName;
}

void IBaseCommand::setData(SharedInterface &inputData)
{
    assert(!taskList.isEmpty() && "IBaseCommand::setData(): logical error!");
    taskList.front()->setData(inputData);
}

SharedInterface &IBaseCommand::getResult()
{
    assert(lastTask != nullptr && "IBaseCommand::getResult(): logical error");
    return lastTask->getResult();
}

void IBaseCommand::registerFunc(IFunction *newTask)
{
    ///@todo Если не будет работать waitForFinished, пробовать без переноса в тот же поток
    if (!newTask->isFunction())
        dynamic_cast<IBaseTask*>(newTask)->setThread(taskThread);
    taskList.enqueue(newTask);
}

void IBaseCommand::runNextTask(IFunction *previousTask)
{
    if (taskList.isEmpty() || isStopRequested) {
        lastTask = previousTask;
        emit finished();
        return;
    }

    if (previousTask != nullptr)
        previousTask->deleteLater();

    IFunction *currentTask = taskList.dequeue();
    if (previousTask != nullptr)
        currentTask->setData(previousTask->getResult());

    if (currentTask->isFunction()) {
        currentTask->exec();
        runNextTask(currentTask);
    } else
        execTask( dynamic_cast<IBaseTask*>(currentTask) );
}

void IBaseCommand::execTask(IBaseTask *task)
{
    connect(task, &IBaseTask::finished,  this, &IBaseCommand::taskFinished);
    connect(this, &IBaseCommand::stopAll, task, &IBaseTask::stop);

    logDebug << QString("%1;runNextTask();started : %2;tasksLeft = %3")
                .arg(getName(), task->getName()).arg(taskList.size());

    task->start();
}

void IBaseCommand::taskFinished()
{
    auto finishedTask = dynamic_cast<IBaseTask*>(sender());

    assert(finishedTask != nullptr && QString("%1;taskFinished();can't get task!;tasksLeft = %2")
            .arg(getName()).arg(taskList.size()).toStdString().data());

    logDebug << QString("%1;taskFinished();finished: %2;tasksLeft = %3")
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
