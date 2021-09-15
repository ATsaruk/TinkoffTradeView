/* Класс для загрузки свечной информации с сервера брокера, для этого:
 * Пример использования:
 *   auto *loadCandlesTask = new TaskLoadStocksFromBroker;
 *
 * далее 1 из 4 нужных вариантов:
 *   loadCandlesTask->setData(curStockKey, beginTime, endTime, 100);    //минимум 100 свечей
 *   loadCandlesTask->setData(curStockKey, beginTime, endTime);         //если важен именно временной интервал (но не менее 1 свечи)
 *   loadCandlesTask->setData(curStockKey, beginTime);                  //загрузка с beginTime и до текущего времени (но не менее 1 свечи)
 *   loadCandlesTask->setData(curStockKey);                             //будет загружен maxLoadInterval для данной длинны сечи (но не менее 1 свечи)
 *
 *   taskManager->registerTask(loadCandlesTask);
 *
 * Заданный временной интервал будет разбит на подинтервалы длительностью не более getMaxLoadInterval() это ограничение OpenAPI Tinkoff
 *
 * Порядок действий:
 * п.1 Получение доступа к классу Broker функцией lock();
 * п.2 Подключение к сигналу onResponse для получения ответа от сервера
 * п.3 Формирование и отправка POST запрос за подинтервал curTime + getMaxLoadInterval() (из интервала запроса исключаются периоды
 *     на которые у нас уже есть данные)
 * п.4 Получение ответа от сервера и проверка его статуса
 * п.5 Получение свечной информации с последующим помещением её в БД и в класс с данными DataStocks
 * п.6 Если ещё не все данные получены, то смещаем время curTime вперед на getMaxLoadInterval() и переходим к п.3
 * п.7 После получения свечных данных со всего запрашиваемного диапазона производим следующие действия:
 *     - проверяем получено ли достаточно вечей (> minCandleCount), если нет то загружаем дополнительный 2 недельный
 *       интервал предшествующий timeBegin
 *     - отключаемся от сигнала onResponse
 *     - освобождаем доступ к классу Broker функцией unLock();
 *     - отправляем сигнал о завершении задачи
 */
#ifndef LOADSTOCKFROMBROKER_H
#define LOADSTOCKFROMBROKER_H

#include "Tasks/ibasetask.h"
#include "Data/range.h"
#include "Data/Stock/stocks.h"

namespace Task {


///Задача загрузки свечей из БД за указанный временной интервал
class LoadStockFromBroker : public IBaseTask
{
    Q_OBJECT

public:
    /** @brief LoadStockFromBroker
     *  @param stockKey - ключ акции для загрузки
     *  @param minCandlesCount_ - минимальное число свечей, после которого можно прекратить загрузку
     */
    explicit LoadStockFromBroker(const Data::StockKey &stockKey, const uint minCandlesCount_ = 0);
    ~LoadStockFromBroker();

    void setData(SharedInterface &inputData) override;

    SharedInterface &getResult() override;

protected:
    //Запустить задачу
    void exec() override;

    //Запрос данных у брокера
    bool sendRequest();

    //Смещает начало очередного интервала загрузки
    bool getNextLoadRange();

    //Освобождение ресурсов и сохранение полученных данных
    void finishTask();

    //Читает свечи из QJsonDocument'а
    bool readCandles(const QByteArray &answer);

    //Проверяет корректность полученного ключа акции
    bool checkStockKey(const QJsonObject &payload);

    //Удаляет незавершенную свечу
    void removeIncompleteCandle();

protected slots:
    //Обработка отвера с сервера брокера
    void onResponse(QByteArray answer);

private:
    uint minCandlesCount;

    InterfaceWrapper<Data::Range> range;
    InterfaceWrapper<Data::Stock> stock;

    Data::Range subRange;     //Текущий подинтервал загрузки
};

}

#endif // LOADSTOCKFROMBROKER_H
