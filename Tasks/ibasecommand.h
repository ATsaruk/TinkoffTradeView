/* Перед знакомством с классом рекомендую ознакомится с IBaseTask.
 * Данный класс предназначен для выполнения последовательности задач(комманд)
 * Есть 2 варианта использования:
 *
 *-> Первый вриант использовать как базовый класс для набора комманд, это нужно когда мы постоянно выполняем одни
 *   и те же команды в одной и той же последовательности, например:
 *
 *   class CommandDoSomething : public BaseCommand
 *   {
 *     public:
 *       void setData(const Place &from, const Place &to) //must be created
 *
 *       //Возвращает имя класса владельца
 *       QString getName() override;
 *
 *     private:
 *       Data data; //variable for save temprory data
 *   }
 *
 *   void CommandDoSomething::setData(const Place &from, const Place &to)
 *   {
 *      addTask <TaskTakeSomething> (from, data);
 *      addTask <TaskCalcSomething> (data);
 *      addTask <TaskPutSomething>  (to, data);
 *      addTask <TaskSaveAll> ();
 *   }
 *
 *   QString CommandDoSomething::getName()
 *   {
 *     return "CommandDoSomething";
 *   }
 *
 *   void MainWindow::MainWindow()
 *   {
 *     auto *command = TaskManager::get()->addTask <CommandDoSomething> (placeSource, placeTarget);
 *     connect(command, &IBaseTask::finished, this, &MainWindows::commandFinished);
 *   }
 *
 * В данном примере:
 *   классы TaskTakeSomething, TaskCalcSomething, TaskPutSomething и TaskSaveAll - это классы наследники IBaseTask (или других BaseCommand классов)
 *   TaskTakeSomething - берет что то из "from", и помещает это в "data"
 *   TaskCalcSomething - производит какие то вычисления с "data"
 *   TaskPutSomething - помещает "data" в "to"
 *   TaskSaveAll - что то сохраняет
 *
 *   у класса TaskTakeSomething должен быть метод setData(const Place &from, Data &data);
 *   у класса TaskCalcSomething должен быть метод setData(Data &data);
 *   у класса TaskPutSomething должен быть метод setData(const Place &from, Data &data);
 *   у класса TaskSaveAll может не быть метода setData(), т.к. он объявлен в IBaseTask, как пустой метод;
 *   данные методы нужны для добавления в список задач
 *
 *-> Второй вариант создание команд без создания нового класса:
 *
 *   void MainWindow::createAndRegisterNewCommand()
 *   {
 *     auto *command = new CustomCommand("DoSomething");
 *     command->addTask <TaskTakeSomething> (from, data);
 *     command->addTask <TaskCalcSomething> (data);
 *     command->addTask <TaskPutSomething>  (to, data);
 *     command->addTask <TaskSaveAll> ();
 *     connect(command, &IBaseTask::finished, this, &MainWindows::commandFinished);
 *     TaskManager::get()->registerTask(command);
 *   }
 *
 * Результат будет такой же как и в первом случае, если нам комманда нужна однократно, то проще создать её как в примере 2
 * если задача будет использоваться в нескольких разных классах, логичнее создать новый класс CommandDoSomething
 *
 *-> Ещё пример создания команды пользователем для сложения/вычитания/умножения и деления чисел, последовательность действий будет определена пользователем:
 *
 *   class MainWindow
 *   {
 *   ...
 *   private:
 *     qreal result;
 *     CustomCommand *command;
 *   }
 *
 *   void MainWindow::on_pushButtorCreateNewCommand_clicked()
 *   {
 *     result = 0;
 *     QString commandName = QString("MyCommand");
 *     command = new CustomCommand(commandName);
 *   }
 *
 *   void MainWindow::on_pushButtonAddActionAddition_clicked()
 *   {
 *     qreal second = ui->lineEdit()->text()->toReal();
 *     command->addTask <TaskActionAddition> (result, second);
 *   }
 *
 *   void MainWindow::on_pushButtonAddActionSubtraction_clicked()
 *   {
 *     qreal second = ui->lineEdit()->text()->toReal();
 *     command->addTask <TaskActionSubtraction> (result, second);
 *   }
 *
 *   void MainWindow::on_pushButtonAddActionMultiplication_clicked()
 *   {
 *     qreal second = ui->lineEdit()->text()->toReal();
 *     command->addTask <TaskActionMultiplication> (result, second);
 *   }
 *
 *   void MainWindow::on_pushButtonAddActionDivision_clicked()
 *   {
 *     qreal second = ui->lineEdit()->text()->toReal();
 *     command->addTask <TaskActionDivision> (result, second);
 *   }
 *
 *   void MainWindow::on_pushButtorExecTask_clicked()
 *   {
 *     connect(command, &IBaseTask::finished, this, &MainWindows::slotCommandFinished);
 *     TaskManager::get()->registerTask(command);
 *   }
 *
 *   void MainWindow::slotCommandFinished()
 *   {
 *     ui->label1->setText(result);
 *   }
 *
 *   Для краткости опущены проверки что бы действия не добавлялись до создания комманды и другие само собой разумеющиеся проверки вроде деления на 0
 *   это просто попытка показать создания динамической комманды:
 *     * 1. пользователь нажимает кнопку создать команду
 *     * 2. далее пользователь вводит нужное значение в поле lineEdit и нажимает кнопку с необходимым действием, комманда будет зарегистрирована, но выполнена не будет
 *     * 3. после добавления желаемого колличества задач, нажимает кнопку ExecTask, к сигналу завершения комманды подключается слот, который отобразит результат
 *     *    и комманда запускается
 *
 *   в данном примере каждая задача производит соответсвующее действие с result и second, результат записывает в result
 *   Если бы это был класс задачи, в классе могло быть приватное поле result, и можно было бы передавать только параметр second, а результат получить с сигналом
 *
 *
 *   class CommandDoSomething : public BaseCommand
 *   {
 *   public:
 *     CommandDoSomething();
 *   ...
 *   signals:
 *     void returnResult(qreal);
 *
 *   protected slots:
 *     void onFinished();
 *
 *   private:
 *     qreal result;
 *   }
 *
 *   CommandDoSomething::CommandDoSomething()
 *   {
 *     connect(this, &IBaseTask::finished, this, &CommandDoSomething::onFinished);
 *   }
 *
 *   void CommandDoSomething::onFinished()
 *   {
 *     emit returnResult(result);
 *   }
 *
 ****************************************************************************************************************
 * Ну и напоследок конкретный пример, нам нужно загрузить свечные данные (3 варианта реализации) первый и второй
 * вариант вызывают готовую задачу CommandLoadStock, третий вариант создает её сам из более простых задач, после
 * выполнения задачи должен вызваться метод addCandles() для отображения загруженных свечей
 *
 * Первый вариант создание через NEW_TASK:
 *   auto *command = NEW_TASK<CommandLoadStock>(curStockKey, beginTime, endTime, minCandleCount);
 *   connect(command, &IBaseTask::finished, this, &ChartCandlesGroup::addCandles);
 *
 * Ещё тот же самый вариант без использования #define NEW_TASK:
 *   auto *command = TaskManager::get()->addTask<CommandLoadStock>(curStockKey, beginTime, endTime, minCandleCount);
 *   connect(command, &IBaseTask::finished, this, &ChartCandlesGroup::addCandles);
 *
 * Второй вариант создание комманды "ручками":
 *   auto *command = new CommandLoadStock;
 *   command->setData(curStockKey, beginTime, endTime, minCandleCount);
 *   connect(command, &CommandLoadStock::finished, this, &ChartCandlesGroup::addCandles);
 *   TaskManager::get()->registerTask(command);
 *
 * Третий вариант, создание комманды через CustomCommand:
 *   auto *command = new CustomCommand("LoadCandles");
 *   command->addTask<TaskLoadStockFromDb>(curStockKey, beginTime, endTime, minCandleCount);
 *   command->addTask<TaskLoadStocksFromBroker>(curStockKey, beginTime, endTime, minCandleCount);
 *   connect(command, &CommandLoadStock::finished, this, &ChartCandlesGroup::addCandles);
 *   TaskManager::get()->registerTask(command);
 *
 * Если открыть класс CommandLoadStock, то можно увидеть что он состоит из вызова двух задач TaskLoadStockFromDb
 * и TaskLoadStocksFromBroker что мы собственно и сделали
 */

