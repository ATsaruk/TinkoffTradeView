#include <QJsonObject>

#include "tinkoff.h"

namespace Broker {

TinkoffApi::TinkoffApi()
{
    html = new Request;
    connect(html, &Request::getResopnse, this, &Api::getResopnse);
}

TinkoffApi::~TinkoffApi()
{
    delete html;
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

}
