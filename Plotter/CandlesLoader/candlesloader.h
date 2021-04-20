#ifndef CANDLESLOADER_H
#define CANDLESLOADER_H

#include <QObject>

#include "Data/daterange.h"
#include "Data/Stock/stockkey.h"

namespace Plotter {


class CandlesLoader : public QObject
{
public:
    explicit CandlesLoader(QObject *parent = nullptr);
    virtual ~CandlesLoader();

    virtual void askCandles(const Data::Range &range);
    virtual void setStockKey(const Data::StockKey &stockKey, Data::Range &&range);

signals:
    void candlesRecieved();

protected:
    bool isRequestExecuted;
    bool isNewRequestReceived;
    Data::Range requestedRange;
    Data::Range requiredRange;
    Data::StockKey key;

    virtual void requestCandles() = 0;

private:
    Q_OBJECT
};

}

#endif // CANDLESLOADER_H
