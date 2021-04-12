#include "daterange.h"

namespace Data {

DateRange::DateRange()
{

}

DateRange::DateRange(DateRange &&range)
{
    begin = std::move(range.begin);
    end = std::move(range.end);
}

DateRange::DateRange(const DateRange &range)
{
    begin = range.begin;
    end = range.end;
}

DateRange::DateRange(const QDateTime &begin_, const QDateTime &end_)
{
    begin = begin_;
    end = end_;
}

DateRange::DateRange(const QDateTime &&begin_, const QDateTime &&end_)
{
    begin = std::move(begin_);
    end = std::move(end_);
}

void DateRange::operator=(DateRange &&range)
{
    begin = std::move(range.begin);
    end = std::move(range.end);
}

void DateRange::operator=(const DateRange &range)
{
    begin = range.begin;
    end = range.end;
}

const QDateTime &DateRange::getBegin() const
{
    return begin;
}

const QDateTime &DateRange::getEnd() const
{
    return end;
}

bool DateRange::isValid() const
{
    if (!begin.isValid() || !end.isValid())
        return false;
    else if (begin >= end)
        return false;

    return true;
}

qint64 DateRange::toSec() const
{
    if (!isValid())
        return 0;

    return begin.secsTo(end);
}

bool DateRange::contains(const QDateTime &date) const
{
    if (!isValid() || !date.isValid())
        return false;

    return begin <= date && date <= end;
}

bool DateRange::contains(const DateRange &range) const
{
    if (!isValid() || !range.isValid())
        return false;

    return contains(range.begin) && contains(range.end);
}

bool DateRange::isIntersected(const DateRange &range) const
{
    if (!isValid() || !range.isValid())
        return false;

    return !(begin > range.end || end < range.begin);
}

void DateRange::setBegin(const QDateTime &begin_)
{
    begin = begin_;
}

void DateRange::setEnd(const QDateTime &end_)
{
    end = end_;
}

void DateRange::setRange(const QDateTime &date, const long &duration)
{
    if (!date.isValid())
        return;

    if (duration >= 0) {
        begin = date;
        end = begin.addSecs(duration);
    } else {    //duration < 0
        end = date;
        begin = end.addSecs(duration);
    }
}

void DateRange::displace(const long &beginSecs, const long &endSecs)
{
    if (!isValid())
        return;

    begin = begin.addSecs(beginSecs);
    end = end.addSecs(endSecs);
}

void DateRange::constrain(const DateRange &range)
{
    if (!isValid() || !range.isValid())
        return;

    if (begin < range.begin)
        begin = range.begin;

    if (end > range.end)
        end = range.end;
}

void DateRange::remove(const DateRange &range)
{
    if (!isIntersected(range))
        return; //диапазоны не пересекаются

    if (begin < range.begin )
        end = range.begin;
    else
        begin = range.end;
}

void DateRange::extend(const DateRange &range)
{
    if (isValid() && range.isValid()) {
        begin = std::min(begin, range.begin);
        end = std::max(end, range.end);
    }
}

}
