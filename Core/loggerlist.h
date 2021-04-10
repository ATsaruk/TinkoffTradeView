/* Пара примеров вызова:
 *   *loggerList->get("warning") << "и тааак сойдет!";
 *   logWarning << "но вроде так выглядит лучше!";
 * Если же warning логгер предварительно не был добавлен, при вызове get будет создан FileLogger с тэгом warning.
 * Далее некоторые особенности вызова функции get(), если перед обращением к логгеру warning (или любому другому) предварительно явно
 * не указать логгер, например:
 *   loggerList->add<MsgBoxLogger> (warning);
 * Ещё пример, если хотим явно указать создаваемый по умолчанию логгер, так лучше не делать (лучше явно добавлять нужные нам логи), но все же:
 *   *loggerList->get<MsgBoxLogger>("warning") << "Если логгер с тэгом warning не создан заранее, это сообщение выведется на экран!";
 *   logWarning << "Если этот вызов будет произведен после предыдущего, то сообщение будет выведено туда же";
 * <MsgBoxLogger> будет использоваться только в случае, если логгера warning на данный момент не существует.
 * Например мы хотим писать warning в FileLogger и в MsgBoxLogger с разным содержимым:
 *   *loggerList->get<MsgBoxLogger>("warning") << "Это я запишу я MsgBoxLogger!"
 *   *loggerList->get<FileLogger>("warning") << "А это я запишу в FileLogger!"
 * Это так не работает! При вызове первой функции будет добавлен MsgBoxLogger (если конечно он раньше не был инициализирован каким то другим логгером)
 * и в этот MsgBoxLogger будет записано первое сообщение, во второй строчке FileLogger создан не будет и вторая запись произойдет так же в MsgBoxLogger,
 * Правильно это вот так:
 *   *loggerList->get<MsgBoxLogger>("warningMsgBox") << "Это я запишу я MsgBoxLogger!"
 *   *loggerList->get<FileLogger>("warning") << "А это я запишу в FileLogger!"
 * У них просто должны быть разные тэги.
 * Если нам нужно писать одни и те же сообщения в оба лога, такие примеры приведены в заголовочном файле imultilogger.h */

#ifndef LOGGERLIST_H
#define LOGGERLIST_H

#include <QVariant>

#include "Core/Logs/imultilogger.h"
#include "Core/Logs/filelogger.h"

namespace Core {

#define defaultLogger FileLogger

/** @ingroup Core
  * @brief Класс хранит в себе все наши логгеры
  * @details Работает только с наследниками IMultiLogger'а, ILogger'ы не признает, зачем они нужны? Когда есть IMultiLogger?! */
class LoggerList
{
public:
    LoggerList();
    ~LoggerList();


    /** @brief Возвращает логгер с тэгом tag
      * @param[IN] tag - тэг лог (debug, info, warning, ... , superPuperLog )
      * @return Указатель на ILogger, да именно ILogger, а не IMultiLogger, "клиент" будет работать с
      * интерфейсом ILogger, метод appendLogger ему не нужн.
      * @details Если логгер не был создан до этого, то создает defaultLogger (#define defaultLogger FileLogger) */
    ILogger *get(QString tag);

    /** @brief Создает логгер заданного типа с заданным тэгом
      * @param[IN] tag - тэг лог (debug, info, warning, ... )
      * @param[IN] T - класс добавляемого лога, если ничего не указать, то добавит defaultLogger
      * @details Если логгера с указанным тэгом не существует, то он создается типа T, если же логгер
      * с указанным тэгом уже существует, то к нему будет добавлено новый логгер с типом T.
      * @see Core::Logs::IMultiLogger */
    template<class T = defaultLogger>
    typename std::enable_if_t<std::is_base_of_v<IMultiLogger, T>>
    add(const QString &tag)
    {
        if (logs.find(tag) == logs.end())
            logs[tag] = new T(tag);
        else
            logs[tag]->appendLogger<T>();
    }

private:
    std::unordered_map<QString, IMultiLogger*> logs;

    /** @brief Ищет логгер с тэгом tag, если не находит создает его с типом T
      * @param[IN] tag - тэг нашего лога (debug, info, warning, ... , superPuperLog )
      * @param[IN] T - класс логгера по умолчанию
      * @return Указатель на ILogger с запрашиваемым тэгом.
      * @details Ищет логгер с указанным тэгом, если не находит то создает его типа T, и возвращает на него ссылку
      * @code get("warning")->message("text") @endcode
      * Если warning logger не существует, он будет создан как FileLogger (#define defaultLogger FileLogger) */
    template<class T>
    typename std::enable_if_t<std::is_base_of_v<IMultiLogger, T>, ILogger*>
    get(const QString &tag)
    {
        if (logs.find(tag) == logs.end())
            add<T>(tag);

        return dynamic_cast <ILogger*> (logs.find(tag)->second);
    }

};

}

#endif // LOGGERLIST_H
