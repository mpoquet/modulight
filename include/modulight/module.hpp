#ifndef MODULE_HPP
#define MODULE_HPP

#include <iostream>

#include <mpi.h>
#include <zmq.hpp>

#include <QMap>

#include <modulight/module/port.hpp>
#include <modulight/module/messagereader.hpp>
#include <modulight/module/messagewriter.hpp>
#include <modulight/module/modulestate.hpp>

#include <modulight/common/sequence.hpp>
#include <modulight/common/arguments.hpp>
#include <modulight/common/network.hpp>
#include <modulight/common/dynamicorder.hpp>
#include <modulight/common/dynamicrequest.hpp>

namespace modulight
{
/**
 * @defgroup groupModule Module tools
 * @brief Regroups everything needed to create modules
 * @{
 */

/**
 * @brief The Module abstract class is the main tool to build modules
 * \nosubgrouping
 *
 * This class handles Modulight process launching protocol and should be used to create every Modulight module.<br/>
 * It provides every tool needed to communicate with the rest of the application (via ports) and to control the application network.<br/>
 * <br/>
 * The common way to use a module is the following:<br/>
 * <ol>
 *     <li> You create an instance of the module
 *     <li> You specify what your input and outputs ports are. It is generally done in the constructor of the module, by calling addInputPort() and addOutputPort().
 *     <li> You call the start() method, which will take care of the module environment and then call the run() method
 * </ol>
 *
 * The run() method must be redefining. It is important to understand that a module must be iterative (call the wait method occasionally)
 * to handle both lossy connections and dynamic alterations of the application network.
 *
 * Furthermore, to avoid hazardous behaviors, please check the readMessage method return value.
 * <br/>
 * The following codes shows a common way to use the Module class:
 */
class Module
{
public:
    /** @name External methods
     * These methods allow to create, destroy and start a module
     */
    ///@{

    /**
     * @brief Constructor
     * @param moduleName The module name
     * @param argc The main function argument count
     * @param argv The main function argument values
     *
     * Please note that two modules sharing the same name within one Modulight application will NOT be distinguished, they will share the same set for their instance numbers.<br/>
     */
    Module(const QString & moduleName, int argc, char ** argv);

    /**
     * @brief Destructor
     *
     *
     */
    virtual ~Module();

    /**
     * @brief The start method initializes the process then calls the run() method
     *
     * Please note that once start() is called, further calls of addInputPort() and addOutputPort() will fail1<br/>
     * This method must be called once and only once for the module to work properly and to correctly finalize the MPI context.<br/>
     * <br/>
     * This method will return when the process execution is over, which can happen in three cases:
     * <ul>
     *  <li> The process execution had been aborted, the run() method had not been called,
     *  <li> The process called the run() method, which had finished,
     *  <li> The process received a dynamic kill order while the run() method was being executed.
     * </ul>
     */
    void start();

    /**
     * @brief Initializes a Module
     */
    bool initialize();

    /**
     * @brief Finalizes a Module
     */
    void finalize();

    ///@}

    /** @name Environment methods
     * These methods allow to extract information about the module and its environment
     */
    ///@{

    /**
     * @brief Gets the module name
     * @return The module name
     */
    QString name() const { return _name; }

    bool isRunning() const { return _state == ModuleState::RUNNING || _state == ModuleState::RUNNING_WITHOUT_ENVIRONMENT; }

    /**
     * @brief Gets the instance number of the current module instance
     * @return The module instance number
     *
     * This method will be able to determine a module instance number only once the start() method had been called.<br/>
     * Before the start() call, it would return -42. Otherwise, instance numbers are positive integers which are uniques during an application execution
     */
    int instanceNumber() const { return _instanceNumber; }

    /**
     * @brief This method is a convenient way to display information
     * @return The default std::ostream, std::cout
     *
     * This method will write the module name, its instance number, a space and then return std::cout.<br/>
     * For example, if you are the first instance of a module called CustomModule, the following line
     * @code display() << "Hello world!" << endl; @endcode
     * will output @verbatim CustomModule0 Hello world! @endverbatim
     */
    std::ostream & display() const;

    /**
     * @brief This method is a convenient way to display errors
     * @return The default error std::ostream, std::cerr
     *
     * This method will write the module name, its instance number, a space and then return std::cerr
     */
    std::ostream & error() const;

