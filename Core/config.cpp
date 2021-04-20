#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

#include "config.h"

namespace Core {


Config::Config(const QString &fileName)
{
    load(fileName);
}

Config::~Config()
{
    save();
}

QVariant Config::getValue(const QString &key, const QVariant &defaultValue)
{
    QReadLocker locker(&lock);

    if (settingsMap.find(key) == settingsMap.end())
        settingsMap[key] = defaultValue;

    return settingsMap[key];
}

void Config::setValue(QString &&key, QVariant &&value)
{
    QWriteLocker locker(&lock);

    //Удаляем возможные '/' в конце ключа, он там не нужен и будет мешать!
    while (key.lastIndexOf('/') == key.length()-1 && !key.isEmpty())
        key.chop(1);

    settingsMap[key] = std::move(value);
}

void Config::load(const QString &fileName)
{
    QWriteLocker locker(&lock);

    _fileName = fileName;

    QFile file(_fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    fromJSON(doc.object());
}

void Config::save(QString fileName)
{
    QReadLocker locker(&lock);

    if (fileName.isNull())
        fileName = _fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QJsonObject settingsObj;
    for (const auto &parametr : settingsMap)
        toJSON(parametr.first, parametr.second, settingsObj);

    QJsonDocument settingsDoc( std::move(settingsObj) );
    file.write( settingsDoc.toJson() );

    file.close();
}

/* Рекурсивная функция, которая добавляет данные в QJsonObject.
 * Функция добавляет в QJsonObject параметр состоящий из ключа key и значения value, где
 * [IN] key - это ключ параметра, имеед вид: key1/key2/../keyN
 * [IN] value - это значение параметра
 * [OUT] obj - QJsonObject в который будет производится добавление
 * Пример:
 * цвет ручки = синий: insertToJSON("pen/color", QVariant(QColor::blue))
 * толщина ручки = 2 : insertToJSON("pen/width", QVariant(2))
 * а кисть тонкая    : insertToJSON("brush",     QVariant("thin"))
 * В итоге будет сформирован JSON документ вида:
 * {
 *     "brush": "thin",
 *     "pen": {
 *         "color": "blue",
 *         "width": 2
 *     }
 * }
 *
 * Алгоритм работы:
 * п.1 Если условие index == -1 выполнено, это значит что мы имеет key вида: keyN!
 * значит мы в конечном узле QJsonObject'а и мы просто добавляем значение с ключем key и значением value:
 *   obj.insert(key, QJsonValue::fromVariant(value));
 *
 * п.2 Если символ '/' все же есть (а в конце его быть не может, об этом заботится функция setValue)
 * то из key выделяем первый ключ key1 и записываем его в curKey, все остальное записываем в subPath = key2/key3/../keyN
 * п.3 Далее: QJsonObject newObj = obj[curKey].toObject() - создаем newObj что бы в него добавить subPath + value :
 *      - если в объекте obj ключ curKey существует, то newObj будет проинициализирован этим объектом,
 *      - если в объекте obj ключа curKey не было, то он будет создан как пустой объет и newObj будет пустым.
 * п.6 Производим рекурсивный вызов toJSON с более коротким subPath, исходным value и результат помещаем в newObj
 * п.7 Перемещаем newObj в наш исходный объект с ключем curKey: obj[curKey] = std::move(newObj);
 *
 * тем самым мы будем укорачивать key, до тех пока он не перестанет содержать симовал '/'
 * как только это произойдет, на очередном рекурсивном вызове см. п.1 (выход из рекурсии) */
void Config::toJSON(const QString &key, const QVariant &value, QJsonObject &obj) const
{
    int index = key.indexOf("/");
    if (index == -1) {
        obj.insert(key, QJsonValue::fromVariant(value));
    } else { //key содержит subPath
        QString curKey = key.left(index);
        QString subPath = key.mid(index + 1);

        QJsonObject newObj = obj[curKey].toObject();
        toJSON(subPath, value, newObj);
        obj[curKey] = std::move(newObj);
    }
}

/* Младшая сестра предыдущей функции
 * [IN] obj - QJsonObject содержащий настройки
 * [IN] subPath - путь к текущему obj, при начальном запуске оставляется пустым!
 * Функция выгружает настройки из JSON формата в settingsMap, но в этот раз все проще, чем в предыдущей функции
 * В цикле for перебираем все поля QJsonObject'а:
 * 1. Если в значении поля находится другой QJsonObject, то делаем рекурсивный вызов для подобъекта при этом
 * текущее значения ключа передаем в переменной subPath не забывая разделять ключи симвовлом '/'
 * 2. Если очередное поля не является друим QJsonObject'ом, то считываем значение параметра и сохраняем в settingsMap
 * Возьмем пример выше:
 * {
 *     "brush": "thin",
 *     "pen": {
 *         "color": "blue",
 *         "width": 2
 *     }
 * }
 *
 * Первая запись "brush": "thin", т.к. "thin" это не QJsonObject, то просто будет сохранена настройка в
 * settingsMap[subPath + it.key()] = it.value().toVariant(),
 * где subPath на верхнем уровне пустой (default subPath = QString()), а it.key() = "brush",
 * it.value().toVariant() это "thin" в формате QVariant, итого: settingsMap["brush"] = QVariant("thin");
 *
 * Следующая запись с ключем "pen" является QJsonObject'ом,
 * QString newPath = subPath + it.key() + QString('/') : newPath = "pen" + '/' = "pen/"
 * Далее следует рекурсивный вызов fromJSON, но в качестве объекта передается объект внутри "pen",
 * в качестве path передается newPath, получаем obj:
 * {
 *     "color": "blue",
 *     "width": 2
 * }
 *
 * в цикле for перебираем поля, поле "color" не евляется QJsonObject'ом, поэтому сохраняется в settingsMap
 * settingsMap[path + it.key()] = it.value().toVariant(),
 * path + it.key() = "pen/" + "color" = "pen/color", значение "blue" : settingsMap["pen/color"] = QVariant("blue");
 * Если бы это был тоже QJsonObject, функция спустилась бы ниже, и так будут пройдены все поля obj
 * так же будет со следующей записью, она не является QJsonObject'ом, и будет сохранена в settingsMap
 * path + it.key() = "pen/" + "width" = "pen/width", значение 2 : settingsMap["pen/width"] = QVariant(2); */
void Config::fromJSON(const QJsonObject &obj, const QString &subPath /* = QString() */ )
{
    for (auto it = obj.begin(); it < obj.end(); it++) {
        if (it.value().isObject()) {
            QString newPath = subPath + it.key() + QString('/');
            fromJSON(it.value().toObject(), newPath);
        } else {
            settingsMap[subPath + it.key()] = it.value().toVariant();
        }
    }
}

}
