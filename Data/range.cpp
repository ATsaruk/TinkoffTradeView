#include "range.h"

namespace Data {

Range::Range()
{

}

Range::Range(const Range &other)
    : _begin(other._begin), _end(other._end)
{
}

Range::Range(const QDateTime &begin, const QDateTime &end)
    : _begin(begin), _end(end)
{
}

Range::Range(const QDateTime &date, const long &duration)
{
    setRange(date, duration);
}

Range &Range::operator =(const Range &other)
{
    _begin = other._begin;
    _end = other._end;
    return *this;
}

QDateTime &Range::begin()
{
    return _begin;
}

QDateTime &Range::end()
{
    return _end;
}

Range::Range(Range &&other) noexcept
    : _begin(std::move(other._begin)), _end(std::move(other._end))
{
}

Range::Range(QDateTime &&begin, QDateTime &&end) noexcept
    : _begin(std::move(begin)), _end(std::move(end))
{
}

Range &Range::operator =(Range &&other) noexcept
{
    _begin = std::move(other._begin);
    _end = std::move(other._end);
    return *this;
}

const QDateTime &Range::begin() const
{
    return _begin;
}

const QDateTime &Range::end() const
{
    return _end;
}

bool Range::isBeginValid() const
{
    return _begin.isValid();
}

bool Range::isEndValid() const
{
    return _end.isValid();
}

bool Range::isValid() const
{
    return _begin.isValid() && _end.isValid() && _begin <= _end;
}

bool Range::isBeginNull() const
{
    return _begin.isNull();
}

bool Range::isEndNull() const
{
    return _end.isNull();
}

bool Range::isNull() const
{
    return _begin.isNull() && _end.isNull();
}

Data::Range::operator bool() const
{
    return isValid();
}

qint64 Range::toSec() const
{
    if (!isValid())
        return 0;

    return _begin.secsTo(_end);
}

bool Range::contains(const QDateTime &date) const
{
    if (isNull() || !date.isValid())
        return false;

    if (isBeginValid() && date < _begin)
        return false;

    if (isEndValid() && date > _end)
        return false;

    return true;
}

bool Range::contains(const Range &other) const
{
    bool valid = false;
    if (isBeginValid() && other.isBeginValid()) {
        if (_begin <= other._begin)
            valid = true;
        else
            return false;
    }

    if (isEndValid() && other.isEndValid()) {
        if (_end >= other._end)
            valid = true;
        else
            return false;
    }

    return valid;
}

bool Range::isIntersected(const Range &other) const
{
    return contains(other._begin) || contains(other._end) || other.contains(_begin) || other.contains(_end);
}

void Range::setRange(const QDateTime &date, const long &duration)
{
    if (!date.isValid())
        return;

    if (duration >= 0) {
        _begin = date;
        _end = date.addSecs(duration);
    } else {
        _begin = date.addSecs(duration);
        _end = date;
    }
}

void Range::shift(const long &secs)
{
    if (isBeginValid())
        _begin = _begin.addSecs(secs);

    if (isEndValid())
        _end = _end.addSecs(secs);
}

void Range::constrain(const Range &other)
{
    if (other.isBeginValid() && (isBeginNull() || _begin < other._begin))
        _begin = other._begin;

    if (other.isEndValid() && (isEndNull() || _end > other._end))
        _end = other._end;

    if (_begin > _end) {
        _begin = QDateTime();
        _end = QDateTime();
    }
}

void Range::remove(const Range &other)
{
    if (!isIntersected(other))
        return; //нет пересечений

    if (other.isBeginValid() && (isBeginNull() || (isBeginValid() && _begin < other._begin)) )
        _end = other._begin;
    else if (other.isEndValid())
        _begin = other._end;

    if (isBeginValid() && isEndValid() && _begin > _end) {
        _begin = QDateTime();
        _end = QDateTime();
    }
}

}