    /**
     * @brief Gets the module arguments
     * @return The module arguments
     *
     * Module arguments are running parameters of a module which can be set in an application, thanks to Application::setArgument<br/>
     * This method can be called at any time: in Module's children classes constructors, before the start() method call and after it.
     *
     * The following example shows how to use this method:
     * @code
     * void MyModule::run()
     * {
     *     bool doSend = arguments().getBool("doSend", true);
     *     int sendCount = arguments().getInt("sendCount", 0);
     *
     *     if (doSend)
     *     {
     *         for (int i = 0; i < sendCount; ++i)
     *         {
     *             wait();
     *
     *             modulight::MessageWriter message;
     *             message.writeQString("Hello !");
     *
     *             send("out", message);
     *         }
     *     }
     * }
     * @endcode
     * If a specified parameter is unset in the application, the default value will be used for it and the module will do nothing (because sendCount's value will be 0).<br/>
     * <br/>
     * View also: Application::setArgument
     */
    const ArgumentReader & arguments() const;

    ///@}

public:
    /** @name Module definition methods
     * These methods allow to specify the ports of a module.<br/>
     * These methods should be called when the Module is uninitialized, that is to say before the initialize method call.
     */
    ///@{

    /**
     * @brief Add an input port
     * @param name The input port name
     *
     * Please note this method can only be called before the initialize method call.
     */
    void addInputPort(const QString & name);

    /**
     * @brief Adds an output port
     * @param name The output port name
     *
     * Please note this method can only be called before the initialize method call.
     */
    void addOutputPort(const QString & name);

    ///@}

    /** @name Running-time methods
     * Once the process is running, that is to say after the start() method call had been done,
     * these methods allow the module to interact with its environment.<br/>
     */
    ///@{

    /**
     * @brief Tries to read a message from an input port and store it in a MessageReader
     * @param iport The input port name
     * @param reader The MessageReader
     * @return true if a valid message had been read, false otherwise.
     *
     * This method returns false if there is no message on the given input port.<br/>
     * This method also returns false if an invalid message is available on the given input port, which occurs on lossy connections (to avoid message duplication, false messages may be sent).
     */
    bool readMessage(const QString & iport, MessageReader & reader);

    /**
     * @brief Tries to read a message from an input port and store it in a user specified memory location
     * @param iport The input port name
     * @param data The data pointer
     * @param size The data maximum size, in bytes
     * @return true if a valid message had been read, false otherwise.
     *
     * This method returns false if there is no message on the given input port.<br/>
     * This method also returns false if an invalid message is available on the given input port, which occurs on lossy connections (to avoid message duplication, false messages may be sent).
     *
     * If this method returns false, the user specified memory location had not been written by this method.
     */
    bool readMessage(const QString & iport, char * data, unsigned int size);

    /**
     * @brief Allows to wait, without specifying any waiting condition.
     *
     * This method allows to enter the waiting state without blocking.<br/>
     * Every module needs to be in the waiting state occasionally, to handle message sending on lossy connections and to allow dynamic connections.
     */
    void wait();

    /**
     * @brief Allows to wait on a single input port
     * @param iport The input port on which the wait will occur
     * @param msToWait The total number of milliseconds to wait. If set to 0, the method will return immediately. If set to -1, the method will loop until the port has received a message
     * @param msToSleep The number of milliseconds to sleep between two tries. If set to 0, there won't be any sleep. If set to -1, the sleep time will be set to 1 µs
     * @return true if a message had been received, false otherwise
     *
     * This method allows to enter the waiting state.<br/>
     * Every module needs to be in the waiting state occasionally, to handle message sending on lossy connections and to allow dynamic alterations of the application network.
     */
    bool wait(const QString & iport, int msToWait = -1, int msToSleep = 1);

    /**
     * @brief Allows to wait on one of many input ports sets
     * @param iports The set of sets of input ports on which the wait will occur
     * @param msToWait The total number of milliseconds to wait. If set to 0, the method will return immediately. If set to -1, the method will loop until the port has received a message
     * @param msToSleep The number of milliseconds to sleep between two tries. If set to 0, there won't be any sleep. If set to -1, the sleep time will be set to 1 µs
     * @return true if at least one of the input ports sets waiting condition is fulfilled, which means a message is available on every input port within it
     *
     * This method allows to enter the waiting state.<br/>
     * Every module needs to be in the waiting state occasionally, to handle message sending on lossy connections and to allow dynamic alterations of the application network.
     */
    bool wait(const QList<QStringList> & iports, int msToWait = -1, int msToSleep = 1);

    /**
     * @brief This method allows to know whether a message can be read on the given input port or not
     * @param iport The input port on which the message existence is checked
     * @return true if a message is available, false otherwise
     *
     * Please note that message availability doesn't mean message validity. Please check readMessage() return value to avoid any hazardous behaviour
     */
    bool messageAvailable(const QString & iport);

