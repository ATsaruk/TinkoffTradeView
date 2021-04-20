#ifndef DBCANDLESLOADER_H
#define DBCANDLESLOADER_H

#include "candlesloader.h"

namespace Plotter {


class DbCandlesLoader : public CandlesLoader
{
public:
    explicit DbCandlesLoader();

protected:
    void requestCandles() override;

protected slots:
    void taskFinished();
};

}

#endif // DBCANDLESLOADER_H
