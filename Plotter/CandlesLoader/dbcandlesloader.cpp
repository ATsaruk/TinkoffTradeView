#include "dbcandlesloader.h"
#include "Core/global.h"
#include "Tasks/DataBaseTasks/loadstockfromdb.h"

namespace Plotter {


DbCandlesLoader::DbCandlesLoader()
{

}

void DbCandlesLoader::requestCandles()
{
    if (isRequestExecuted) {
        /* Предыдущий запрос данных ещё не обработ, запоминаем что пришел новый запрос,
         * что бы его обработать, когда будет завершен предыдущий запрос! */
        isNewRequestReceived = true;
        return;
    }

    Data::Range newRange = requiredRange;
    newRange.remove(requestedRange);

    auto task = NEW_TASK<Task::LoadStockFromDb>(key, newRange);
    connect(task, &Task::IBaseTask::finished, this, &DbCandlesLoader::taskFinished);

    isRequestExecuted = true;
    requestedRange = requiredRange;

    if (isNewRequestReceived)
        isNewRequestReceived = false;
}

void DbCandlesLoader::taskFinished()
{
    emit candlesRecieved();

    isRequestExecuted = false;
    if (isNewRequestReceived)
        requestCandles();
}

}
