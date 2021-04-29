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

#include "candleitem.h"
#include "../chartseries.h"
#include "Data/range.h"

namespace Plotter {


class CandlesSeries : public ChartSeries
{
public:
    explicit CandlesSeries();
    ~CandlesSeries();

public slots:
    void repaint() override;

protected:
    void updateData() override;
    bool clear() override;

    void updateScale();
    void scaleByXAxis();
    void scaleByYAxis();
    void setCandleVisible(const std::map<long, CandleItem*>::iterator &first_iterator, const std::map<long, CandleItem*>::iterator &second_iterator);
    void updatePriceRange();
    const QDateTime getDateByIndex(const long index);

    void loadData(const Data::Range &range);
    void addCandle(const Data::Candle &candleData);
    void addCandles();

protected slots:
    void dataChanged();
    void loadTaskFinished();

private:
    uint drawWait;
    bool autoPriceRange = true;    //Перенести в ChartVerticalAxis!
    bool isDataRequested = false;
    bool isDataChanged = false;
    QMutex drawMutex;
    //Данные для рисования
    std::map<long, CandleItem*>::iterator beginCandle;
    std::map<long, CandleItem*>::iterator endCandle;
    std::map<long, CandleItem*> candleItems;
};

}

#endif // CANDLESSERIES_H
