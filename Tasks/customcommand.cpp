#include "customcommand.h"
#include "Core/globals.h"

namespace Task {


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
CustomCommand::CustomCommand(const QString &customCommandName)
    : CustomCommand(nullptr, customCommandName)
{
    logDebug << QString("Custom%1;Custom%1();+constructor!").arg(customCommandName);
}

CustomCommand::CustomCommand(QThread *parent, const QString &customCommandName)
    : IBaseTask(parent)
{
    taskCount = 0;
    setCommandName(customCommandName);
}

CustomCommand::~CustomCommand()
{
    emit stopAll();
    logDebug << "CustomCommand;~CustomCommand();-destructor!";
}

//Возвращает имя задачи
QString CustomCommand::getName()
{
    return name;
}

//Задание имени комманды
void CustomCommand::setCommandName(QString customCommandName)
{
    //Если первая бука прописная, меняем её на заглавную
    QChar first = customCommandName.at(0).toUpper();
    if(first.isLower())
        customCommandName.replace(0, 1, first);

    //Сохраняем имя команды, для динамически сформированных комманд всегда будет префикс Custom
    name = QString("Custom%1").arg(customCommandName);
}

//Регистрация задачи
void CustomCommand::registerTask(IBaseTask *newTask)
{
    QMutexLocker locker(&mutex);

    //Регистрация возможна только есть задачи работают в одном потоке
    if (newTask->getThread() == taskThread)
        taskList.enqueue(newTask);
    else
        logWarning << "CustomCommand;registerTask();try register task with other thread";
}

//Запускает следующую в очереди задачу
void CustomCommand::exec()
{
    runNextTask();
}

//Отправляет сигнал всем запущенным задачам на остановку (сигнал подключается в функции runNextTask)
void CustomCommand::stop()
{
    emit stopAll();
}

/* Возвращает сколько задач можно запускать паралельно (по уммолчанию 1 - последовательный запуск задач)
 * При переопределении данной функции, нужно что бы запускаемые задачи находились в разных потоках!
 * Данная функция была сделана для TaskManager'а, т.к. в нем могут выполнятся паралельно разные задачи
 * Каждая комманда помещена в отдельный поток и попытки создавать потоки внутри потоков ни к чему хорошему
 * не приводили, вернее создавать их не проблема, проблема потом возникает при удалении.
 * В общем оставялем эту функцию "как есть"
 *
 * Если все же понадобится мультипоточность внутри одной задачи, в теории втутри задачи в функции exec()
 * можно создать задачи для TaskManager::get()->addTask <нужная задача> () и подключится к сигналу о завершении.
 * Создать нужное количество таких задачь, TaskManager запустит их паралельно, нужно проверять.
 */
uint CustomCommand::getMaxExecTask()
{
    return 1;
}

/* Запускает задачи из очереди
 * Одновременно может работать не более getMaxExecTask() x2 задач
 * Достаем очередную задачу из очереди, подключаемся к сигналу завершения задачи (что бы по завершении запустить следующую)
 * Увеличиваем счетчик запущенных задач и запускам задачу
 */
void CustomCommand::runNextTask()
{
    QMutexLocker locker(&mutex);

    uint maxTaskCount = getMaxExecTask();
    //Запуск задач из очереди, их может быть одновременно не более чем getMaxExecTask() задач
    while (taskCount < maxTaskCount) {
        if (taskList.empty())
            break;  //Задач больше нет

        IBaseTask *task = taskList.dequeue();
        connect(task, &IBaseTask::finished,  this, &CustomCommand::taskFinished);
        connect(this, &CustomCommand::stopAll, task, &IBaseTask::stop);

        taskCount++;
        logDebug << QString("%1;runNextTask();started : %2;taskCount = %3/%4").arg(getName(), task->getName()).arg(taskCount).arg(taskList.size()+1);
        task->start();
    }
}

/* Обработка сигнала завершения задачи
 * Уменьшяем счетчик числа запущенных комманд, если есть задачи в очереди, запускаем следующую задачу
 * Если задач в очереди нет и все запущенные команды завершились (taskCount == 0), завершаем выполнение отправкой соответсвующего сигнала
 */
void CustomCommand::taskFinished()
{
    QMutexLocker locker(&mutex);

    if (taskCount > 0) {
        taskCount--;

        IBaseTask *task = static_cast<IBaseTask*>(sender());
        if (task != nullptr) {
            logDebug << QString("%1;taskFinished();finished: %2;taskCount = %3/%4").arg(getName(), task->getName()).arg(taskCount).arg(taskList.size());
            //task->deleteLater();
            delete task;
        } else
            logCritical << QString("%1;taskFinished();can't get task!;taskCount = %2/%3").arg(getName()).arg(taskCount).arg(taskList.size());

        if (!taskList.isEmpty() && !isStopRequested)
            //Если остались задачи в очереди и нет запроса на остановку, то запускаем следующую задачу
            runNextTask();
        else if (taskCount == 0)
            //Если список задач пуст (или пришел запрос на остановку задачи) и все активные задачи завершены, отправляем сигнал о завершении
            emit finished();
    } else
        //Такого быть не должно, если произошло, пишем в лог!
        logCritical << QString("%1;taskFinished();task finished but taskCount == 0").arg(getName());
}

}
