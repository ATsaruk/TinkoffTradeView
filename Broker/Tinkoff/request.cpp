#include <QFile>

#include "request.h"
#include "Core/globals.h"

namespace Broker {

constexpr auto defaultSandFileToken = "path/sand.key";
constexpr auto defaultWorkFileToken = "path/main.key";
constexpr auto defaultSandBaseUri = "https://api-invest.tinkoff.ru/openapi/sandbox";
constexpr auto defaultWorkBaseUri = "https://api-invest.tinkoff.ru/openapi";
constexpr auto defaultWebSocetUri = "wss://api-invest.tinkoff.ru/openapi/md/v1/md-openapi/ws";

Request::Request()
{
    _sandMode = Glo.conf->getValue("Tinkoff/sandMode", false);
    initMode();

    _webSocketBaseUri = Glo.conf->getValue("Tinkoff/webSocketBaseUri", QString(defaultWebSocetUri));
}

Request::~Request()
{

}

bool Request::sendGet(const QString &path)
{
    return send(path, QByteArray());
}

bool Request::sendPost(const QString &path, const QByteArray &data)
{
    return send(path, data);
}

bool Request::isSandMode() const
{
    return _sandMode;
}

void Request::setSandMode(const bool sand)
{
    if (_sandMode == sand)
        return;

    _sandMode = sand;

    initMode();
    emit sandModeChanged(_sandMode);
}

void Request::initMode()
{
    QString tokenFile;          //Имя файла с токеном авторизации
    if (isSandMode()) {
        _baseUri = Glo.conf->getValue("Tinkoff/sandboxBaseUri", QString(defaultSandBaseUri));
        tokenFile = Glo.conf->getValue("Tinkoff/fileSandKey", QString(defaultSandFileToken));
    } else {
        _baseUri = Glo.conf->getValue("Tinkoff/baseURi", QString(defaultWorkBaseUri));
        tokenFile = Glo.conf->getValue("Tinkoff/fileKey", QString(defaultWorkFileToken));
    }

    _authToken.clear();

    QFile file(tokenFile);
    if (file.open(QIODevice::ReadOnly)) {
        _authToken = file.readAll();
        file.close();
        logInfo << "BrokerQuery;setMode();Successful loading broker key";
    } else
        logWarning << "BrokerQuery;setMode();Can't finder broker key!";
}

bool Request::send(const QString &path, const QByteArray &data)
{
    if (_authToken.isEmpty())
        return false;;

    QNetworkRequest request(_baseUri + path);

    //Добавляем данные в заголовок для авторизации
    QByteArray authInfo("Bearer ");
    authInfo.append(_authToken);
    request.setRawHeader("Authorization", authInfo);
    request.setRawHeader("Content-Type", "application/json");

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager;
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &Request::onResponse);

    QNetworkReply *reply = data.isEmpty() ?
                networkAccessManager->get(request) :        //Если data is empty отправляем GET запрос
                networkAccessManager->post(request, data);  //Иначе отправляем POST запрос

    connect(reply, &QNetworkReply::errorOccurred, this, &Request::_onError);

    return true;
}

// Получает ответ от сервера и отправляет об этом сигнал с полученными данными
void Request::onResponse(QNetworkReply* resp)
{
    QByteArray answer = resp->readAll();
    emit getResopnse(answer);

    //Удаляем QNetworkAccessManager, который выполнил свою функцию
    QNetworkAccessManager *manager = static_cast<QNetworkAccessManager*>(sender());
    manager->deleteLater();
}

// Обрабатывает ошибоки от сервера
void Request::_onError(QNetworkReply::NetworkError code)
{
    logWarning << QString("BrokerQuery;onError;Error=%1").arg(code);
}

}
