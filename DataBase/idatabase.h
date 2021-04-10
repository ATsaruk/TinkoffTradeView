#ifndef IDATABASE_H
#define IDATABASE_H

#include <QMutex>
#include <QSqlDatabase>

namespace DB {


///Класс для работы с базой данных (чтение/запись данных)
//Перед записю/чтением данных в/из БД нужно получить разрешние на доступ, а по завершению доступ освободить
class IDataBase
{
public:
    //Mutex для управления доступом к данным
    QMutex mutex;

    IDataBase();
    virtual ~IDataBase();

    //Получение QSqlDatabase
    virtual QSqlDatabase& get() = 0;

    //Состояние базы данных, true - база данных открыта
    virtual bool isOpen() const = 0;
};

}

#endif // IDATABASE_H
