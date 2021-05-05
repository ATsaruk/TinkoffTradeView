#include <QThread>
#include <QMetaMethod>
#include <QApplication>

#include "ibasetask.h"

namespace Task {


IBaseTask::IBaseTask(QString taskName)
    : IFunction(taskName)
{
    isFunc = false;
}

QThread *IBaseTask::getThread()
{
    return taskThread;
}

void IBaseTask::setThread(QThread *parent)
{
    isRootTask = parent == nullptr;

    if (isRootTask) {    //Это корневая задача, создаем поток, и перемещаемся в него
        taskThread = new QThread;
        connect(taskThread, &QThread::started,  this, &IBaseTask::exec);
        connect(this, &IBaseTask::finished,  taskThread, &QThread::quit);
        connect(this, &IBaseTask::destroyed, taskThread, &QThread::deleteLater);
    } else    //Это не корневая задача, запоминаем родительский поток и перемещаемся в него
        taskThread = parent;

    this->moveToThread(taskThread);
}

bool IBaseTask::isFinished()
{
    return taskThread->isFinished();
}

void IBaseTask::start()
{
    if (isRootTask) {
        taskThread->setObjectName(getName());
        taskThread->start();
    } else
        exec();
}

void IBaseTask::stop()
{
    if (isRootTask)
        taskThread->terminate();
    else
        isStopRequested = true;
}

}
