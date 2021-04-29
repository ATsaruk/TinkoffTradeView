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
#include "customcommand.h"
#include "Core/globals.h"

namespace Task {


CustomCommand::CustomCommand(const QString &customCommandName)
    : CustomCommand(nullptr, customCommandName)
{
    logDebug << QString("Custom%1;Custom%1();+constructor!").arg(customCommandName);
}

CustomCommand::CustomCommand(QThread *parent, const QString &customCommandName)
    : IBaseTask(parent)
{
    setCommandName(customCommandName);
}

CustomCommand::~CustomCommand()
{
    emit stopAll();
    logDebug << "CustomCommand;~CustomCommand();-destructor!";
}

QString CustomCommand::getName()
{
    return name;
}

void CustomCommand::setCommandName(QString customCommandName)
{
    //Если первая бука прописная, меняем её на заглавную
    QChar first = customCommandName.at(0).toUpper();
    if(first.isLower())
        customCommandName.replace(0, 1, first);

    //Сохраняем имя команды, для динамически сформированных комманд всегда будет префикс Custom
    name = QString("Custom%1").arg(customCommandName);
}

void CustomCommand::registerTask(IBaseTask *newTask)
{
    QMutexLocker locker(&mutex);

    //Регистрация возможна только есть задачи работают в одном потоке
    if (newTask->getThread() == taskThread)
        taskList.enqueue(newTask);
    else
        logWarning << "CustomCommand;registerTask();try register task with other thread";
}

void CustomCommand::exec()
{
    runNextTask();
}

void CustomCommand::stop()
{
    emit stopAll();
}

void CustomCommand::runNextTask()
{
    QMutexLocker locker(&mutex);

    if (taskList.isEmpty())
        throw std::logic_error("CustomCommand: Call runNextTask() with empty taskList!");

    IBaseTask *task = taskList.dequeue();
    connect(task, &IBaseTask::finished,  this, &CustomCommand::taskFinished);
    connect(this, &CustomCommand::stopAll, task, &IBaseTask::stop);

    logDebug << QString("%1;runNextTask();started : %2;tasksLeft = %3")
                .arg(getName(), task->getName()).arg(taskList.size());

    task->start();
}

void CustomCommand::taskFinished()
{
    QMutexLocker locker(&mutex);

    //Удаляем завершившуюся задачу
    if ( auto task = dynamic_cast<IBaseTask*>(sender()) ) {
        logDebug << QString("%1;taskFinished();finished: %2;tasksLeft = %3")
                    .arg(getName(), task->getName()).arg(taskList.size());
        delete task;
    } else
        logCritical << QString("%1;taskFinished();can't get task!;tasksLeft = %2")
                       .arg(getName()).arg(taskList.size());

    //Запускаем следующую или завершаем выполнение
    if (!taskList.empty() && !isStopRequested)
        runNextTask();
    else
        emit finished();
}

}
