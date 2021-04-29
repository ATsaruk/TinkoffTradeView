#ifndef CONFIG_H
#define CONFIG_H

#include <QVariant>
#include <QReadWriteLock>

namespace Core {


/** @ingroup Core
  * @brief Класс настроек приложения
  * @details Настройки хранятся в файле в формате json. При завершении программы, настройки сохраняются в указанный файл.\n
  * @see getValue(), setValue(); */
class Config
{
public:
    /// Загружает настройки из файла
    explicit Config(const QString &fileName = QString("config.cfg"));
    /// Сохраняются настройки в файл
    ~Config();

    /** @brief Возвращает значение параметра
      * @param[IN] key ключ параметра
      * @param[IN] defaultValue значение параметра по умолчания
      * @return значение параметра с ключем key, если такого ключа в списке нет, то вернет переданное defaultValue
      * @return Возвращает значение параметра с ключем key, если такого параметра не найдено, то вернет переданное defaultValue,
      * а так же создаст параметр с ключем key и инициализирует его значением defaultValue. */
    template<typename T>
    std::enable_if_t<std::is_constructible_v<QVariant, T>, T>
    getValue(const QString &key, T &&defaultValue) {
        QReadLocker locker(&lock);

        if (settingsMap.count(key) == 0)
            settingsMap[key] = std::move(defaultValue);

        return settingsMap[key].value<T>();
    }

    /** @brief Сохраняет значение параметра
      * @param[IN] key ключ параметра
      * @param[IN] value значение параметра (будет перемещено)
      * @details Если параметр с ключем key существует, его значение будет заменено.\n
      * Если параметра с ключем key не существует, то он будет создан и проинициализирован значением value.\n
      * Значение value будет перемещено: @code settingsMap[key] = std::move(value); @endcode */
    void setValue(QString &&key, QVariant &&value);


    /** @brief Загружает настройки приложения из файла
      * @param[IN] fileName имя файла с настройками */
    void load(const QString &fileName);


    /// Сохранение настройки в файл указанный при загрузке настроек
    void save(QString fileName = QString());

private:
    QReadWriteLock lock;
    //Имя файла с настройками
    QString _fileName;
    //Список настроек
    std::unordered_map<QString, QVariant> settingsMap;

    //Добавляет параметр с ключем key и значением value в QJsonObject
    void toJSON(const QString &key, const QVariant &value, QJsonObject &obj) const;
    //Читает значения key + value из QJsonObject и добавляет их в settingsMap
    void fromJSON(const QJsonObject &obj, const QString &subPath = QString());
};

}

#endif // CONFIG_H
