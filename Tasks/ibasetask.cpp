#include <QThread>
#include <QApplication>

#include "ibasetask.h"

namespace Task {

IBaseTask::IBaseTask(QThread *parent)
{
    isStopRequested = false;

    if (parent == nullptr) {
        //Это корневая задача, создаем поток, и перемещаемся в него
        isRootTask = true;
        taskThread = new QThread;
        this->moveToThread(taskThread);
        connect(taskThread, &QThread::started,  this, &IBaseTask::exec);
        connect(this, &IBaseTask::finished,  taskThread, &QThread::quit);
        connect(this, &IBaseTask::destroyed, taskThread, &QThread::deleteLater);
    } else {
        //Это не корневая задача, запоминаем родительский поток и перемещаемся в него
        isRootTask = false;
        taskThread = parent;
        this->moveToThread(taskThread);
    }
}

IBaseTask::~IBaseTask()
{

}

QThread *IBaseTask::getThread()
{
    return taskThread;
}

//Запуск задачи
void IBaseTask::start()
{
    if (isRootTask) {
        taskThread->setObjectName(getName());
        taskThread->start();
    } else
        exec();
}

//Остановить задату
void IBaseTask::stop()
{
    isStopRequested = true;
}

}
