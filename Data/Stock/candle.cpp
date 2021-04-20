#include <QJsonObject>

#include "candle.h"

namespace Data {

void Candle::fromJson(const QJsonObject &json)
{
    open = json.value("o").toDouble();
    close = json.value("c").toDouble();
    high = json.value("h").toDouble();
    low = json.value("l").toDouble();
    volume = json.value("v").toInt();

    //QDateTime от брокера приходт со непонятной временной зоной, которая потом создает кучу проблем, приходится пересобирать дату
    const QDateTime &j = json.value("time").toVariant().toDateTime().addSecs(3*60*60);
    dateTime = QDateTime( QDate(j.date().year(),
                                j.date().month(),
                                j.date().day()),
                          QTime(j.time().hour(),
                                j.time().minute(),
                                j.time().second()) );
}

}
