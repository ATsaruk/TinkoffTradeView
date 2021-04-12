QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# C++ 17
QMAKE_CXXFLAGS += -std=c++17

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
    Core/global.cpp \
    Core/config.cpp \
    Core/loggerlist.cpp \
    Data/Stock/candle.cpp \
    Data/Stock/stockkey.cpp \
    Data/Stock/stocks.cpp \
    Data/daterange.cpp \
    DataBase/Query/stocksquery.cpp \
    DataBase/idatabase.cpp \
    DataBase/postgresql.cpp \
    Plotter/Axis/axis.cpp \
    Plotter/Axis/horizontaldateaxis.cpp \
    Plotter/Groups/Candles/candleitem.cpp \
    Plotter/Groups/Candles/stockitem.cpp \
    Plotter/Groups/chartgroup.cpp \
    Plotter/chartplotter.cpp \
    Tasks/BrokerTasks/loadstockfrombroker.cpp \
    Tasks/Commands/loadstock.cpp \
    Tasks/DataBaseTasks/loadstockfromdb.cpp \
    Tasks/customcommand.cpp \
    Tasks/ibasetask.cpp \
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
    Core/global.h \
    Core/loggerlist.h \
    Data/Stock/candle.h \
    Data/Stock/stockkey.h \
    Data/Stock/stocks.h \
    Data/daterange.h \
    DataBase/Query/stocksquery.h \
    DataBase/idatabase.h \
    DataBase/postgresql.h \
    Plotter/Axis/axis.h \
    Plotter/Axis/horizontaldateaxis.h \
    Plotter/Groups/Candles/candleitem.h \
    Plotter/Groups/Candles/stockitem.h \
    Plotter/Groups/chartgroup.h \
    Plotter/chartplotter.h \
    Tasks/BrokerTasks/loadstockfrombroker.h \
    Tasks/Commands/loadstock.h \
    Tasks/DataBaseTasks/loadstockfromdb.h \
    Tasks/customcommand.h \
    Tasks/ibasetask.h \
    Tasks/manager.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += C:/Program Files/PostgreSQL/10/include
#LIBS+= C:/Program Files/PostgreSQL/10/lib/libpq.dll

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#uncomment to disable std::assert warnings!
DEFINES += NDEBUG

RESOURCES += \
  source.qrc

DISTFILES += \
  datamodel.qmodel \
  logsdiagram.qmodel \
  plotterdiagram.qmodel \
  tasksdiagram.qmodel

STATECHARTS +=