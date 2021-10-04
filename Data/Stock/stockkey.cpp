#include <QJsonObject>

#include "stockkey.h"
#include "Core/globals.h"

namespace Data {


StockKey::StockKey()
{

}

StockKey::StockKey(const QString &figi, const INTERVAL &interval)
{
    _figi = figi;
    _interval = interval;
}

StockKey &StockKey::operator =(const StockKey &other)
{
    _figi = other.figi();
    _interval = other.interval();
    return *this;
}

const QString &StockKey::figi() const
{
    return _figi;
}

const StockKey::INTERVAL &StockKey::interval() const
{
    return _interval;
}

long StockKey::candleLenght() const
{
    switch (_interval) {
      case INTERVAL::MIN1  : return 60;           break;
      case INTERVAL::MIN5  : return 5*60;         break;
      case INTERVAL::MIN15 : return 15*60;        break;
      case INTERVAL::MIN30 : return 30*60;        break;
      case INTERVAL::HOUR  : return 60*60;        break;
      case INTERVAL::DAY   : return 24*60*60;     break;
      case INTERVAL::WEEK  : return 7*24*60*60;   break;
    }
    logCritical << "StockKey::intervalToSec;unknow interval";
    throw std::bad_exception();
}

QString StockKey::intervalToString() const
{
    switch (_interval) {
      case INTERVAL::MIN5  : return QString("5min");
      case INTERVAL::MIN15 : return QString("15min");
      case INTERVAL::MIN30 : return QString("30min");
      case INTERVAL::MIN1  : return QString("1min");
      case INTERVAL::HOUR  : return QString("hour");
      case INTERVAL::DAY   : return QString("day");
      case INTERVAL::WEEK  : return QString("week");
    }
    logCritical << "StockKey::intervalToString;unknow interval";
    throw std::bad_exception();
}

QDateTime StockKey::prevCandleTime(const QDateTime &time) const
{
    return time.addSecs(-candleLenght());
}

QDateTime StockKey::nextCandleTime(const QDateTime &time) const
{
    return time.addSecs(candleLenght());
}

QString StockKey::keyToString() const
{
    return QString("%1:%2").arg(_figi, intervalToString());
}

StockKey::INTERVAL StockKey::stringToInterval(QString stringInterval) const
{
    stringInterval = stringInterval.toLower();
    if (stringInterval == QString("1min"))       return INTERVAL::MIN1;
    else if (stringInterval == QString("5min"))  return INTERVAL::MIN5;
    else if (stringInterval == QString("15min")) return INTERVAL::MIN15;
    else if (stringInterval == QString("30min")) return INTERVAL::MIN30;
    else if (stringInterval == QString("hour"))  return INTERVAL::HOUR;
    else if (stringInterval == QString("day"))   return INTERVAL::DAY;
    else if (stringInterval == QString("week"))  return INTERVAL::WEEK;

    throw std::invalid_argument( QString("StockKey::stringToInterval;throw unknow interval: %1").arg(stringInterval).toUtf8().data() );
}

void StockKey::fromJson(const QJsonObject &json)
{
    _figi = json.value("figi").toString();
    _interval = stringToInterval( json.value("interval").toString() );
}

}
