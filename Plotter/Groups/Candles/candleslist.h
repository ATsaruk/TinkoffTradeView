#ifndef CANDLESLIST_H
#define CANDLESLIST_H

#include "candleitem.h"
#include "seriesdata.h"

namespace Plotter {

///@todo подумать можно ли обойтись std::list
///@todo возможно стоит сделать экран влево и экран вправо запас по свечам

struct ItemType {
    CandleItem data;
    ItemType *next = nullptr;

    ItemType(std::shared_ptr<SeriesData> seriesData) : data(seriesData) {}
};

class CandlesListIterator: public std::iterator<std::input_iterator_tag, ItemType, ptrdiff_t, CandleItem*, CandleItem&>
{
public:
    CandlesListIterator(const CandlesListIterator &it);

    CandlesListIterator operator++();
    ItemType* operator & () const;
    operator bool() const;

    typename CandlesListIterator::reference operator*() const;
    CandleItem* operator ->() const;
    bool operator!=(CandlesListIterator const& other) const;

private:
    ItemType* ptr;
    CandlesListIterator(ItemType* p = nullptr) : ptr(p) {}
    friend class CandlesList;
};

class CandlesList
{
public:
    explicit CandlesList(std::shared_ptr<SeriesData> seriesData, CandlesListIterator list_first = nullptr);
    ~CandlesList();

    CandlesList(CandlesList &&) noexcept = default;
    CandlesList(const CandlesList &) = default;
    CandlesList& operator = (CandlesList &&) noexcept = default;
    CandlesList& operator = (const CandlesList &) = default;

    void clear();
    [[nodiscard]] bool empty() const;
    size_t size() const;

    //Если pos > size, то создает недостающие элементы!
    CandlesListIterator operator [] (const size_t pos);

    //Функции push_front и push_back вставляют список в начало/в конец, ptr - указатель на начало списка
    void push_front(CandlesListIterator ptr);
    void push_back(CandlesListIterator ptr);

    //Функции возвращают список из count элементов, элементы "отрываются" из начала/конца
    //функции гарантированно возвращает список из count элементов
    //если this->size() < count, то недостающие элементы будут созданы
    CandlesListIterator pop_front(const size_t count);
    CandlesListIterator pop_back(const size_t count);

    //Итераторы для стандартных алгоритмов
    CandlesListIterator begin();
    CandlesListIterator end();
    const CandlesListIterator begin() const;
    const CandlesListIterator end() const;

    CandlesListIterator last(CandlesListIterator start = nullptr) const;

private:
    CandlesListIterator firstItem;
    CandlesListIterator lastItem;

    std::shared_ptr<SeriesData> data;
};

}

#endif // CANDLESLIST_H
