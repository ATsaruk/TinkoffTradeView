#ifndef REQUEST_H
#define REQUEST_H

#include <QNetworkReply>

namespace Broker {


/** @ingroup Tinkoff
  * @brief Класс для отправки GET/POST запросов на сервер брокера Тинькофф
  * @todo Реализовать режим работы в песочнице (https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/sandbox)
  * @details Класс формирует GET/POST запросы, отправляет их на сервер брокера, при получении ответа формирует сигнал getResopnse(QByteArray)
  * Подробнее о запросах и ответах: https://tinkoffcreditsystems.github.io/invest-openapi/swagger-ui/#/ */
class Request : public QObject
{
public:
    Request();
    ~Request();


    /** @brief Отправляет GET запрос
      * @param[IN] path - путь, который добавляется в заголовок запроса
      * @warning Если отсутсвует токен авторизации, то получим исключение! */
    bool sendGet(const QString &path);


    /** @brief Отправляет POST запрос
      * @param[IN] path - путь, который добавляется в заголовок запроса
      * @param[IN] data - данные, которые будут отправлены в теле запроса
      * @warning Если отсутсвует токен авторизации, то получим исключение! */
    bool sendPost(const QString &path, const QByteArray &data);


    /// Возвращает TRUE - если токен авторизации найдет, FALSE - если токен авторизации отсутствует
    bool isTokenAvailable() const;

    /// Возвращает TRUE - если включем режим "в песочнице"
    bool isSandMode() const;

    /// Управление режимом работы (TRUE - режим "в песочнице"
    void setSandMode(const bool sand);

Q_SIGNALS:
    /// Сигнал с ответом от брокера на POST/GET запрос
    void getResopnse(QByteArray);

    /// Сигнал о смене режима работы (TRUE - режим песочницы, FALSE - нормальный режим)
    void sandModeChanged(bool);

protected:
    /// Загружает токен авторизации и читает из настроек адресс сервера брокера для работы в соответсвующем режиме.
    void initMode();

protected slots:
    /// Обрабатывает ответы от брокера и генерирует сигнал с полученным ответом
    void onResponse(QNetworkReply*);

private:
    Q_OBJECT

    //Текущий режим работы
    //TRUE - режим работы в песочнице
    //FALSE - режим работы с основным сервером брокера
    bool sandMode;

    QString tokenFile;          //Имя файла с токеном авторизации
    QString baseUri;            //Адрес сервера для работы с биржей
    QString webSocketBaseUri;   //Адрес сервера для подписки на свечи
    QByteArray authToken;       //Токен для авторизации

private slots:
    //Обрабатывает сетевые ошибоки при работе с брокером
    void onError(QNetworkReply::NetworkError code);
};


}

#endif // REQUEST_H
