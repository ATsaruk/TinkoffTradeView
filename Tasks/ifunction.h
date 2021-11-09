/* Для наследования от данного класса необходимо реализовать функции exec, setData, getResult и конструктор
 * Объяснение будет общее и для класса IBaseTask, т.к. по сути функция отличиается от задачи только 1, функция
 * выполняется сразу и результат работы доступен сразу после вызова функции exec()!
 * Завершение выполнения задачи заканчивается только после формирования сигнала finished()!
 * Например загрузка из БД это функция, т.е. результат нам доступен сразу.
 * Загрузка данных от брокера это задача, т.к. в функции exec() мы может только отправить запрос брокеру и результата
 * у нас сразу нет! Нужно дать сигнал о получении данных от брокера, обработать его и только после этого сформировать
 * сигнал finished()!
 * И ещё одно отличие заключается в том, что у функции exec() находится в разделе public и её можно просто вызвать,
 * у задач exec() находится в секции protected и запуск задач возможен только через TaskManager или из других задач!
 * Теперь как создать свой класс функцию или задачу, приведу пример:
 * Допустим у нас есть акция с ключем stockKey (Data::StockKey), и нам нужны свечи Heiken Ashi за заданный интервал
 * времени  range (Data::Range), немного поясню, свечи Heiken Ashi это не обычные японские свечи, а это свечи над
 * которыми выполнены определенные преобразования (детали сейчас неважны).
 * Для упрощенного решения данной задачи нам понадобится 1 задача и 1 функция:
 * 1. Задача для загрузуи свечей от брокера:
 *   class loadFromBroker : public IBaseTask {
 *     Data::StockKey stockKey;
 *     InterfaceWrapper<Data::Range> range;
 *     InterfaceWrapper<Data::Stock> stock;
 *   public:
 *     loadFromBroker(const Data::StockKey &key)
 *         : IBaseTask("loadFromBroker"), stockKey(key) {}
 *
 *     void setData(SharedInterface &inputData) override {
 *       range = inputData;
 *     }
 *
 *     SharedInterface &getResult() override {
 *       return &stock;
 *     }
 *
 *   protected:
 *     void exec() override {
 *       ...
 *       //Это просто какие то действия
 *       auto *broker = Glo.getBroker;
 *       connect(broker, &Broker::onResponse, this, &loadFromBroker::onResponse);
 *       stock = Broker::loadCandles(stockKey, range); //some action for load data
 *       ...
 *     }
 *
 *   protected slots:
 *     void onResponse(ResponseDataType &data) {
 *       ...
 *       stock = getCandlesFromAnswer(data);
 *       ...
 *       emit finished();
 *     }
 *   }
 *
 * В конструкторе мы указали имя задачи и передали данные, которые не зависят от возомжного выполнения предыдущих задач,
 * например Data::Range range это может быть так же результат работы задачи по формирования интервала для загрузки, и
 * окончательный интервал для загрузки может быть известен только непосредственно перед запуском задачи,
 * Ключ акции же извествен заранее и он не изменится, мы значем для какой акции мы хотим сформировать свечи, поэтому
 * он передается в конструкторе.
 *
 * setData и getResult просто сохраняют исходные данные и возвращают результат InterfaceWrapper является QSharedPointer
 * поэтому беспокоится об их удалении не нужно, это произойдет автоматически, когда будет удалена последняя задача
 * использующая соотвествующие данные
 *
 * В функции exec() мы подключили сигнал о получении ответа от брокера и отправили запрос, соответственно результата у
 * нас ещё нет (поэтому это задача, а не функция!)
 * Когда будет получен ответ от брокера, будет вызвана функция onResponse, будут получены свечи, далее мог бы произойти
 * какой нибудь ещё запрос, не важно любые действия, задача будет выполняться пока она не отправит сигнал emit finished();
 *
 * 2. Функция для преобразования японских свечей в свечи Heiken Ashi
 *   class ConvertToHeikenAshiFunc : public IFunction {
 *     SomeType someConstAdditionData;
 *     InterfaceWrapper<Data::Stock> stock;
 *   public:
 *     ConvertToHeikenAshiFunc(const SomeType &data)
 *         : IFunction("ConvertToHeikenAshiFunc"), stockKey(key), someConstAdditionData(data) {}
 *
 *     void setData(SharedInterface &inputData) override {
 *       stock = inputData;
 *     }
 *
 *     SharedInterface &getResult() override {
 *       return &stock;
 *     }
 *
 *     void exec() override {
 *       ...
 *       //действия над свечами из stock
 *       ...
 *     }
 *   }
 *
 * В конструкторе просто для примера указал ещё какие то дополнительные данные, которые известны перед запуском задачи
 * по загрузке данных, например это может быть какой то настроечный параметр для преобразования японской свечи в свечу
 * Heiken Ashi, в данном примере дополнительных данных не требуется.
 *
 * В качестве входных данных будут выступать свечи в акции stock, которые были загружены предыдущей задачей, и действия
 * по преобразованию будут проведены над ними же и в качестве результат они же и будут возвращены, хотя никто не мешает
 * объявить новый InterfaceWrapper<Data::Stock> resultStock, и результат формировать в нем.
 *
 * exeс() так же в отличии от предыдущего пример он являет public и завершает все необходимые действия в теле функции
 * И можно использовать где угодно:
 * void foo() {
 *   ...
 *   какие то действия в результате которых у нас есть Data::Stock stock
 *   ...
 *   SomeType confData(...);
 *   ...
 *   ConvertToHeikenAshiFunc bar(confData);
 *   InterfaceWrapper<Data::Stock> inData (stock);  //обертка над исходными данными, для передачи в функцию
 *   bar.setData( &inData );    // перегруженная операция &, она возвращает SharedInterface на данные (см. Task/Interfaces/interface.h)
 *   bar.exec();
 *
 *   InterfaceWrapper<Data::Stock> result = bar.getResult();
 *   или может сохранить в ту же обертку
 *   inData = bar.getResult(); //но как то не красиво что результат записывается в inData...
 * }
 */

/** @defgroup Task Задачи
  * @author Царюк А.В.
  * @date Апрель 2021 года */

#ifndef IFUNCTION_H
#define IFUNCTION_H

#include <QObject>
#include <QString>
#include <QSharedPointer>

#include "Interfaces/interfase.h"

namespace Task {


/** @ingroup Task
  * @brief Базовый класс функции для паттерна компоновщик */
class IFunction : public QObject
{
public:
    explicit IFunction(const QString &name);
    virtual ~IFunction();

    ///Возвращает имя функции (нужно для ведения логов)
    virtual QString getName() final;

    ///Возвращает true - если это функция, false - если это задача
    virtual bool isFunction() final;

    ///Функция которая будет вызвана при запуске функции/задачи (QThread)
    virtual void exec() = 0;

    /** @brief Задает исходные данные для функции/задачи
      * @param inputData - класс обертка над исходными данными
      * setData вызывается при запуске функции/задачи, перед вызовом функции exec(). Подробнее см. в начале файла.*/
    virtual void setData(SharedInterface &inputData) = 0;

    ///Возвращает класс интерфейс с результатом
    virtual SharedInterface &getResult() = 0;

protected:
    //Запрос на остановку задачи
    bool isStopRequested = false;

    //Признак является данный класс классом функции или задачи (у функции резальтат доступен сразу после вызова exec())
    //У задачи результат доступен только после генерации сигнала finished()!
    bool isFunc = true;

    //Имя функции/задачи
    QString functionName;
};

}

#endif // IFUNCTION_H
