#include <QJsonObject>

#include "candle.h"

namespace Data {

Candle Candle::fromJson(const QJsonObject &json)
{
    Candle newCandle;
    newCandle.open = json.value("o").toDouble();
    newCandle.close = json.value("c").toDouble();
    newCandle.high = json.value("h").toDouble();
    newCandle.low = json.value("l").toDouble();
    newCandle.volume = json.value("v").toInt();

    //QDateTime от брокера приходт со непонятной временной зоной, которая потом создает кучу проблем, приходится пересобирать дату
    const QDateTime &j = json.value("time").toVariant().toDateTime().addSecs(3*60*60);
    newCandle.dateTime = QDateTime( QDate(j.date().year(),
                                          j.date().month(),
                                          j.date().day()),
                                    QTime(j.time().hour(),
                                          j.time().minute(),
                                          j.time().second()) );

    return newCandle;
}

}
