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
#include "candleslist.h"
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

protected:
    void clear() override;

    void updateVisibleCandles();
    void updateScaleByXAxis();
    void updateScaleByYAxis();
    void updatePriceRange();
    void updateCandlesPos();

    void requestCandles(const Data::Range &range, const size_t requiredCount);
    void popFrontCandles(const long long count);
    void popBackCandles(const long long count);

    //void setCandleVisible(const CandleItems::iterator &first_iterator, const CandleItems::iterator &second_iterator);

    ///Добавляем загруженные свечи
    /*void addCandles(Data::Candles &&candles);
    ///Удалям свечи, которые уже существуют
    void removeExistedCandles(Data::Candles &candles);*/

    //const QDateTime getDateByIndex(const int32_t index);

protected slots:
    void loadCandlesFinished();

private:
    uint drawWait;
    QMutex drawMutex;
    bool isDataRequested = false;
    bool isUpdatePosRequered = false;

    ItemType *unset = nullptr;
    CandlesList candles;
    CandlesList unused;
};

}

#endif // CANDLESSERIES_H
