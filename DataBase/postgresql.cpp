#include <QSqlError>

#include "postgresql.h"
#include "Core/globals.h"

namespace DB {

PostgreSql::PostgreSql()
{
    db = QSqlDatabase::addDatabase( Glo.conf->getValue("DataBase/type", QString("QPSQL")) );
    db.setHostName     ( Glo.conf->getValue("DataBase/ip",       QString("127.0.0.1")) );
    db.setDatabaseName ( Glo.conf->getValue("DataBase/dbName",   QString("Tinkoff"))   );
    db.setUserName     ( Glo.conf->getValue("DataBase/userName", QString("postgres"))  );
    db.setPassword     ( Glo.conf->getValue("DataBase/password", QString("password"))  );

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
