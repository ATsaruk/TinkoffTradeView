#include "sequencecommand.h"

#include "Core/globals.h"

namespace Task {


SequenceCommand::SequenceCommand(QThread *thread, IBaseTask *baseTask)
    : IBaseTask(thread)
{
    taskList.push_back(baseTask);
}

InterfaceType SequenceCommand::getInputInterfaceName()
{
    InputInterface *input = dynamic_cast<InputInterface*>(taskList.front());
    return input->getInputInterfaceName();
}

InterfaceType SequenceCommand::getOutputInterfaceName()
{
    OutputInterface *output = dynamic_cast<OutputInterface*>(taskList.back());
    return output->getOutputInterfaceName();
}

void SequenceCommand::exec()
{
    if (isStopRequested) {
        emit finished();
        return;
    }

    if (taskList.isEmpty())
        throw std::logic_error("SequenceCommand: Call runNextTask() with empty taskList!");

    IBaseTask *task = taskList.dequeue();
    connect(task, &IBaseTask::finished,  this, &SequenceCommand::taskFinished);
    connect(this, &SequenceCommand::stopAll, task, &IBaseTask::stop);

    logDebug << QString("%1;runNextTask();started : %2;tasksLeft = %3")
                .arg(getName(), task->getName()).arg(taskList.size());

    task->start();

}

void SequenceCommand::taskFinished()
{

}

}
