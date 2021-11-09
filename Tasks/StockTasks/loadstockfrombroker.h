#ifndef LOADSTOCKFROMBROKER_H
#define LOADSTOCKFROMBROKER_H

#include "Tasks/ibasetask.h"
#include "Data/range.h"
#include "Data/stocks.h"

namespace Task {


/** @ingroup Commands
  * @brief Задача загрузки свечей от брокера за указанный временной интервал
  * @details Порядок действий:\n
  * п.1 Получение доступа к классу Broker функцией lock();\n
  * п.2 Подключение к сигналу onResponse для получения ответа от сервера;\n
  * п.3 Формирование и отправка POST запрос за подинтервал curTime + getMaxLoadInterval() (из интервала запроса
  *     исключаются периоды на которые у нас уже есть данные);\n
  * п.4 Получение ответа от сервера и проверка его статуса;\n
  * п.5 Получение свечной информации с последующим помещением её в БД и в класс с данными DataStocks;\n
  * п.6 Если ещё не все данные получены, то смещаем время curTime вперед на getMaxLoadInterval() и переходим к п.3;\n
  * п.7 После получения свечных данных со всего запрашиваемного диапазона производим следующие действия:\n
  *     - отключаемся от сигнала onResponse,\n
  *     - освобождаем доступ к классу Broker функцией unLock(),\n
  *     - отправляем сигнал о завершении задачи. */
class LoadStockFromBroker : public IBaseTask
{
    Q_OBJECT

public:
    /** @brief LoadStockFromBroker
      * @param stockKey - ключ акции для загрузки */
    explicit LoadStockFromBroker(const Data::StockKey &stockKey);
    ~LoadStockFromBroker();

    ///Сохраняет диапазон, в котором будет происходить загрузка (Data::Range)
    void setData(SharedInterface &inputData) override;

    ///Возвращает загруженные свечи (Data::Stock)
    SharedInterface &getResult() override;

protected:
    ///Запустить задачу
    void exec() override;

    ///Запрос данных у брокера
    bool sendRequest();

    ///Смещает начало очередного интервала загрузки
    bool getNextLoadRange();

    ///Освобождение ресурсов и сохранение полученных данных
    void finishTask();

    ///Читает свечи из QJsonDocument'а
    bool readCandles(const QByteArray &answer);

    ///Проверяет корректность полученного ключа акции
    bool checkStockKey(const QJsonObject &payload);

    ///Удаляет незавершенную свечу
    void removeIncompleteCandle();

protected slots:
    ///Обработка отвера с сервера брокера
    void onResponse(const QByteArray &answer);

private:
    Data::Range _subRange;     //Текущий подинтервал загрузки

    InterfaceWrapper<Data::Range> _range;    //интервал для загрузки
    InterfaceWrapper<Data::Stock> _stock;    //загруженные свечи
};

}

#endif // LOADSTOCKFROMBROKER_H
