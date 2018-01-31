#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <QString>
#include <QVector>

#include <mpi.h>

#include <modulight/common/moduledescription.hpp>
#include <modulight/common/network.hpp>
#include <modulight/common/dynamicrequest.hpp>

#include <modulight/master/process.hpp>
#include <modulight/master/masterconnection.hpp>
#include <modulight/master/argumenthandler.hpp>
#include <modulight/master/hostfile.hpp>
#include <modulight/master/reachableexecutables.hpp>
#include <modulight/master/userinterface.hpp>

namespace modulight
{
/**
 * @defgroup groupApplication Application tools
 * @brief Regroups everything needed to create an application and to deploy it
 * @{
 */

/**
 * @brief The Application class is the main tool to build applications
 *
 * This class handles Modulight process launching protocol and should be used to create every Modulight application.<br/>
 * It provides every tool needed to specify initial settings of an application.<br/>
 * <br/>
 * The common way to create an application is the following:<br/>
 * <ol>
 *     <li> You create an application
 *     <li> Then you can do a list of things including:
 *     <ul>
 *         <li> Adding the process you want to the application
 *         <li> Giving arguments to your processes
 *         <li> Connecting your processes together
 *     </ul>
 *     <li> Then you call the start() method
 * </ol>
 * <br/>
 * The following code shows a common way to use this class:
 * @code
 * #include <modulight/application.hpp>
 *
 * using namespace modulight::user_interface;
 *
 * int main(int argc, char ** argv)
 * {
 *     modulight::Application app(argc, argv);
 *
 *     Process * a = app.addProcess("moduleA");
 *     Process * b = app.addProcess("moduleB");
 *
 *     app.connect(a, "out", b, "in");
 *
 *     app.start();
 *
 *     return 0;
 * }
 * @endcode
 */
class Application
{
public:
    /**
     * @brief Constructor
     * @param argc The main function argument count
     * @param argv The main function argument values
     *
     * If you don't use the real main function argc and argv, your application won't be
     * able to execute options such as --dot. The option list may be accessed with the --help io
     */
    Application(int argc, char ** argv);

    /**
      * @brief Destructor
      */
    ~Application();

    /**
     * @brief Starts the application
     */
    void start();

    /** @name Process adding methods
     * These methods allow to add processes to an application
     */
    ///@{

    /**
     * @brief Adds a process in the application
     * @param executableFilename The filename which will be executed
     * @return The added process
     */
    user_interface::Process * addProcess(const QString & executableFilename);

    /**
     * @brief Adds a parallel process in the application
     * @param executableFilename The filename which will be executed
     * @param processCount The number of instances to execute
     * @return The added parallel process
     */
    user_interface::ParallelProcess * addParallelProcess(const QString & executableFilename, unsigned int processCount);

    ///@}

    /** @name Connection methods
     * These methods allow to connect processes to other processes in an application
     */
    ///@{

    /**
     * @brief Connects an input port to an output port
     * @param processA The source process
     * @param portA The source port name
     * @param processB The destination process
     * @param portB The destination port name
     * @param lossyConnection if set to true, the connection will be lossy. Otherwise, it will be lossless
     */
    void connect(user_interface::Process * processA, const QString & portA,
                 user_interface::Process * processB, const QString & portB,
                 bool lossyConnection = false);

    /**
     * @brief Connects an input port to an output port
     * @param processA the source process
     * @param portA the source port name
     * @param processB the destination parallel process
     * @param portB the destination port name
     * @param lossyConnection if set to true, the connection will be lossy. Otherwise, it will be lossless
     *
     * The connection will be of type 1-n, which means the source process will be connected to every instance of the parallel process.
     */
    void connect(user_interface::Process *processA, const QString &portA,
                 user_interface::ParallelProcess *processB, const QString &portB,
                 bool lossyConnection = false);

    /**
     * @brief Connects an input port to an output port
     * @param processA the source parallel process
     * @param portA the source port name
     * @param processB the destination process
     * @param portB the destination port name
     * @param lossyConnection if set to true, the connection will be lossy. Otherwise, it will be lossless
     *
     * The connection will be of type n-1, which means every instance of the parallel process will be connected to the destination process.
     */
    void connect(user_interface::ParallelProcess *processA, const QString &portA,
                 user_interface::Process *processB, const QString &portB,
                 bool lossyConnection = false);

    ///@}

    /** @name Process configuration methods
    * These methods allow to specify running arguments to the processes of an application
    * or to grant them dynamic rights on the application
    */
   ///@{

    /**
     * @brief Gives a process the right to access the application network and to do dynamic requests
     * @param process The process
     */
    void allowProcessToAlterNetwork(user_interface::Process * process);

