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
        return *downCast->data;
    }

    //Проверяет может ли интерфейс кастануться к типу T
    template<class T>
    bool isCompatible() const {
        const InterfaceData<T> *upClass = dynamic_cast<const InterfaceData<T> *>(this);
        return upClass != nullptr;
    }
};

///Шаблонная структура содержащая данные
template<class T>
struct InterfaceData : public Interface
{
    T *data = nullptr;
    explicit InterfaceData() { data = new T; }

    explicit InterfaceData(T *other) { data = new T(*other); }


    template<class N>
    explicit InterfaceData(N &&init) noexcept : data( new T(std::forward<N>(init)) ) { }

    InterfaceData(const InterfaceData&) = delete;
    InterfaceData& operator = (const InterfaceData&) = delete;
    InterfaceData& operator = (InterfaceData&&) = delete;

    ~InterfaceData() { delete data; }
};

using SharedInterface = QSharedPointer<Task::Interface>;

///Обертка над QSharedPointer<Interface>
template<class S, class T = std::remove_reference_t<std::remove_pointer_t<S>>>
class InterfaceWrapper {
public:
    explicit InterfaceWrapper() = default;

    //Дропает старую ссылку, и получает новую (если интерфейсы совместимы!)
    InterfaceWrapper(SharedInterface &inputData) {
        assert(inputData->isCompatible<T>() && "DataWrapper constructor from Interface: Interfaces aren't compatible!");
        _sharePtr = inputData;
    }
    void operator= (SharedInterface &inputData) {
        assert(inputData->isCompatible<T>() && "DataWrapper = Interface: Interfaces aren't compatible!");
        _sharePtr = inputData;
    }

    //Конструирует sharedPtr из forwarding reference или из QSharedPointer<T>
    template<class N>
    InterfaceWrapper(N &&data) {
        if constexpr (std::is_constructible_v<T, N>) {
            _sharePtr = QSharedPointer<InterfaceData<T>>::create( std::forward<N>(data) );
        } else if constexpr (std::is_same_v<QSharedPointer<T>, std::remove_reference_t<N>>) {
            _sharePtr = QSharedPointer<InterfaceData<T>>::create( data.data() );     //Создаем SharedInterface из QSharedPointer
        } else {
            throw std::logic_error("InterfaceWrapper::constructor InterfaceWrapper(N &&data) error input data");
        }
    }

    template<class N>
    void operator=(N &&data) {
        if constexpr (std::is_constructible_v<T, N>) {
            if (_sharePtr.isNull())
                _sharePtr = QSharedPointer<InterfaceData<T>>::create( std::forward<N>(data) );
            else
                _sharePtr.data()->get<T>() = std::forward<N>(data);
        } else if constexpr (std::is_same_v<QSharedPointer<T>, std::remove_reference_t<N>>) {
            _sharePtr = QSharedPointer<InterfaceData<T>>::create( data.data() );     //Создаем SharedInterface из QSharedPointer
        } else {
            throw std::logic_error("InterfaceWrapper::operator=(N &&data) error input data");
        }
    }

    //Каст к ссылке
    operator T& () {
        return _sharePtr.data()->get<T>();
    }

    //Обращение к элементам хранимого объекта
    T* operator->() {
        return &(_sharePtr.data()->get<T>());
    }

    //Оператор & возвращает ссылку на sharedPoint
    SharedInterface& operator & () {
        return _sharePtr;
    }

    //Создает "оборачиваемый" объект с параметрами args ...
    template <typename... N>
    void create(const N& ... args) {
        if (_sharePtr.isNull())
            _sharePtr = SharedInterface( new InterfaceData<T>(args ...) );
    }

private:
    SharedInterface _sharePtr;
};

}


#endif // INPUTINTERFASE_H
