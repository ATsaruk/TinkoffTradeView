#ifndef CANDLESDATA_H
#define CANDLESDATA_H

#include <QPen>
#include <QBrush>
#include <QObject>

#include "candlespool.h"
#include "Data/range.h"
#include "Data/Stock/stockkey.h"

namespace Plotter {

//class CandleItem;
//class ChartSeries;

class CandlesData : public QObject
{
    Q_OBJECT

public:
    explicit CandlesData();
    ~CandlesData();

    void update(const long long newIndex, const size_t candlesCount);

    void waitForRequestData() const;

    void setStockKey(const Data::StockKey &key);
    const Data::StockKey& getStockKey() const;

    CandleItem* operator [](const long long index);

    void clear();
    [[nodiscard]] bool empty() const;

    CandlesPool::Iterator begin();
    CandlesPool::Iterator end();

signals:
    void updateItems();
    void newItems(CandlesPool::PairRange);

protected:
    void requestCandles(const Data::Range &range, const size_t requiredCount);

protected slots:
    void recieveCandles();

private:
    //Параметры масштабирования
    qreal _clearance = 0.;           //Зазор по оси Х между свечами
    long long _candlesCount = 0;     //Количество отображаемых свечей
    long long _offsetIndex = 0;      //Индекс первой отображаемой свечи
    qreal _xScale = 0;               //Масштаб по оси Х
    qreal _yScale = 0;               //Масштаб по оси Y
    bool _autoPriceRange = true;     //Режим автомастабирования оси Y по цене //@note возможно его нужно от сюда перенести?

    //Pen и Brush для отрисовки свечи (обычно бычьи свечи - красные, медвежьи свечи - зеленые)
    QPen _bearPen, _bullPen;
    QBrush _bearBrush, _bullBrush;

    //Ключи акции для которой предназначены данные
    Data::StockKey _stockKey;

    //Свечная информация
    bool _isDataRequested = false;
    CandlesPool _candles;            //Список свечей
    CandlesPool _candlesPool;        //Список неиспользуемых свечей (по мотивам паттерна Object Pool)

    friend class Axis;
    friend class CandleItem;
    friend class ChartSeries;
    friend class CandlesSeries;
};

}

#endif // CANDLESDATA_H
