#include "candleslist.h"

#include "Core/globals.h"
#include "Data/StockView/stockviewreference.h"

namespace Plotter {


Plotter::CandlesListIterator::CandlesListIterator(const Plotter::CandlesListIterator &it)
    : ptr(it.ptr)
{

}

CandlesListIterator CandlesListIterator::operator++()
{
    if (ptr)
        ptr = ptr->next;
    return CandlesListIterator(ptr);
}

ItemType *CandlesListIterator::operator &() const
{
    return ptr;
}

Plotter::CandlesListIterator::operator bool() const
{
    return ptr != nullptr;
}

typename CandlesListIterator::reference CandlesListIterator::operator*() const
{
    if (!ptr)
        throw std::logic_error("call CandlesListIterator::operator*() without object!");
    return ptr->data;
}

CandleItem *CandlesListIterator::operator ->() const
{
    if (!ptr)
        throw std::logic_error("call CandlesListIterator::operator ->() without object!");
    return &ptr->data;
}

bool CandlesListIterator::operator!=(const CandlesListIterator &other) const
{
    return ptr != other.ptr;
}



CandlesList::CandlesList(std::shared_ptr<SeriesData> seriesData, CandlesListIterator list_first)
    : firstItem(list_first), data(seriesData)
{
    lastItem = last(firstItem);
}

CandlesList::~CandlesList()
{
    clear();
}

void CandlesList::clear()
{
    std::for_each(begin(), end(), [](auto &it){ delete &it; });
    firstItem = nullptr;
    lastItem = nullptr;
}

bool CandlesList::empty() const
{
    return !firstItem;
}

size_t CandlesList::size() const
{
    size_t size = 0;
    for (auto it = firstItem; it; ++it, ++size);
    return size;
}

CandlesListIterator CandlesList::operator [](const size_t pos)
{
    CandlesListIterator itemAtPos = firstItem;
    for (size_t i = 0; i < pos; ++i, ++itemAtPos) {
        if (!itemAtPos) {
            //вышли за предел списка, добавляем недостающие элементы!
            itemAtPos = new ItemType(data);
            if (lastItem)
                (&lastItem)->next = &itemAtPos;
            else    //если нет lastItem, то нет и firstItem!
                firstItem = itemAtPos;  //список пуст, запоминаем начало
            lastItem = itemAtPos;
        }
    }
    return itemAtPos;
}

void CandlesList::push_front(CandlesListIterator ptr)
{
    if (!ptr)
        return;

    auto cur = last(ptr);
    (&cur)->next = &firstItem;

    firstItem = ptr;
    if (!lastItem)
        lastItem = cur;
}

void CandlesList::push_back(CandlesListIterator ptr)
{
    if (!ptr)
        return;

    if (lastItem)
        (&lastItem)->next = &ptr;
    else
        firstItem = ptr;

    lastItem = last(ptr);
}

CandlesListIterator CandlesList::pop_front(const size_t count)
{
    size_t cur_size = size();

    CandlesListIterator lastPopedItem = (*this)[count - 1]; //-1 т.к. count = [1, 2, 3...], а (*this)[i], i = [0, 1, 2, ...]

    auto result = firstItem;
    if (cur_size <= count) {            //pop весь список, или даже больше...
        firstItem = nullptr;
        lastItem = nullptr;
    } else {                            //pop часть списка
        firstItem = (&lastPopedItem)->next;
        (&lastPopedItem)->next = nullptr;
    }
    return result;
}

CandlesListIterator CandlesList::pop_back(const size_t count)
{
    size_t cur_size = size();
    size_t index = (count < cur_size)
            ? (count - 1)               //-1 т.к. count = [1, 2, 3...], а (*this)[i], i = [0, 1, 2, ...]
            : (cur_size - count - 1);   /* В данном случае "-1" в cur_size и "-1" в count скоменсируют друг друга,
                                         * но все равно -1, т.к. нам нужен элемент перед запрашиваемым, который станет
                                         * последним в текущем списке, а так же позволит разорвать список! */
    auto newLast = (*this)[index];

    if (count >= cur_size) {            //pop весь список, или даже больше...
        auto result = firstItem;
        firstItem = nullptr;
        lastItem = nullptr;
        return result;
    } else {                            //pop часть списка
        auto result = (&newLast)->next;
        lastItem = newLast;
        (&lastItem)->next = nullptr;
        return result;
    }
}

CandlesListIterator CandlesList::begin()
{
    return CandlesListIterator(firstItem);
}

CandlesListIterator CandlesList::end()
{
    return nullptr;
}

const CandlesListIterator CandlesList::begin() const
{
    return CandlesListIterator(firstItem);
}

const CandlesListIterator CandlesList::end() const
{
    return nullptr;
}

CandlesListIterator CandlesList::last(CandlesListIterator start) const
{
    if (!firstItem)
        return nullptr;

    if (!start) //start = nullptr, значит возвращаем последний элемент в текущем списке
        start = firstItem;

    auto cur = start;
    while ((&cur)->next != nullptr)
        ++cur;

    return CandlesListIterator(cur);
}

}
