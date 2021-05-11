#include <QJsonObject>

#include "tinkoff.h"

namespace Broker {

constexpr quint64 SECS_IN_ONE_DAY   = 24 * 3600;
constexpr quint64 SECS_IN_ONE_WEEK  = 7 * SECS_IN_ONE_DAY;
constexpr quint64 SECS_IN_ONE_MONTH = 30 * SECS_IN_ONE_DAY;

TinkoffApi::TinkoffApi()
    : html(new Request)
{
    connect(html.data(), &Request::getResopnse, this, &Api::getResopnse);
}

bool TinkoffApi::loadStocks()
{
    return html->sendGet("/market/stocks");
}

/* Пример готового запроса:
 * https://api-invest.tinkoff.ru/openapi/market/candles?figi=BBG000B9XRY4&from=2020-03-03T00%3A00%3A00.00%2B03%3A00&to=2021-03-03T00%3A00%3A00.00%2B03%3A00
 *                                                                        &interval=week" -H "accept: application/json" -H "Authorization: Bearer "authKey"
 *
 * В переменной path нужно сформировать строку вида:
 * R("/market/candles?figi=BBG000B9XRY4&from=2020-03-03T00%3A00%3A00.00%2B03%3A00&to=2021-03-03T00%3A00%3A00.00%2B03%3A00&interval=week")
 *
 * Адресс и данные для авторизации будут добавлены внутри функции html->sendGet(path);
 *
 * Подробнее о формате запроса и ответа тут: https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/market/get_market_candles */
bool TinkoffApi::loadCandles(const Data::StockKey &stockKey, const Data::Range &range)
{
    const QDateTime &from = range.getBegin();
    const QDateTime &to = range.getEnd();

    QString request = QString("/market/candles?figi=%1").arg(stockKey.figi());

    //Дата начала интервала запроса
    request += QString("&from=%1").arg(from.date().toString("yyyy-MM-dd"))
            + QString("T%1").arg(from.time().hour(), 2, 10, QChar('0'))  + R"(%3A)"
            + QString("%1").arg(from.time().minute(), 2, 10, QChar('0')) + R"(%3A)"
            + QString("%1").arg(from.time().second(), 2, 10, QChar('0')) + R"(%2B03%3A00)";

    //Дата конца интервала запроса
    request += QString("&to=%1").arg(to.date().toString("yyyy-MM-dd"))
            + QString("T%1").arg(to.time().hour(), 2, 10, QChar('0'))  + R"(%3A)"
            + QString("%1").arg(to.time().minute(), 2, 10, QChar('0')) + R"(%3A)"
            + QString("%1").arg(to.time().second(), 2, 10, QChar('0')) + R"(%2B03%3A00)";

    request += QString("&interval=%1").arg(stockKey.intervalToString());

    return html->sendGet(request);
}

qint64 TinkoffApi::getMaxLoadInterval(const Data::StockKey::INTERVAL &interval)
{
    switch (interval) {
    case Data::StockKey::INTERVAL::DAY  :   //no break;
    case Data::StockKey::INTERVAL::WEEK :
        //Свечи длительностью 1 день и более загрузаются с максимальным интервалом 1 месяц
        return SECS_IN_ONE_MONTH;

    case Data::StockKey::INTERVAL::HOUR :
        //Свечи длительностью 1 час загрузаются с максимальным интервалом 1 неделя
        return SECS_IN_ONE_WEEK;

    default :
        break;
    }

    //Все свечи длительностью меньше часа имеют максимальный интервал загрузки 1 сутки
    return SECS_IN_ONE_DAY;
}

}
