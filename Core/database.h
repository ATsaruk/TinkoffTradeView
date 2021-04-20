#ifndef DATABASE_H
#define DATABASE_H

namespace Core {


class Database
{
public:
    static void connect();
    static void disconnect();

private:
    Database();
};

}

#endif // DATABASE_H
