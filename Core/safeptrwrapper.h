#ifndef SAFEPTRWRAPPER_H
#define SAFEPTRWRAPPER_H

#include "safe_ptr.h"

namespace Core {


/** @brief Обертка над классом safe_ptr
  * @details Обертка сделана для возможности отложенной инициализации объекта
  */
template <class T>
class SafePtr {
public:
    SafePtr() { data = nullptr; }

    template<typename... Args>
    SafePtr<T>(Args... args) { init(args...); }

    ~SafePtr() { clear(); }

    template<class S, typename... Args>
    std::enable_if_t<std::is_base_of_v<T, S>>
    init(Args... args) {
        clear();
        T *t = new S(args ...);
        data = new safe_ptr<T>(t);
    }

    /** @brief Инициализация объекта
      * @param args... - параметры для коструктора создаваемого объекта
      * @details Наверное это то, для чего вообще делалась эта обертка, что бы можно было
      * делать отложенную инициализацию, а так же замену хранимого объекта. \n
      * Например мы работали с одной БД и мы можем на рантайме переинициализировать БД другого типа.
      */
    template<typename... Args>
    void init(Args... args) {
        clear();
        data = new safe_ptr<T>(args...);
    }


    /** @brief Получение доступа к публичным членам и методам объекта
      * @return safe_ptr<T> &operator -> - на самом деле будет предоставлен доступ к методам класса T&
      * @details Пример: @code
      * SafePtr<Config> conf ("config.cfg");
      * conf->setValue("key", QVariant("value")); @endcode */
    safe_ptr<T>& operator -> () { return (*data); }


    /** @brief Получение ссылки на исходный объект
      * @return safe_ptr<T> - будет предоставлен доступ к T&
      * @details Пример: @code
      * SafePtr<int> val;
      * val.init(5);
      * *val = 2;
      * qDebug() << *val; //2
      * @endcode
      */
    T* operator * () { return *(*(*data)); }


    /** @brief Получение доступа к исходному safe_ptr
      * @details Это нужно, если мы хотим заблокировать доступ сразу к группе объектов: @code
      * SafePtr<MydataClass1> data1(initData11, ...);
      * SafePtr<MydataClass2> data2(initData21, ...);
      * lock_timed_any_infinity locker(&data1, &data2);
      * //Действия с данными @endcode
      * При выходе из видимости locker'а доступ к объектам data1 и data2 будет освобожден */
    safe_ptr<T>& operator & () { return *data; }


    /// Удаляем операто =, нельзя писать SafePtr<int> val; val = 2;!, нужно писать *val = 2;
    template<class NewValue>
    void operator = (NewValue) = delete;

    const T&           operator *  () const { return *(*(*data)); }
    const safe_ptr<T>& operator &  () const { return *data; }
    const safe_ptr<T>& operator -> () const { return *data; }

private:
    safe_ptr<T> *data;

    void clear() { if(data !=nullptr) delete data; }
};

}

#endif // SAFEPTRWRAPPER_H
