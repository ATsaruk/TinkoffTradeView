/** @defgroup DataBase
  * @brief Модуль базы данных
  * @author Царюк А.В.
  * @date Сентябрь 2021 года */

#ifndef IDATABASE_H
#define IDATABASE_H

#include <QMutex>
#include <QSqlDatabase>

namespace DB {


/** @ingroup DataBase
  * @brief Базовый класс для работы с базой данных */
class IDataBase
{
public:
    QMutex mutex;

    explicit IDataBase() {}
    virtual ~IDataBase() {}

    ///Получение QSqlDatabase
    virtual QSqlDatabase& get() = 0;

    ///Состояние базы данных, true - база данных открыта
    virtual bool isOpen() const = 0;
};

}

#endif // IDATABASE_H
