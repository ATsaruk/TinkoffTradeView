#include <QJsonObject>

#include "candle.h"

namespace Data {

const QDateTime &Candle::dateTime() const
{
    return _dateTime;
}

const float &Candle::open() const
{
    return _open;
}

const float &Candle::close() const
{
    return _close;
}

const float &Candle::high() const
{
    return _high;
}

const float &Candle::low() const
{
    return _low;
}

const long long &Candle::volume() const
{
    return _volume;
}

Candle Candle::fromJson(const QJsonObject &json)
{
    float open = json.value("o").toDouble();
    float close = json.value("c").toDouble();
    float high = json.value("h").toDouble();
    float low = json.value("l").toDouble();
    uint volume = json.value("v").toInt();

    //QDateTime от брокера приходт со непонятной временной зоной, которая потом создает кучу проблем, приходится пересобирать дату
    const QDateTime &j = json.value("time").toVariant().toDateTime().addSecs(3*60*60);
    QDateTime date = QDateTime( QDate(j.date().year(),
                                          j.date().month(),
                                          j.date().day()),
                                    QTime(j.time().hour(),
                                          j.time().minute(),
                                          j.time().second()) );

    return Candle(date, open, close, high, low, volume);
}

}
