#include "range.h"

namespace Data {

Range::Range()
{

}

Range::Range(const Range &other)
    : _start(other._start), _end(other._end)
{
}

Range::Range(const QDateTime &start, const QDateTime &end)
    : _start(start), _end(end)
{
}

Range::Range(const QDateTime &date, const long &duration)
{
    setRange(date, duration);
}

Range &Range::operator =(const Range &other)
{
    _start = other._start;
    _end = other._end;
    return *this;
}

bool Range::operator<(const Range &other) const
{
    return this->isEndValid() && other.isStartValid() && this->end() < other.start();
}

bool Range::operator>(const Range &other) const
{
    return this->isStartValid() && other.isEndValid() && this->start() > other.end();
}

QDateTime &Range::start()
{
    return _start;
}

QDateTime &Range::end()
{
    return _end;
}

Range::Range(Range &&other) noexcept
    : _start(std::move(other._start)), _end(std::move(other._end))
{
}

Range::Range(QDateTime &&start, QDateTime &&end) noexcept
    : _start(std::move(start)), _end(std::move(end))
{
}

Range &Range::operator =(Range &&other) noexcept
{
    _start = std::move(other._start);
    _end = std::move(other._end);
    return *this;
}

const QDateTime &Range::start() const
{
    return _start;
}

const QDateTime &Range::end() const
{
    return _end;
}

bool Range::isStartValid() const
{
    return _start.isValid();
}

bool Range::isEndValid() const
{
    return _end.isValid();
}

bool Range::isValid() const
{
    return _start.isValid() && _end.isValid() && _start <= _end;
}

bool Range::isStartNull() const
{
    return _start.isNull() || !_start.isValid();
}

bool Range::isEndNull() const
{
    return _end.isNull() || !_end.isValid();;
}

bool Range::isNull() const
{
    return isStartNull() && isEndNull();
}

Data::Range::operator bool() const
{
    return isValid();
}

qint64 Range::toSec() const
{
    if (!isValid())
        return 0;

    return _start.secsTo(_end);
}

bool Range::contains(const QDateTime &date) const
{
    if (isNull() || !date.isValid())
        return false;

    if (isStartValid() && date < _start)
        return false;

    if (isEndValid() && date > _end)
        return false;

    return true;
}

bool Range::contains(const Range &other) const
{
    bool valid = false;
    if (isStartValid() && other.isStartValid()) {
        if (_start <= other._start)
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
    return contains(other._start) || contains(other._end) || other.contains(_start) || other.contains(_end);
}

void Range::setRange(const QDateTime &date, const long &duration)
{
    if (!date.isValid())
        return;

    if (duration >= 0) {
        _start = date;
        _end = date.addSecs(duration);
    } else {
        _start = date.addSecs(duration);
        _end = date;
    }
}

void Range::shift(const long &secs)
{
    if (isStartValid())
        _start = _start.addSecs(secs);

    if (isEndValid())
        _end = _end.addSecs(secs);
}

void Range::constrain(const Range &other)
{
    if (other.isStartValid() && (isStartNull() || _start < other._start))
        _start = other._start;

    if (other.isEndValid() && (isEndNull() || _end > other._end))
        _end = other._end;

    if (_start > _end) {
        _start = QDateTime();
        _end = QDateTime();
    }
}

void Range::remove(const Range &other)
{
    if (!isIntersected(other))
        return; //нет пересечений

    if (other.isStartValid() && (isStartNull() || (isStartValid() && _start < other._start)) )
        _end = other._start;
    else if (other.isEndValid())
        _start = other._end;

    if (isStartValid() && isEndValid() && _start > _end) {
        _start = QDateTime();
        _end = QDateTime();
    }
}

}
