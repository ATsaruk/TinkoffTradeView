QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 c++14 c++17 c++2a

#uncomment to disable std::assert warnings!
#DEFINES += NDEBUG

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Broker/Tinkoff/request.cpp \
    Broker/Tinkoff/tinkoff.cpp \
    Core/Logs/filelogger.cpp \
    Core/Logs/ilogger.cpp \
    Core/Logs/imultilogger.cpp \
    Core/Logs/msgboxlogger.cpp \
    Core/globals.cpp \
    Core/config.cpp \
    Core/loggerlist.cpp \
    Data/Stock/candle.cpp \
    Data/Stock/stockkey.cpp \
    Data/Stock/stocks.cpp \
    Data/range.cpp \
    DataBase/Query/stocksquery.cpp \
    DataBase/idatabase.cpp \
    DataBase/postgresql.cpp \
    Plotter/Axis/axis.cpp \
    Plotter/Axis/dateaxis.cpp \
    Plotter/Axis/numericaxis.cpp \
    Plotter/Axis/priceaxis.cpp \
    Plotter/Groups/Candles/candleitem.cpp \
    Plotter/Groups/Candles/candlesdata.cpp \
    Plotter/Groups/Candles/candlesseries.cpp \
    Plotter/Groups/chartseries.cpp \
    Plotter/chartplotter.cpp \
    Plotter/chartscene.cpp \
    Tasks/StockTasks/loadstockfrombroker.cpp \
    Tasks/StockTasks/loadstock.cpp \
    Tasks/StockTasks/loadstockfromdbfunc.cpp \
    Tasks/ibasecommand.cpp \
    Tasks/ibasetask.cpp \
    Tasks/ifunction.cpp \
    Tasks/manager.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Broker/Tinkoff/request.h \
    Broker/Tinkoff/tinkoff.h \
    Broker/api.h \
    Core/Logs/filelogger.h \
    Core/Logs/ilogger.h \
    Core/Logs/imultilogger.h \
    Core/Logs/msgboxlogger.h \
    Core/config.h \
    Core/globals.h \
    Core/loggerlist.h \
    Data/Stock/candle.h \
    Data/Stock/stockkey.h \
    Data/Stock/stocks.h \
    Data/range.h \
    DataBase/Query/stocksquery.h \
    DataBase/idatabase.h \
    DataBase/postgresql.h \
    Plotter/Axis/axis.h \
    Plotter/Axis/dateaxis.h \
    Plotter/Axis/numericaxis.h \
    Plotter/Axis/priceaxis.h \
    Plotter/Groups/Candles/candleitem.h \
    Plotter/Groups/Candles/candlesdata.h \
    Plotter/Groups/Candles/candlesseries.h \
    Plotter/Groups/chartseries.h \
    Plotter/chartplotter.h \
    Plotter/chartscene.h \
    Tasks/Interfaces/interfase.h \
    Tasks/StockTasks/loadstockfrombroker.h \
    Tasks/StockTasks/loadstock.h \
    Tasks/StockTasks/loadstockfromdbfunc.h \
    Tasks/ibasecommand.h \
    Tasks/ibasetask.h \
    Tasks/ifunction.h \
    Tasks/manager.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  source.qrc

DISTFILES += \
  Models/datamodel.qmodel \
  Models/logsdiagram.qmodel \
  Models/plotterdiagram.qmodel \
  Models/tasksdiagram.qmodel

STATECHARTS +=
