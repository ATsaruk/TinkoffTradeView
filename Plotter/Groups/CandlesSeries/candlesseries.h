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

namespace Plotter {

class CandlesSeries : public ChartSeries
{
    Q_OBJECT

public:
    explicit CandlesSeries(CandlesData *candlesData);
    ~CandlesSeries();

public slots:
    void repaint() override;

protected:
    void clear() override;

    void updateVisibleCandles();
    void updatePriceRange();
    void updateCandlesPos();

protected slots:
    void askForRepaint();
    void appendCandles(CandlesPool::PairRange range);

private:
    uint _drawWait;
    QMutex _drawMutex;
};

}

#endif // CANDLESSERIES_H
