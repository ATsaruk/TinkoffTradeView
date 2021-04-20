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
#include "Data/Stock/stockkey.h"
#include "Data/Stock/candle.h"
#include "Data/range.h"

namespace Task {
using namespace Data;


///Задача загрузки свечей из БД за указанный временной интервал
class LoadStockFromBroker : public IBaseTask
{
    Q_OBJECT

public:
    LoadStockFromBroker(QThread *parent = nullptr);
    ~LoadStockFromBroker();

    //Возвращает имя класса владельца
    QString getName() override;

    //Задание исходных данных для загрузки
    void setData(const StockKey &stockKey_, const Range &range, const qint64 minCandleCount = 1);

    //Возвращает максимально допустимый интервал загрузки для primaryKey.interval
    static qint64 getMaxLoadInterval(const StockKey::INTERVAL &interval);

protected:
    size_t minCandles;      //Минимальное количество свечей, которое должно быть загружено
    StockKey stockKey;      //ключ акции

    Range curRange;     //Текущий подинтервал загрузки
    Range loadRange;    //Полный интервал загрузки
    Range extraRange;   //Дополнительный 2 недельный интервал в начале (нужно для загрузки через новогодние праздники)

    //Список полученных свечей
    Candles candles;

    //Запустить задачу
    void exec() override;

    //Запрос данных у брокера
    bool sendRequest();

    //Формируем временной интервал, на котором требуется загрузка данных
    bool initRanges();

    bool checkRange();

    bool loadExtraRange();

    //Смещает начало очередного интервала загрузки
    void shiftCurRange();

    //Освобождение ресурсов и сохранение полученных данных
    void finishTask();

protected slots:
    //Обработка отвера с сервера брокера
    void onResponse(QByteArray answer);

private:
    //Удаляет незавершенную свечу
    void removeIncompleteCandle(Candles &candles);
};

}

#endif // LOADSTOCKFROMBROKER_H
