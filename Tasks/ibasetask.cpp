#include <QThread>
#include <QMetaMethod>
#include <QApplication>

#include "ibasetask.h"

namespace Task {


IBaseTask::IBaseTask(const QString &taskName)
    : IFunction(taskName)
{
    isFunc = false;
}

IBaseTask::~IBaseTask()
{

}

QThread *IBaseTask::getThread()
{
    return taskThread;
}

void IBaseTask::setThread(QThread *parent)
{
    _isRootTask = parent == nullptr;     //parent == nullptr означает, что это корневая задача!

    if (_isRootTask) {    //Это корневая задача, создаем поток, и перемещаемся в него
        taskThread = new QThread;
        connect(taskThread, &QThread::started,  this, &IBaseTask::exec);
        connect(this, &IBaseTask::finished,  taskThread, &QThread::quit);
        connect(this, &IBaseTask::destroyed, taskThread, &QThread::deleteLater);
    } else    //Это не корневая задача, запоминаем родительский поток и перемещаемся в него
        taskThread = parent;

    this->moveToThread(taskThread);
}

void IBaseTask::start()
{
    if (_isRootTask) {
        taskThread->setObjectName(getName());
        taskThread->start();
    } else
        exec();
}

void IBaseTask::stop()
{
    isStopRequested = true;
}

}