#ifndef IBASECOMMAND_H
#define IBASECOMMAND_H

#include <QQueue>
#include <QRecursiveMutex>

#include "ibasetask.h"
#include "Tasks/Interfaces/interfase.h"


#include <QMetaMethod>


namespace Task {


class IBaseCommand : public IBaseTask
{
    Q_OBJECT

public:
    explicit IBaseCommand(const QString commandName);
    ~IBaseCommand();

    void setData(SharedInterface &inputData) override;
    SharedInterface &getResult() override;

    //Реристрирует задачу
    virtual void registerFunc(IFunction *newTask);

    /* Создаёт и запускает функцию
    *  T - класс добавляемой задачи, наследник IFunction
    *  N...args - аргументы необходимые для функции конструктора задачи
    *  Возвращает ссылку на созданную задачу (например для подключения к сигналу finished)
    *  Пример:
    *    TaskManager::get()->addTask <LoadStock> (rangeInterface, stockKey, candleCount);
    */
    template <class T, typename... N>
    std::enable_if_t<std::is_base_of_v<IFunction, T>, T*>
    execFunc(SharedInterface &inputData, N ... args)
    {
        T *newFunc = new T(args ...);
        assert(newFunc->isFunction() && "Can't exec task! use createTask!");
        newFunc->setData(inputData);
        newFunc->exec();
        newFunc->deleteLater();
        return newFunc;
    }

    /* Создаёт новую задачу
    *  T - класс добавляемой задачи, наследник IFunction
    *  N...args - аргументы необходимые для функции конструктора задачи
    *  Возвращает ссылку на созданную задачу (например для подключения к сигналу finished)
    *  Пример:
    *    TaskManager::get()->addTask <LoadStock> (rangeInterface, stockKey, candleCount);
    */
    template <class T, typename... N>
    std::enable_if_t<std::is_base_of_v<IFunction, T>, T*>
    createTask(N ... args)
    {
        T *newTask = new T(args ...);
        newTask->setThread(taskThread);
        registerFunc(newTask);
        return newTask;
    }

    ///Подключает method к сигналу finished : thisCommand->connect(this, SLOT(slotFinished())), this - это this класса к которому принадлежит слот
    void connect(QObject *receiver, const char *method);

signals:
    //Сигнал остановки всех задач
    void stopAll();

protected:
    //Основной цикл выполнения задачи
    void exec() override;

    //Остановить задату
    void stop() override;

    //Запуск очередной задачи
    virtual void runNextTask(IFunction *previousTask = nullptr);

    virtual void startTask(IBaseTask *task);

protected slots:
    //Обработка завершения потока
    virtual void taskFinished();

protected:
    //QRecursiveMutex mutex;
    QQueue<IFunction*> taskList;  //очередь задач на запуск

private:
    IFunction *lastTask = nullptr;
};

}

#endif // IBASECOMMAND_H
