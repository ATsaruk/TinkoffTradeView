#include <QSqlError>

#include "postgresql.h"
#include "Core/globals.h"

namespace DB {

PostgreSql::PostgreSql()
{
    db = QSqlDatabase::addDatabase( Glo.conf->getValue("DataBase/type", QVariant("QPSQL")).toString() );

    db.setHostName     ( Glo.conf->getValue("DataBase/ip",       QVariant("127.0.0.1")).toString() );
    db.setDatabaseName ( Glo.conf->getValue("DataBase/dbName",   QVariant("Tinkoff")).toString() );
    db.setUserName     ( Glo.conf->getValue("DataBase/userName", QVariant("postgres")).toString() );
    db.setPassword     ( Glo.conf->getValue("DataBase/password", QVariant("inputPasswordHere")).toString() );

    if (!db.open())
        logWarning << QString("DataBase;DataBase();Can't open DataBase, check config file; error text=%1").arg(db.lastError().text());
    else
        logInfo << "DataBase;DataBase();DB connected!";
}

QSqlDatabase &PostgreSql::get()
{
    return db;
}

bool PostgreSql::isOpen() const
{
    return db.isOpen();
}

}
