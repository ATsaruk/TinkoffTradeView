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
        end = begin.addSecs(duration);
    } else {    //duration < 0
        end = date;
        begin = end.addSecs(duration);
    }
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

void Range::extend(const Range &range)
{
    if (isValid() && range.isValid()) {
        begin = std::min(begin, range.begin);
        end = std::max(end, range.end);
    }
}

}
