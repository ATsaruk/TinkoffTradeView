#include <QSqlError>

#include "postgresql.h"
#include "Core/globals.h"

namespace DB {

PostgreSql::PostgreSql()
    : _db (QSqlDatabase::addDatabase( Glo.conf->getValue("DataBase/type", QString("QPSQL")) ))
{
    _db.setHostName     ( Glo.conf->getValue("DataBase/ip",       QString("127.0.0.1")) );
    _db.setDatabaseName ( Glo.conf->getValue("DataBase/dbName",   QString("Tinkoff"))   );
    _db.setUserName     ( Glo.conf->getValue("DataBase/userName", QString("postgres"))  );
    _db.setPassword     ( Glo.conf->getValue("DataBase/password", QString("password"))  );

    if (!_db.open())
        logWarning << QString("DataBase;DataBase();Can't open DataBase, check config file; error text=%1").arg(_db.lastError().text());
    else
        logInfo << "DataBase;DataBase();DB connected!";
}

QSqlDatabase &PostgreSql::get()
{
    return _db;
}

bool PostgreSql::isOpen() const
{
    return _db.isOpen();
}

}
