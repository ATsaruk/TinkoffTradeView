/* Паттерн компановщик, это класс Component, а класс Composite это CustomCommand
 * Тут есть одна заморочка с потоками, по задумке каждая задача должна выполнятся в отдельном потоке, но задача
 * может состоять из нескольких других задач, а те в свою очереь так же из подзадач и появляются проблемы с удалением
 * вложенных потоков...
 * По поводу используемых понятий:
 * Задача - это наследник от IBaseTask, задача выполняет некое 1 действие
 * Комманда - это наследник или эксземпляр CustomCommand, комманда состоит из задач и/или других комманд,
 *   но т.к. CustomCommand в свою очередь так же является наследником от IBaseTask, то далее говоря "задача":
 *   речь может идти как о задаче (IBaseTask), так и о комманде (CustomCommand).
 *
 * Введем понятие:
 *   Корневая задача (RootTask) - это задача(комманда), которая будет передана на исполнение TaskManager'у
 * но в другом случае эта же задача может использоватся как часть какой то другой комманды
 * Корневой может быть как задача так и комманда (CustomCommand и её производные),
 * bool isRootTask - признак того что это корневая задача(комманда), этот признак присваивается в конструкторе
 * в зависимотси от значения родительского потока (QThread *parent):
 *   * если в конструктор передан ссылка на родительский поток, то:
 *     - ссылка на поток будет сохранена в taskThread = parent и текущий объект будет перенесон в поток parent
 *     - в функции start() будет вызван виртуальный метод exec(), который должен быть реализован в объектах наследниках, это
 *       основной метод, в котором и должны производиться действия, заложенные в задачу/комманду
 *
 *   * если в конструкторе передан nullptr - это означает что это корневая задача(комманда), и значит для данной задачи(комманды):
 *     - в конструкторе будет создан новый QThread (taskThread), и задача будет переперенесена в этот новый поток.
 *       И если это комманда, то задачи/комманды, которые будут добавлятся так же будут перенесены в поток taskThread, при создании
 *       новых задач/комманд для данной команды в конструктор будет передаваться taskThread.
 *     - в функции start() произведет запуск потока taskThread->start(),
 *
 * При наследовании от данного класса необходимо:
 *   1. создать коструктор, который принимает указатель на QThread:
 *     class MyTask : public IBaseTask
 *     {
 *     public:
 *       MyTask(QThread *parent = nullptr);
 *     }
 *
 *     MyTask::MyTask(QThread *parent)
 *            : IBaseTask(parent)
 *     {
 *     }
 *   при создании:
 *     auto *newTask = new MyTask;
 *   будет создана корневая задача, которую можно запустить через TaskManager:
 *     TaskManager::get()->registerTask(newTask);
 *
 *   а так же при добавлении задачи в CustomCommand, функцией addTask, в конструкторе ей будет передаваться указатель на поток,
 *   в котором она будет выполняться
 *
 *   2. переопределить функцию QString getName():
 *     QString MyTask::getName()
 *     {
 *       return "MyTask";
 *     }
 *   это нужно для отладки, имя задач используется для логов
 *
 *   3. Если нужны входные параметры, то определить функцию void setData(type1 var1, type2 var2)
 *   в неё будут переданы данные при добавлении задачи функцией addTask, например вызов будет выглядеть так:
 *     TaskManager::get()->addTask <MyTask> (var1, var2);
 *   или так:
 *     TaskManager::get()->addTask <MyTask, type1, type2> (var1, var2);
 *   первый вариант компактнее, а типы будут подставлены автоматически, количество аргументов функции и их тип может быть любым,
 *   если дополнительных данных передовать не требуется, то жужно объявить функцию setData без аргументов и с пустым телом
 *
 *   4. Переопределить функцию void exec(), при выполнении нужно следить за флагом isStopRequested, если флаг = true завершаем выполнени,
 *      функция exec() будет вызвана при запуске задачи:
 *     void MyTask::exec()
 *     {
 *       //выполнить необходимые действия
 *       for(auto &it : someContainer) {
 *         someAction(it);
 *         ...
 *   ->    if (isStopRequested) {
 *           emit finished();
 *           return;
 *         }
 *       }
 *
 *       saveSomeData();
 *
 *       ...
 *
 *       //по завершению задачи отправить сигнал о завершении
 *       emit finished();
 *     }
 *
 *   Задача считаться запущенной пока она не отправит сигнал finished().
 *   Задача не обязана выполнятся в методе exec() и быть завершена при выходе из метода exec(), задача может завершится в другой функции,
 *   которая например может быть вызвана по таймеру, или по какому либо другому сигналу или событию.
 */
#ifndef IBASETASK_H
#define IBASETASK_H

#include <QObject>

namespace Task {


///Базовый класс для задачи (паттерн компоновщик)
class IBaseTask : public QObject
{
public:
    virtual ~IBaseTask();

    //Возвращает имя задачи
    virtual QString getName() = 0;

    //Возвращает поток, в котором находится данный экземпляр класса
    virtual QThread* getThread();

public slots:
    //Основная функция, запускает задачу (taskThread->start())
    virtual void start() final;

    //Остановить задату
    virtual void stop();

    int isFinishedSignalHasConnection();

signals:
    //Сигнал о завершении работы
    void finished();

protected:
    //Запрос на остановку задачи
    bool isStopRequested = false;
    //Поток в котором будет выполняться данная задача
    QThread *taskThread;

    IBaseTask(QThread *parent);

    //Функция которая будет вызвана при запуске потока (QThread)
    virtual void exec() = 0;

private:
    Q_OBJECT

    //Признак того, что это корневая задача, она управляет выделением памяти, запуском, остановкой и удалением taskThread
    bool isRootTask;
};

}

#endif // IBASETASK_H