    /**
     * @brief This method allows to know whether a message can be read on every given input ports or not
     * @param iports The input ports on which the message existence is checked
     * @return true if a message is available on every input port, false otherwise
     *
     * Please note that message availability doesn't mean message validity. Please check readMessage() return value to avoid any hazardous behaviour
     */
    bool messageAvailable(const QStringList & iports);

    /**
     * @brief Thiodules method allows to know whether an input port is connected or not
     * @param iport The input port
     * @return true if there is at least one connection on the given input port, false otherwise
     */
    bool isInputPortConnected(const QString & iport);

    /**
     * @brief This method allows to know whether an output port is connected or not
     * @param oport The input port
     * @return true if there is at least one connection on the given output port, false otherwise
     */
    bool isOutputPortConnected(const QString & oport);

    /**
     * @brief This method allows to send a message on a given output port
     * @param port The output port on which the message is sent
     * @param writer The MessageWriter, which allowed the user to write data in the message
     */
    void send(const QString & port, const MessageWriter & writer);

    /**
     * @brief This method allows to send raw data on a given output port
     * @param port The output port on which the message is sent
     * @param data The pointer to the data
     * @param size The number of bytes to send
     */
    void send(const QString &port, const char * data, unsigned int size);

    /**
     * @brief Gets every available source of a given port. Sources are formatted as 12:3, where 1 is the module name, 2 the module instance number and 3 the port name
     * @param iport The input port
     * @return every available source of iport
     */
    QStringList portSources(const QString & iport);

    /**
     * @brief Gets a MPI communicator which can be used as MPI_COMM_WORLD in simpler SPMD MPI applications
     * @return A communicator which can be used as MPI_COMM_WORLD in simpler SPMD MPI applications
     *
     * This method gives a communicator containing all instances of the current module within an application.<br/>
     * All ranks within it are consecutive, starting at 0 and finishing at MPI_Comm_size(mpiWorld())-1.<br/>
     * <br/>
     * If you are implementing a parallel module, using MPI_COMM_WORLD wouldn't work properly since
     * all initial processes of a running application are in the same MPI_COMM_WORLD. This method allows the user to
     * get a communicator which can be used like MPI_COMM_WORLD in simpler SPMD MPI applications.
     */
    MPI_Comm mpiWorld() const;

    ///@}

    ///@}

    /** @name Dynamic control methods
     * These methods allows a module to have some control on the application.<br/>
     * Please note that such modules must have the right to do it in the current application.
     * To do so, use Application::allowProcessToAlterNetwork.<br/>
     * These methods can only be called when the module is running.
     */
    ///@{

    /**
     * @brief Gets the current Network of the application
     * @param network The current network
     * @return true if the module can access the Network, false otherwise
     *
     * Please note that this method does a network request to the Master, which may take a certain amount of time
     */
    bool getNetwork(Network & network);

    /**
     * @brief This method allows to dynamically add a connection in the network
     * @param sourceName The name of the source process
     * @param sourceInstance The instance number of the source process
     * @param sourcePort The output port name of the source process
     * @param destinationName The name of the destination process
     * @param destinationInstance The instance number of the destination process
     * @param destinationPort The input port name of the destination process
     * @param lossyConnection If set to true, the connection will be lossy. Otherwise, it will be lossless
     * @return true if the connection has been done, false otherwise
     */
    bool addConnection(const QString & sourceName, int sourceInstance, const QString & sourcePort,
                       const QString & destinationName, int destinationInstance, const QString & destinationPort,
                       bool lossyConnection = false);

    /**
     * @brief This method allows to dynamically remove a connection in the network
     * @param sourceName The name of the source process
     * @param sourceInstance The instance number of the source process
     * @param sourcePort The output port name of the source process
     * @param destinationName The name of the destination process
     * @param destinationInstance The instnace number of the destination process
     * @param destinationPort The input port name of the destination process
     * @return true if the connection has been removed, false otherwise
     */
    bool removeConnection(const QString & sourceName, int sourceInstance, const QString & sourcePort,
                          const QString & destinationName, int destinationInstance, const QString & destinationPort);

    /**
     * @brief This method allows to dynamically apply a sequence of dynamic requests
     * @param sequence The DynamicRequestSequence
     * @return true if the sequence had been handled correctly, false otherwise
     */
    bool applyDynamicRequestSequence(DynamicRequestSequence & sequence);

