#include "candlesloader.h"
#include "Core/global.h"

namespace Plotter {


CandlesLoader::CandlesLoader(QObject *parent) : QObject(parent)
{
    isRequestExecuted = false;
    isNewRequestReceived = false;
}

CandlesLoader::~CandlesLoader()
{

}

void CandlesLoader::askCandles(const Data::Range &range)
{
    assert (key.figi().isEmpty() && "StockKey is not specified");

    requiredRange.extend(range);

    requestCandles();
}

void CandlesLoader::setStockKey(const Data::StockKey &stockKey, Data::Range &&range)
{
    key = stockKey;

    requestedRange.setRange(QDateTime::fromTime_t(0), 0);
    requiredRange = std::move(range);

    requestCandles();
}

}
