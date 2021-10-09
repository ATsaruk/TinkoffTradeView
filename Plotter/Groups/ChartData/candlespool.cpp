#include "candlespool.h"

namespace Plotter {

CandlesPool::CandlesPool(CandlesData *seriesData)
{
    this->seriesData = seriesData;
}

void CandlesPool::clear()
{
    items.clear();
}

bool CandlesPool::empty() const
{
    return items.empty();
}

size_t CandlesPool::size() const
{
    return items.size();
}

CandleItem *CandlesPool::operator [](const size_t pos)
{
    if (pos >= items.size()) {
        //Если недостаточно элементов, создаем их!
        Iterator newFirst = items.end();
        if (!items.empty())
            --newFirst;

        for (size_t i = items.size(); i <= pos; i++)
            items.emplace_back(seriesData);

        if (newFirst == items.end())
            newFirst = items.begin();
        else
            ++newFirst;

        //Отправляем сигнал о новых элементах
        newItems(std::make_pair(newFirst, items.end()));
    }

    auto itemAtPos = items.begin();
    for (size_t i = 0; i < pos; ++i, ++itemAtPos);

    return &(*itemAtPos);
}

void CandlesPool::push_front(CandlesPool &pool, const PairRange &it)
{
    auto [from, to] = it;
    items.splice(items.begin(), pool.items, from, to);
}

void CandlesPool::push_back(CandlesPool &pool, const PairRange &it)
{
    auto [from, to] = it;
    items.splice(items.end(), pool.items, from, to);
}

CandlesPool::PairRange CandlesPool::pop_front(const size_t count)
{
    Iterator it = at(count);
    return std::make_pair(items.begin(), it);
}

CandlesPool::PairRange CandlesPool::pop_back(const size_t count)
{
    if (count < items.size()) {
        Iterator it = at(items.size() - count);
        return std::make_pair(it, items.end());
    } else {
        return pop_front(count);
    }
}

CandlesPool::Iterator CandlesPool::begin()
{
    return items.begin();
}

CandlesPool::Iterator CandlesPool::end()
{
    return items.end();
}

CandlesPool::ReverseIt CandlesPool::rbegin()
{
    return items.rbegin();
}

CandlesPool::ReverseIt CandlesPool::rend()
{
    return items.rend();
}

CandleItem* CandlesPool::getBack()
{
    return &items.back();
}

CandlesPool::Iterator CandlesPool::at(const size_t pos)
{
    if (pos > items.size()) {
        //Если недостаточно элементов, создаем их!
        Iterator newFirst = items.end();
        if (!items.empty())
            --newFirst;

        for (size_t i = items.size(); i < pos; i++)
            items.emplace_back(seriesData);

        if (newFirst == items.end())
            newFirst = items.begin();
        else
            ++newFirst;

        //Отправляем сигнал о новых элементах
        newItems(std::make_pair(newFirst, items.end()));
    }

    Iterator itemAtPos = items.begin();
    for (size_t i = 0; i < pos; ++i, ++itemAtPos);

    return itemAtPos;
}

}

