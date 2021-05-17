///@todo проверить загрузку пустой акции и загрузку backward!

#include <QThread>

#include "loadstock.h"
#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfromdbfunc.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

LoadStock::LoadStock(const Data::StockKey &stockKey, const uint minCandleCount_)
    : IBaseCommand("LoadStock")
{
    if (stockKey.interval() == Data::StockKey::INTERVAL::ANY)
        throw std::logic_error("LoadStock(): can't load stock with ANY interval!");

    stock->key = stockKey;
    loadedCandles->key = stockKey;
    minCandleCount = minCandleCount_;
}

void LoadStock::setData(SharedInterface &inputData)
{
    range = inputData;
}

SharedInterface &LoadStock::getResult()
{
    return *stock;
}

void LoadStock::exec()
{
    assert(range->isValid() && "LoadStock::exec(): Invalid range!");

    auto *loadFromDb = execFunc<LoadStockFromDbFunc>(*range, stock->key, minCandleCount);
    stock = loadFromDb->getResult();

    if (stock->candles.empty()) {
        startLoading();
        return;
    }

    Glo.stocks->insertCandles(stock);

    //Определяем направление загрузки и вообще её необходимость!
    Data::Range existedRange = Glo.stocks->getRange(stock->key);
    bool isLeftBorder = existedRange.getBegin() <= range->getBegin();

    QDateTime maxDateTime = std::max_element(stock->candles.begin(), stock->candles.end())->dateTime;
    bool isRightBorder = existedRange.getEnd() <= maxDateTime.addSecs(*stock->key.time());

    if (isRightBorder) {
        forwardLoading = true;
        loadForwardFromBroker();
    } else if (isLeftBorder)
        loadBackwardFromBroker();
    else
        emit finished();
}

void LoadStock::loadForwardFromBroker()
{
    QDateTime last = std::max_element(stock->candles.begin(), stock->candles.end())->dateTime;
    range->setBegin(last.addSecs(*stock->key.time()));

    if (range->toSec() >= stock->key.time())
        startLoading();
    else
        emit finished();
}

void LoadStock::loadBackwardFromBroker()
{
    QDateTime first = std::min_element(stock->candles.begin(), stock->candles.end())->dateTime;
    range->setEnd(first);

    if (range->toSec() >= stock->key.time())
        startLoading();
    else
        emit finished();
}

void LoadStock::startLoading()
{
    //Закрузка недостающих данных от брокера
    auto *task = createTask<LoadStockFromBroker>(stock->key);
    task->setData(*range);

    runNextTask();
}

void LoadStock::finishLoading()
{
    if (!loadedCandles->candles.empty()) {
        Data::Stock newCandles = Glo.stocks->insertCandles(loadedCandles);
        DB::StocksQuery::insertCandles(newCandles);
        stock->appendCandles(loadedCandles->candles);
    }
    emit finished();
}

void LoadStock::receiveResult(QObject *sender)
{
    auto *task = dynamic_cast<LoadStockFromBroker*>(sender);

    assert(task != nullptr && QString("%1;taskFinished();can't get task!;tasksLeft: %2")
            .arg(getName()).arg(taskList.size()).toStdString().data());

    InterfaceWrapper<Data::Stock> brokerCandles = task->getResult();
    loadedCandles->appendStock(brokerCandles);

    logDebug << QString("%1;taskFinished();finished: %2;loaded;%3;candles")
                .arg(getName(), task->getName()).arg(brokerCandles->candles.size());

    task->deleteLater();
}

bool LoadStock::isLoadFinished()
{
    int remainsCount = minCandleCount - stock->candles.size() - loadedCandles->candles.size();
    if (remainsCount <= 0 || endLoadDate.secsTo(range->getBegin()) < stock->key.time())  {
        finishLoading();
        return true;
    }
    return false;
}

//сделать
//taskFinished()

void LoadStock::taskFinished()
{
    receiveResult(sender());

    if (isLoadFinished())
        return;

    assert(forwardLoading == false && "LoadStock::taskFinished(): forwardLoading can't move here!");

    //Сдвигаем интервал загрузки
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(stock->key.interval());
    range->setRange(range->getBegin().addSecs(*stock->key.time() * -1), -maxLoadRange);

    startLoading();
}

}
