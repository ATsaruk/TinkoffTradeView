#ifndef SEQUENCECOMMAND_H
#define SEQUENCECOMMAND_H

#include <QQueue>

#include "ibasetask.h"
#include "Interfaces/interfases.h"

namespace Task {


class SequenceCommand : public IBaseTask, public InputInterface, public OutputInterface
{
public:
    explicit SequenceCommand(QThread *thread, IBaseTask *baseTask);

    template <class T>
    std::enable_if_t<std::is_base_of_v<IBaseTask, T> && std::is_base_of_v<OutputInterface, T>, T*>
    attachFront()
    {
        InputInterface *input = dynamic_cast<InputInterface*>(taskList.front());
        if (input == nullptr)
            throw std::logic_error("SequenceCommand::attachFront: taskList.front() has no InputInterface!");

        T *newTask = new T(taskThread);
        InterfaceType outputInterface = newTask->getOutputInterfaceName();
        InterfaceType inputInterface = input->getInputInterfaceName();
        assert(outputInterface != inputInterface && "SequenceCommand: interfaces are not compatible");

        taskList.push_front(newTask);
        return newTask;
    }

    template <class T>
    std::enable_if_t<std::is_base_of_v<IBaseTask, T> && std::is_base_of_v<InputInterface, T>, T*>
    attachBack()
    {
        OutputInterface *output = dynamic_cast<OutputInterface*>(taskList.back());
        if (output == nullptr)
            throw std::logic_error("SequenceCommand::attachFront: taskList.back() has no OutputInterface!");

        T *newTask = new T(taskThread);
        InterfaceType inputInterface = newTask->getInputInterfaceName();
        InterfaceType outputInterface = output->getOutputInterfaceName();
        assert(inputInterface != outputInterface && "SequenceCommand: interfaces are not compatible");

        taskList.push_back(newTask);
        return newTask;
    }

    InterfaceType getInputInterfaceName() override;
    InterfaceType getOutputInterfaceName() override;

    template <class T, typename... N>
    std::enable_if_t<std::is_base_of_v<InputInterface, T>, void>
    setData(N ... args)
    {
        T *inputInterface = dynamic_cast<T*>(taskList.front());
        inputInterface->setData(args ...);
    }

    template <class T, typename... N>
    std::enable_if_t<std::is_base_of_v<OutputInterface, T>, decltype (T::getResult())>
    getResult()
    {
        T* outputInterface = dynamic_cast<T*>(lastTask);
        return outputInterface->getResult();
    }

signals:
    void stopAll();

protected:
    void exec() override;

protected slots:
    void taskFinished();

private:
    Q_OBJECT

    QQueue<IBaseTask*> taskList;
    IBaseTask *lastTask;
};

}
#endif // SEQUENCECOMMAND_H
