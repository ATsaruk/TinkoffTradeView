#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "idatabase.h"

namespace DB {


class PostgreSql : public IDataBase
{
public:
    explicit PostgreSql();

    //Получение QSqlDatabase
    QSqlDatabase& get() override;

    //Состояние базы данных, true - база данных открыта
    bool isOpen() const override;
private:
    //База данных
    QSqlDatabase db;
};

}

#endif // POSTGRESQL_H
