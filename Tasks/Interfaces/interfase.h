///Убрать void operator= (InterfaceWrapper &other)!


#ifndef INPUTINTERFASE_H
#define INPUTINTERFASE_H

#include <QString>
#include <QSharedPointer>

#include <QDebug>

namespace Task {


template<class T> struct InterfaceData;

///Базовая структура для интерфеса между задачами
struct Interface
{
    virtual ~Interface() = default;

    //Возвращает данные, хранящиеся в интерфейсе
    template<class T>
    T& get() {
        InterfaceData<T> *downCast = dynamic_cast<InterfaceData<T> *>(this);
        assert(downCast!=nullptr && QString("Interface::get<%1>()").arg(typeid (T).name()).toStdString().data());
        return downCast->data;
    }

    //Проверяет может ли интерфейс кастануться к типу T
    template<class T>
    bool isCompatible() {
        InterfaceData<T> *upClass = dynamic_cast<InterfaceData<T> *>(this);
        return upClass != nullptr;
    }
};

///Шаблонная структура содержащая данные
template<class T>
struct InterfaceData : public Interface
{
    T data;
    explicit InterfaceData() = default;

    template<class N>
    InterfaceData(N &&init) noexcept : data(std::forward<N>(init)) { }
};


///Обертка над QSharedPointer<Interface>
template<class S, class T = std::remove_reference_t<std::remove_pointer_t<S>>>
class InterfaceWrapper {
public:
    explicit InterfaceWrapper() = default;

    //Дропает старую ссылку, и получает новую (если интерфейсы совместимы!)
    InterfaceWrapper(QSharedPointer<Task::Interface> &inputData) {
        assert(inputData->isCompatible<T>() && "DataWrapper constructor from Interface: Interfaces aren't compatible!");
        sharePtr = inputData;
    }
    void operator= (QSharedPointer<Task::Interface> &inputData) {
        assert(inputData->isCompatible<T>() && "DataWrapper = Interface: Interfaces aren't compatible!");
        sharePtr = inputData;
    }

    //Конструирует sharedPtr из forwarding reference
    template<class N>
    InterfaceWrapper(N &&data) noexcept : sharePtr( new InterfaceData<T>(std::forward<N>(data)) ) { }

    template<class N>
    void operator=(N &&data) noexcept {
        if (sharePtr.isNull())
            sharePtr = QSharedPointer<Interface>( new InterfaceData<T>(std::forward<N>(data)) );
        else
            sharePtr.data()->get<T>() = std::forward<N>(data);
    }

    //Каст к ссылке
    operator T& () {
        deferredInit();
        return sharePtr.data()->get<T>();
    }

    //Обращение к элементам хранимого объекта
    T* operator->() {
        deferredInit();
        return &(sharePtr.data()->get<T>());
    }

    //Оператор & возвращает ссылку на sharedPoint
    QSharedPointer<Interface>& operator & () {
        deferredInit();
        return sharePtr;
    }

protected:
    //Отсроченная инициализация, инициализирует объект, если sharedPointer.isNull()
    void deferredInit() {
        if (sharePtr.isNull())
            sharePtr = QSharedPointer<Interface>( new InterfaceData<T> );
    }

private:
    QSharedPointer<Interface> sharePtr = nullptr;
};

}

using SharedInterface = QSharedPointer<Task::Interface>;


#endif // INPUTINTERFASE_H
