#include <QThread>

#include "manager.h"

namespace Task {

Manager::Manager(QThread *parent)
    : CustomCommand(parent)
{
    //Обнуляем taskThread, если этого не сделать, то при вызове addTask() для новых задач, им будет передан taskThread в
    //качестве родительского потока и они будут создаваться внутри этого потока, а нам нужно что бы каждая задача была
    //отдельным потоком, и обнулением мы лишаем их родительского потока и они создадут свои потоки
    taskThread = nullptr;
}

//Возвращает имя задачи
QString Manager::getName()
{
    return "TaskManager";
}

//Регистрируем новую задачу и сразу её запускаем
void Manager::registerTask(IBaseTask *newTask)
{
    taskList.enqueue(newTask);
    exec();
}

/* Возвращает сколько задач можно запускать паралельно
 * QThread::idealThreadCount() - количество ядер в системе
 * Указывает IBaseCommand, что задачи нужно запускать паралельно
 */
uint Manager::getMaxExecTask()
{
    return QThread::idealThreadCount() * 2;
}

}
