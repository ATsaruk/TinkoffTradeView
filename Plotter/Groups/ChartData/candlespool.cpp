#include "candlespool.h"

namespace Plotter {

CandlesPool::CandlesPool(CandlesData *seriesData)
{
    this->_seriesData = seriesData;
}

void CandlesPool::clear()
{
    _items.clear();
}

bool CandlesPool::empty() const
{
    return _items.empty();
}

size_t CandlesPool::size() const
{
    return _items.size();
}

CandleItem *CandlesPool::operator [](const size_t pos)
{
    if (pos >= _items.size()) {
        //Если недостаточно элементов, создаем их!
        Iterator newFirst = _items.end();
        if (!_items.empty())
            --newFirst;

        for (size_t i = _items.size(); i <= pos; i++)
            _items.emplace_back(_seriesData);

        if (newFirst == _items.end())
            newFirst = _items.begin();
        else
            ++newFirst;

        //Отправляем сигнал о новых элементах
        newItems(std::make_pair(newFirst, _items.end()));
    }

    auto itemAtPos = _items.begin();
    for (size_t i = 0; i < pos; ++i, ++itemAtPos);

    return &(*itemAtPos);
}

void CandlesPool::push_front(CandlesPool &pool, const PairRange &it)
{
    auto [from, to] = it;
    _items.splice(_items.begin(), pool._items, from, to);
}

void CandlesPool::push_back(CandlesPool &pool, const PairRange &it)
{
    auto [from, to] = it;
    _items.splice(_items.end(), pool._items, from, to);
}

CandlesPool::PairRange CandlesPool::pop_front(const size_t count)
{
    Iterator it = at(count);
    return std::make_pair(_items.begin(), it);
}

CandlesPool::PairRange CandlesPool::pop_back(const size_t count)
{
    if (count < _items.size()) {
        Iterator it = at(_items.size() - count);
        return std::make_pair(it, _items.end());
    } else {
        return pop_front(count);
    }
}

CandlesPool::Iterator CandlesPool::begin()
{
    return _items.begin();
}

CandlesPool::Iterator CandlesPool::end()
{
    return _items.end();
}

CandlesPool::ReverseIt CandlesPool::rbegin()
{
    return _items.rbegin();
}

CandlesPool::ReverseIt CandlesPool::rend()
{
    return _items.rend();
}

CandleItem* CandlesPool::getBack()
{
    return &_items.back();
}

CandlesPool::Iterator CandlesPool::at(const size_t pos)
{
    if (pos > _items.size()) {
        //Если недостаточно элементов, создаем их!
        Iterator newFirst = _items.end();
        if (!_items.empty())
            --newFirst;

        for (size_t i = _items.size(); i < pos; i++)
            _items.emplace_back(_seriesData);

        if (newFirst == _items.end())
            newFirst = _items.begin();
        else
            ++newFirst;

        //Отправляем сигнал о новых элементах
        newItems(std::make_pair(newFirst, _items.end()));
    }

    Iterator itemAtPos = _items.begin();
    for (size_t i = 0; i < pos; ++i, ++itemAtPos);

    return itemAtPos;
}

}

