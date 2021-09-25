#include "range.h"

namespace Data {

constexpr quint64 SECS_IN_ONE_DAY   = 24 * 3600;

Range::Range()
{

}

Range::Range(const Range &range)
{
    begin = range.begin;
    end = range.end;
}

Range::Range(const QDateTime &begin_, const QDateTime &end_)
{
    begin = begin_;
    end = end_;
}

Range::Range(const QDateTime &date, const long &duration)
{
    setRange(date, duration);
}

Range &Range::operator =(const Range &range)
{
    begin = range.begin;
    end = range.end;
    return *this;
}

Range::Range(Range &&range) noexcept
{
    begin = std::move(range.begin);
    end = std::move(range.end);
}

Range::Range(QDateTime &&begin_, QDateTime &&end_) noexcept
{
    begin = std::move(begin_);
    end = std::move(end_);
}

Range &Range::operator =(Range &&range) noexcept
{
    begin = std::move(range.begin);
    end = std::move(range.end);
    return *this;
}

const QDateTime &Range::getBegin() const
{
    return begin;
}

const QDateTime &Range::getEnd() const
{
    return end;
}

bool Range::isValid() const
{
    if (!begin.isValid() || !end.isValid())
        return false;
    else if (begin >= end)
        return false;

    return true;
}

Data::Range::operator bool() const
{
    return isValid();
}

bool Range::operator !() const
{
    return  !isValid();
}

qint64 Range::toSec() const
{
    if (!isValid())
        return 0;

    return begin.secsTo(end);
}

bool Range::contains(const QDateTime &date) const
{
    if (!isValid() || !date.isValid())
        return false;

    return begin <= date && date <= end;
}

bool Range::contains(const Range &range) const
{
    if (!isValid() || !range.isValid())
        return false;

    return contains(range.begin) && contains(range.end);
}

bool Range::isIntersected(const Range &range) const
{
    if (!isValid() || !range.isValid())
        return false;

    return !(begin > range.end || end < range.begin);
}

bool Range::operator >(const Range &range) const
{
    if (!isValid() || !range.isValid())
        return false;

    return begin > range.getEnd();
}

bool Range::operator <(const Range &range) const
{
    if (!isValid() || !range.isValid())
        return false;

    return end < range.getBegin();
}

void Range::setBegin(const QDateTime &begin_)
{
    begin = begin_;
}

void Range::setEnd(const QDateTime &end_)
{
    end = end_;
}

void Range::setRange(const QDateTime &date, const long &duration)
{
    if (!date.isValid())
        return;

    if (duration >= 0) {
        begin = date;
        end = date.addSecs(duration);
    } else {
        begin = date.addSecs(duration);
        end = date;
    }
}

void Range::addSecs(const long &secs)
{
    displace(secs, secs);
}

void Range::addDays(const long &days)
{
    long secs = days * SECS_IN_ONE_DAY;
    displace(secs, secs);
}

void Range::displace(const long &beginSecs, const long &endSecs)
{
    if (!isValid())
        return;

    begin = begin.addSecs(beginSecs);
    end = end.addSecs(endSecs);
}

void Range::constrain(const Range &range)
{
    if (!isValid() || !range.isValid())
        return;

    if (begin < range.begin)
        begin = range.begin;

    if (end > range.end)
        end = range.end;
}

void Range::remove(const Range &range)
{
    if (!isIntersected(range))
        return; //диапазоны не пересекаются

    if (begin < range.begin )
        end = range.begin;
    else
        begin = range.end;
}

Range Range::remove(const Range &range) const
{
    if (!isIntersected(range))
        return Range(); //диапазоны не пересекаются

    Range result = *this;
    if (result.begin < range.begin )
       result.end = range.begin;
    else
        result.begin = range.end;

    return result;
}

void Range::extend(const Range &range)
{
    if (isValid() && range.isValid()) {
        begin = std::min(begin, range.begin);
        end = std::max(end, range.end);
    }
}

}
