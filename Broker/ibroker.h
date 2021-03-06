/** @defgroup Broker Брокер API
  * @brief Модуль интерфейса взаимодействия с брокером
  * @author Царюк А.В.
  * @date Апрель 2021 года
  */

#ifndef IBROKER_H
#define IBROKER_H

#include <QMutex>

#include "Data/Stock/stockkey.h"
#include "Data/range.h"

namespace Broker {


/** @ingroup Broker
  * @brief Интерфейс для запроса информации у брокера
  * @details Для добавления API для нового брокера, нужно наследоваться от данного класса.
  * @warning Перед началом работы с классом блокируем mutex, после окончания освобождаем. */
class IBroker : public QObject
{
public:
    QMutex mutex;

    explicit IBroker() { }
    virtual ~IBroker() { }


    /** @brief Запроса на получение списка всех акций
      * @return TRUE - запрос отправлен, FALSE - ошибка отправки (не стоит ждать ответа от сервера) */
    [[nodiscard]] virtual bool loadStocks() = 0;


    /** @brief Запрос на получение свечной информации по акции
      * @param[IN] stockKey первичный ключ акции
      * @param[IN] from дата начала интервала запроса (МСК)
      * @param[IN] to дата окончания интервала запроса (МСК)
      * @return TRUE - запрос отправлен, FALSE - ошибка отправки (не стоит ждать ответа от сервера) */
    [[nodiscard]] virtual bool loadCandles(const Data::StockKey &stockKey, const Data::Range &range) = 0;


signals:
    /** @brief Cигнал о получении ответа от сервера брокера
      * @details При получении ответа с сервера брокера отправляется сигнал getResopnse, в качестве параметра QByteArray
      * передаются полученные данные, для дальнейшей обработки. */
    void getResopnse(const QByteArray &);

private:
    Q_OBJECT
};

}

#endif // IBROKER_H
