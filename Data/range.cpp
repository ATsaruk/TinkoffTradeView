#include "range.h"

namespace Data {

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
    else if (begin > end)
        return false;

    return true;
}

Data::Range::operator bool() const
{
    return isValid();
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

bool Range::contains(const Range &other) const
{
    if (!isValid() || !other.isValid())
        return false;

    return contains(other.begin) && contains(other.end);
}

bool Range::isIntersected(const Range &other) const
{
    if (!isValid() || !other.isValid())
        return false;

    return contains(other.begin) || contains(other.end);
}

bool Range::operator >(const Range &other) const
{
    if (!isValid() || !other.isValid())
        return false;

    return begin > other.getEnd();
}

bool Range::operator <(const Range &other) const
{
    if (!isValid() || !other.isValid())
        return false;

    return end < other.getBegin();
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

void Range::addSecs(const long secs)
{
    if (!isValid())
        return;

    begin = begin.addSecs(secs);
    end = end.addSecs(secs);
}

void Range::constrain(const Range &other)
{
    if (!isValid() || !other.isValid())
        return;

    if (begin < other.begin)
        begin = other.begin;

    if (end > other.end)
        end = other.end;
}

void Range::remove(const Range &other)
{
    if (!isIntersected(other))
        return; //диапазоны не пересекаются

    if (begin < other.begin )
        end = other.begin;
    else
        begin = other.end;
}

Range Range::remove(const Range &other) const
{
    if (!isIntersected(other))
        return Range(); //диапазоны не пересекаются

    Range result = *this;
    if (result.begin < other.begin )
       result.end = other.begin;
    else
        result.begin = other.end;

    return result;
}

void Range::extend(const Range &other)
{
    if (isValid() && other.isValid()) {
        begin = std::min(begin, other.begin);
        end = std::max(end, other.end);
    }
}

}
