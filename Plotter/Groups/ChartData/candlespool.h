#ifndef CANDLESPOOL_H
#define CANDLESPOOL_H

#include <QObject>
#include <list>

#include "../CandlesSeries/candleitem.h"

namespace Plotter {

class CandlesData;

///@todo возможно стоит сделать экран влево и экран вправо запас по свечам

class CandlesPool : public QObject
{
    Q_OBJECT

public:
    using Iterator = std::_List_iterator<Plotter::CandleItem>;
    using ReverseIt = std::reverse_iterator<Iterator>;
    using PairRange = std::pair<Iterator, Iterator>;

    explicit CandlesPool(CandlesData *seriesData);

    void clear();
    [[nodiscard]] bool empty() const;
    size_t size() const;

    CandleItem* operator [] (const size_t pos);     //??????Вопрос по возвращаемому типу, может CandleItem* ?

    //Функции push_front и push_back вставляют список в начало/в конец, ptr - указатель на начало списка
    void push_front(CandlesPool &pool, const PairRange &it);
    void push_back(CandlesPool &pool, const PairRange &it);

    //Функции возвращают список из count элементов, элементы "отрываются" из начала/конца
    //функции гарантированно возвращает список из count элементов
    //если this->size() < count, то недостающие элементы будут созданы
    PairRange pop_front(const size_t count);
    PairRange pop_back(const size_t count);

    //Итераторы для стандартных алгоритмов
    Iterator begin();
    Iterator end() ;

    ReverseIt rbegin();
    ReverseIt rend() ;

    CandleItem *getBack();

signals:
    void newItems(CandlesPool::PairRange);

protected:

    //Если pos > size, то создает недостающие элементы!
    Iterator at(const size_t pos);

private:
    std::list<CandleItem> _items;

    CandlesData *_seriesData;
};

}

#endif // CANDLESPOOL_H
