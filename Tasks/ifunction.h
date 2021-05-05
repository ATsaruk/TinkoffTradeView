/* //The interface remains the same type (input and output: InterfaceStock):
 * void Interface::exec()
 * {
 *   output = input;
 *   convertToHeikenAshi(output->candles);
 * }
 *
 * OR
 *
 * //the interface type has changed (input: InterfaceRange, output: InterfaceStock):
 * void Interface::exec()
 * {
 *   output = new InterfaceStock;
 *   output->stock = loadStock(input->range);
 *   delete input;
 * } */

#ifndef IFUNCTION_H
#define IFUNCTION_H

#include <QObject>
#include <QString>
#include <QSharedPointer>

#include "Interfaces/interfase.h"

namespace Task {


/// Базовый класс функции для паттерна компоновщик
class IFunction : public QObject
{
public:
    explicit IFunction(QString name);
    virtual ~IFunction();

    QString getName();

    //Возвращает true - если это функция, false - если это задача
    bool isFunction();

    ///Функция которая будет вызвана при запуске потока (QThread)
    virtual void exec() = 0;

    ///Получается класс интерфейс с исходными данными
    virtual void setData(SharedInterface &inputData) = 0;

    ///Возвращает класс интерфейс с результатом
    virtual SharedInterface &getResult() = 0;

protected:
    //Запрос на остановку задачи
    bool isStopRequested = false;
    bool isFunc = true;
    QString functionName;
};

}

#endif // IFUNCTION_H
