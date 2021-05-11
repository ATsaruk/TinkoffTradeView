/** @ingroup Broker
  * @defgroup Tinkoff Тинькофф Open API
  * @brief Модуль интерфейса взаимодействия с Тинькофф Open API
  * @author Царюк А.В.
  * @date Апрель 2021 года */

#ifndef TINKOFFAPI_H
#define TINKOFFAPI_H

#include "Broker/api.h"
#include "request.h"

namespace Broker {


/** @ingroup Tinkoff
  * @brief Класс для запроса данных с сервера Тинькофф через Open API
  * @todo реализовать подписку на свечи (https://tinkoffcreditsystems.github.io/invest-openapi/marketdata/)
  * @details Ознакомиться с документацией по Тинькофф Open API можно тут:
  * - https://tinkoffcreditsystems.github.io/invest-openapi/
  * - https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/ */
class TinkoffApi : public Api
{
public:
    explicit TinkoffApi();

    /** @brief Отправляет запрос на получение списка всех акций
      * @details Подробнее о формате запроса и ответа тут: https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/market/get_market_stocks
      * @return TRUE - запрос отправлен, FALSE - запрос не отправлен, отсутсвует токен авторизации (не стоит ждать ответа от сервера) */
    [[nodiscard]] bool loadStocks() override;


    /** @brief Отправляет запрос на получение свечной информации по акции
      * @param[IN] stockKey первичный ключ акции
      * @param[IN] from дата начала интервала запроса (МСК)
      * @param[IN] to дата окончания интервала запроса (МСК)
      * @details Подробнее о формате запроса и ответа тут: https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/market/get_market_candles
      * @return TRUE - запрос отправлен, FALSE - запрос не отправлен, отсутсвует токен авторизации (не стоит ждать ответа от сервера) */
    [[nodiscard]] bool loadCandles(const Data::StockKey &stockKey, const Data::Range &range) override;

    //Возвращает максимально допустимый интервал загрузки для primaryKey.interval
    static qint64 getMaxLoadInterval(const Data::StockKey::INTERVAL &interval);

protected:
    /// Интерфейс для отправки POST/GET запросов
    QScopedPointer<Request> html;

private:
    Q_OBJECT
};

}

#endif // TINKOFFAPI_H
