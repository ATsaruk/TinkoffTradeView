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
    sandMode = Glo.conf->getValue("Tinkoff/sandMode", QVariant(false)).toBool();
    initMode();

    webSocketBaseUri = Glo.conf->getValue("Tinkoff/webSocketBaseUri", QVariant(defaultWebSocetUri)).toString();
}

Request::~Request()
{

}

bool Request::sendGet(const QString &path)
{
    if (authToken.isEmpty())
        return false;

    QNetworkRequest request(baseUri + path);

    //Добавляем данные в заголовок для авторизации
    QByteArray authInfo("Bearer ");
    authInfo.append(authToken);
    request.setRawHeader("Authorization", authInfo);
    request.setRawHeader("Content-Type", "application/json");

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager;
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &Request::onResponse);

    QNetworkReply *reply = networkAccessManager->get(request);
    connect(reply, &QNetworkReply::errorOccurred, this, &Request::onError);

    return true;
}

bool Request::sendPost(const QString &path, const QByteArray &data)
{
    if (authToken.isEmpty())
        return false;;

    QNetworkRequest request(baseUri + path);

    //Добавляем данные в заголовок для авторизации
    QByteArray authInfo("Bearer ");
    authInfo.append(authToken);
    request.setRawHeader("Authorization", authInfo);
    request.setRawHeader("Content-Type", "application/json");

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager;
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &Request::onResponse);

    QNetworkReply *reply = networkAccessManager->post(request, data);
    connect(reply, &QNetworkReply::errorOccurred, this, &Request::onError);

    return true;
}

bool Request::isTokenAvailable() const
{
    return !authToken.isEmpty();
}

bool Request::isSandMode() const
{
    return sandMode;
}

void Request::setSandMode(const bool sand)
{
    if (sandMode == sand)
        return;
    sandMode = sand;

    initMode();
    emit sandModeChanged(sandMode);
}

void Request::initMode()
{
    if (isSandMode()) {
        baseUri = Glo.conf->getValue("Tinkoff/sandboxBaseUri", QVariant(defaultSandBaseUri)).toString();
        tokenFile = Glo.conf->getValue("Tinkoff/fileSandKey", QVariant(defaultSandFileToken)).toString();
    } else {
        baseUri = Glo.conf->getValue("Tinkoff/baseURi", QVariant(defaultWorkBaseUri)).toString();
        tokenFile = Glo.conf->getValue("Tinkoff/fileKey", QVariant(defaultWorkFileToken)).toString();
    }

    authToken.clear();

    QFile file(tokenFile);
    if (file.open(QIODevice::ReadOnly)) {
        authToken = file.readAll();
        file.close();
        logInfo << "BrokerQuery;setMode();Successful loading broker key";
    } else
        logWarning << "BrokerQuery;setMode();Can't finder broker key!";
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
void Request::onError(QNetworkReply::NetworkError code)
{
    logWarning << QString("BrokerQuery;onError;Error=%1").arg(code);
}

}
