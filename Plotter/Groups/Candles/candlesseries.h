/* Изменить:
 *  1. При количестве свечей > X часть свечей не влияющий на общий вид (мелких свечей) удалять с экрана
 *     - добавить свойство свечи strength - как уровень значимости, 0..CandlesCount, чем значение ниже, тем свеча важнее
 *     - если она внутри диапазона цена предыдущих и последующих свечей, то значение устанавливается = количеству свечей на экране при котором её можно упустить
 *     - диапазон вычислять в зависимости от кол-ва свечей и ширины экрана
 *     - значимость свечей пересчитывается при перемещении по экрану у вновь отображаемых свечей
 *     - значимость свечей пересчитывается у всех отображаемых свечей при отдалении графика
 *     - при увеличении графика пересчет не производится, а только отображаются свечи у которых параметр strength < currentCandlesCount
 */

#ifndef CANDLESSERIES_H
#define CANDLESSERIES_H

#include <QMutex>
#include <QObject>

#include "../chartseries.h"
#include "candleitem.h"
#include "candleparams.h"

#include "Data/range.h"

namespace Plotter {


class CandlesSeries : public ChartSeries
{
    Q_OBJECT

public:
    explicit CandlesSeries(const Data::StockKey &stockKey);
    ~CandlesSeries();

public slots:
    void repaint() override;
    void loadCandlesFinished();

signals:
    void requestData(const Data::Range &);

protected:
    void clear() override;

    void updateData();
    void scaleByXAxis();
    void scaleByYAxis();
    void setCandleVisible(const std::map<int32_t, CandleItem*>::iterator &first_iterator, const std::map<int32_t, CandleItem*>::iterator &second_iterator);
    void updatePriceRange();
    const QDateTime getDateByIndex(const int32_t index);

    void addCandle(Data::Candle &&candleData);
    void addCandles(Data::Candles &&candles);

protected slots:
    void setXScale(qreal scale);

private:
    uint drawWait;
    QMutex drawMutex;
    CandleParams candleParams;

    bool autoPriceRange = true;    //Перенести в ChartVerticalAxis!
    bool isDataRequested = false;

    ///@todo разобратся с типом контейнера!
    //Данные для рисования
    //QList<CandleItem*> candleItems;
    //qsizetype beginCandle;
    //qsizetype endCandle;

    std::map<int32_t, CandleItem*> candleItems;
    std::map<int32_t, CandleItem*>::iterator beginCandle;
    std::map<int32_t, CandleItem*>::iterator endCandle;
};

}

#endif // CANDLESSERIES_H
