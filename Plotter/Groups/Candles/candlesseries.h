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

#include "Data/range.h"
#include "Data/Stock/candle.h"

namespace Plotter {

using CandleItems = std::map<int32_t, std::shared_ptr<CandleItem>>;

class CandlesSeries : public ChartSeries
{
    Q_OBJECT

public:
    explicit CandlesSeries(const Data::StockKey &stockKey);
    ~CandlesSeries();

public slots:
    void repaint() override;
    //void loadCandlesFinished();

signals:
    void requestData(const Data::Range &);

protected:
    void clear() override;

    void updateScaleByXAxis();
    void updateScaleByYAxis();
    void updatePriceRange();
    void updateCandlesPos();

    void updateData();
    void setCandleVisible(const CandleItems::iterator &first_iterator, const CandleItems::iterator &second_iterator);

    ///Добавляем загруженные свечи
    /*void addCandles(Data::Candles &&candles);
    ///Удалям свечи, которые уже существуют
    void removeExistedCandles(Data::Candles &candles);*/

    const QDateTime getDateByIndex(const int32_t index);

private:
    uint drawWait;
    QMutex drawMutex;
    bool isDataRequested = false;
    bool isUpdatePosRequered = false;

    CandleItems candleItems;
    CandleItems::iterator beginCandle;
    CandleItems::iterator endCandle;
};

}

#endif // CANDLESSERIES_H