    /**
     * @brief This method allows to dynamically spawn a process
     * @param command The process command. Please note it must be accessible from the Master.
     * @param writer The ArgumentWriter which allows to specify launch parameters to the process that will be launched.
     * @param hostname The hostname on which the process will be spawn. Please note that this hostname must be known to MPI (mpirun -host or -hostfile).
     * @return true if the spawn has been succesful, false otherwise
     */
    bool spawnProcess(const QString & command, const ArgumentWriter & writer = ArgumentWriter(), const QString & hostname = "useStaticParameters");

    /**
     * @brief This method allows to dynamically kill a process
     * @param moduleName The target process module name
     * @param instance The target process instance number
     * @return true if the kill has been successful, false otherwise
     */
    bool killProcess(const QString & moduleName, int instance);

private:
    QString xmlDescription() const;
    void sendDescription();

    void receiveInstanceInformation();

    void createPollItems();
    void fillCompletePortNames();

    bool receiveOrders(); //! true => start order received; false => launch aborted
    void handleSequence(const Sequence &seq);

    /**
     * @brief checkDynamicOrders Checks if there is any dynamic order to execute. If so, executes it
     * @return true if something had been done, false otherwise
     */
    bool checkDynamicOrders();

    void handleDynamicOrders();

    void handleDynamicAccept(const DynamicOrder & o);
    void handleDynamicConnect(const DynamicOrder & o);
    void handleDynamicOutputDisconnect(const DynamicOrder & o);
    void handleDynamicInputDisconnect(const DynamicOrder & o);
    void handleDynamicDestroy();

    bool sendAndReceiveRequest(const DynamicRequest & r);

    void handleOnRequestSends();
    void updateMessageAvailability();

    // These methods are useful to display information for debugging purpose
    QString currentConnectionsToString();

private:
    QString _name;
    QString _ip;
    int _instanceNumber;
    bool _launchWithoutEnvironment;
    int _iterationNumber;
    ModuleState::ModuleState _state;

    zmq::context_t _context;
    zmq::socket_t _syncRep;
    quint16 _syncRepPort;

    QMap<QString, InputPort> _inputPorts;
    QMap<QString, OutputPort> _outputPorts;

    QVector<zmq_pollitem_t> _pollItems;

    ArgumentReader _arguments;

    MPI_Comm _parent;
    MPI_Comm _world;
};
///@}
}

/*! \mainpage notitle
 *
 * \section intro_sec Introduction
 * Modulight is a middleware which allows to:
 * <ol>
 *     <li> Create components (named Modules)
 *     <li> Form applications
 *     <li> Deploy applications on a cluster
 * </ol>
 *
 * The goals of Modulight are to:
 * <ol>
 *     <li> Provide an API simple enough to make this kind of application simpler to design and to update
 *     <li> Allow dynamic alterations of an application
 * </ol>
 *
 * \section install_sec Installation
 *
 * \subsection install_sec_1 Building
 *
 * \subsubsection install_sec_1_1 Using qmake project files
 *
 * You can build modulight from its root directory with the following command:
 * @verbatim qmake -r && make@endverbatim
 * Assuming qmake is using Qt5+.
 *
 * \subsubsection install_sec_1_2 Using CMake
 *
 * You can also build modulight via CMake.
 * Assuming you are in the root directory of Modulight:
 * @verbatim mkdir build
 * cd build
 * cmake ..
 * make
 * @endverbatim
 *
 * \subsubsection install_sec_1_3 Using qbs
 * Building can also be done with qbs, with the following command:
 * @verbatim qbs @endverbatim
 *
 * \subsection install_sec_2 Setting the environment
 *
 * \subsubsection install_sec_2_1 Dynamic library path
 * In order to run applications, your system must find the modulight library file.
 * To do so, on Unix-like systems, you can either install modulight in a standard location or
 * add the modulight lib directory (the folder where libmodulight.so* are located) to your LD_LIBRARY_PATH environment variable.
 *
 * For example, you do it this way in bash:
 * @verbatim export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/path/to/modulight/lib @endverbatim
 *
 * \subsubsection install_sec_2_1 Module binaries path
 * When an application tries to run processes, it will search for executable files in the MODULIGHT_PATH environment variable.
 *
 * You can set it in bash for example:
 * @verbatim export MODULIGHT_PATH=/path/to/module1/bin:/path/to/module2/bin @endverbatim
 *
 * <!--\section example_sec Running the examples-->
 *
 * \section module_sec Creating modules
 * To create modules, please refer to the @ref groupModule page.
 *
 * \section app_sec Creating applications
 * To create applications, please refer to the @ref groupApplication page.
 *
 * \section deploy_sec Deploying applications
 * To deploys applications, you can refer to the following pages:
 * <ol>
 *     <li> HostFile
 *     <li> Application::arguments
 * </ol>
 */

#endif // MODULE_HPP