    /**
     * @brief Gets the application arguments
     * @return The application arguments
     *
     * Application arguments are running parameters of an application which can be set with the --args option.<br/>
     * This method can be called at any time once the application is created.<br/>
     * <br/>
     * The following example shows how to use this method:
     * @code
     * #include <modulight/application.hpp>
     *
     * using namespace modulight::user_interface;
     *
     * int main(int argc, char * argv)
     * {
     *     Modulight::Application app(argc, argv);
     *
     *     QString filename = app.arguments().getInt("loadFilename", "unset");
     *     int visuCount = app.arguments().getInt("visuCount", 1);
     *
     *     Process * loader = app.addProcess("loader");
     *     Process * worker = app.addProcess("worker");
     *     ParallelProcess * visu = app.addProcess("visualization", visuCount);
     *
     *     app.setArgument(loader, "fileToRead", filename);
     *
     *     app.connect(loader, "data", worker, "dataIn");
     *     app.connect(worker, "dataOut", visu, "dataIn");
     *
     *     app.start();
     *     return 0;
     * }
     * @endcode
     *
     * Arguments can then be set at running time with the --args FILE option,
     * where FILE is an XML file describing an application's arguments.<br/>
     * The following example show an example of such a file:
     * @verbatim
     <args>
         <arg name="loadFilename" value="example.txt"/>
         <arg name="visuCount" value="4"/>
     </args>
     @endverbatim
     *
     */
    const ArgumentReader & arguments() const { return _arguments; }

    /**
     * @brief Sets the value of a process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::Process * process, const QString & arg, const QString & value);

    /**
     * @brief Sets the value of a process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::Process * process, const QString & arg, const char * value);

    /**
     * @brief Sets the value of a process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::Process * process, const QString & arg, int value);

    /**
     * @brief Sets the value of a process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::Process * process, const QString & arg, float value);

    /**
     * @brief Sets the value of a process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::Process * process, const QString & arg, bool value);

    /**
     * @brief Sets the value of a parallel process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * This argument will be sent on every instance of the parallel process.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::ParallelProcess * process, const QString & arg, const QString & value);

    /**
     * @brief Sets the value of a parallel process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * This argument will be sent on every instance of the parallel process.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::ParallelProcess * process, const QString & arg, const char * value);

    /**
     * @brief Sets the value of a parallel process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * This argument will be sent on every instance of the parallel process.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::ParallelProcess * process, const QString & arg, int value);

    /**
     * @brief Sets the value of a parallel process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * This argument will be sent on every instance of the parallel process.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::ParallelProcess * process, const QString & arg, float value);

    /**
     * @brief Sets the value of a parallel process argument
     * @param process The process
     * @param arg The argument name
     * @param value The new value
     *
     * This method can be used to add or change an argument value.<br/>
     * This argument will be sent on every instance of the parallel process.<br/>
     * <br/>
     * See also : Module::arguments()
     */
    void setArgument(user_interface::ParallelProcess * process, const QString & arg, double value);

    ///@}

private:
    void launchProcesses();
    void sendArguments();

    void receiveDescriptions();
    void handleInstanceInformation();

    void checkPorts();

    void handleOrders();
    void startProcesses();
    void abortLaunch();

    void splitCommunicators();

    void waitForFinished();

    void dynamicLoop();

    bool handleAddConnection(const DynamicRequest & r);
    bool handleRemoveConnection(const DynamicRequest & r);
    bool handleAddModule(const DynamicRequest & r);
    bool handleRemoveModule(const DynamicRequest & r, int requestSourcePID);

    bool containsProcess(int pid) const;
    bool containsProcess(const QString & name, int instance) const;
    Process & processById(int pid);
    const Process & processById(int pid) const;

    bool isCurrentlyConnected(int pidA, const QString & portA,
                              int pidB, const QString & portB);

    Process & processByNameAndInstance(const QString & name, int instance);
    const Process & processByNameAndInstance(const QString & name, int instance) const;

    Network initialNetwork() const;
    Network currentNetwork() const;

private:
    ArgumentHandler * _argumentHandler;

    HostFile _hostfile;
    ArgumentReader _arguments;

    QVector<Process> _processes;
    QVector<user_interface::Process *> _userProcesses;
    QVector<user_interface::ParallelProcess*> _userParallelProcesses;
    QVector<MasterConnection> _pendingConnections;
    QVector<MasterConnection> _currentConnections;

    ReachableExecutables _reachableExecutables;
    bool _started;
};
///@}
}

#endif // APPLICATION_HPP
