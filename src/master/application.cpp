#include <modulight/application.hpp>

#include <iostream>
#include <stdexcept>

#include <unistd.h>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QDir>
#include <QTime>

#include <modulight/common/mpiutils.hpp>
#include <modulight/common/tag.hpp>
#include <modulight/common/xml.hpp>
#include <modulight/common/dot.hpp>
#include <modulight/common/modulightexception.hpp>

#include <modulight/master/userinterface.hpp>

using namespace std;
using namespace modulight::mpi_util;
using namespace modulight;

modulight::Application::Application(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    _argumentHandler = new ArgumentHandler(argc, argv);
    _started = false;

    QString out1;
    QString out2;
    int task = _argumentHandler->handle(out1, out2);

    if (task & ArgumentHandler::PARAM_FILE)
    {
        QFile f(out2);

        if (f.open(QIODevice::ReadOnly))
        {
            QString xmlParameters = f.readAll();
            _arguments.loadFromXML(xmlParameters); // todo : check validity
        }
        else
            cerr << "Cannot read parameter file " << out2.toStdString() << endl;
    }
}

modulight::Application::~Application()
{
    if (!_started)
        cerr << "Destructor of modulight::Application called whereas the application has not been started" << endl;

    if (_argumentHandler)
    {
        delete _argumentHandler;
        _argumentHandler = 0;
    }

    for (int i = 0; i < _userProcesses.size(); ++i)
        delete _userProcesses[i];

    for (int i = 0; i < _userParallelProcesses.size(); ++i)
        delete _userParallelProcesses[i];
}


user_interface::Process * modulight::Application::addProcess(const QString &executableFilename)
{
    Process p;
    p.command = executableFilename;

    if (_processes.isEmpty())
        p.id = 0;
    else
        p.id = _processes.last().id + 1;

    _processes.push_back(p);
    _userProcesses.push_back(new user_interface::Process(_processes.last().id));

    return _userProcesses.last();
}

user_interface::ParallelProcess * modulight::Application::addParallelProcess(const QString &executableFilename, unsigned int processCount)
{
    if (processCount < 1)
        throw Exception("Invalid addParallelProcess : processCount must be greater than 0");

    QVector<int> ids(processCount);

    for (unsigned int i = 0; i < processCount; ++i)
    {
        Process p;
        p.command = executableFilename;

        if (_processes.isEmpty())
            p.id = 0;
        else
            p.id = _processes.last().id + 1;

        _processes.push_back(p);
        ids[i] = p.id;
    }

    QVector<user_interface::Process *> processes;
    for (unsigned int i = 0; i < processCount; ++i)
    {
        user_interface::Process * process = new user_interface::Process(ids[i]);
        processes.append(process);
        _userProcesses.append(process);
    }

    _userParallelProcesses.push_back(new user_interface::ParallelProcess(processCount, ids, processes));

    return _userParallelProcesses.last();
}

void modulight::Application::connect(user_interface::Process *processA, const QString &portA,
                                     user_interface::Process *processB, const QString &portB,
                                     bool lossyConnection)
{
    if (!_userProcesses.contains(processA))
    {
        cerr << QString("Critical error : within connection %1, the first process does not exist").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : process does not exist");
    }
    else if (!_userProcesses.contains(processB))
    {
        cerr << QString("Critical error : within connection %1, the second process does not exist").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : process does not exist");
    }

    MasterConnection c;

    c.processA = processA->id();
    c.portA = portA;
    c.processB = processB->id();
    c.portB = portB;
    c.lossy = lossyConnection;

    if (_pendingConnections.contains(c))
    {
        cerr << QString("Error : within connection %1, connection duplication found").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : connection duplication found");
    }

    _pendingConnections.push_back(c);
}

void modulight::Application::connect(user_interface::Process *processA, const QString &portA,
                          user_interface::ParallelProcess *processB, const QString &portB,
                          bool lossyConnection)
{
    if (!_userProcesses.contains(processA))
    {
        cerr << QString("Critical error : within connection %1, the first process does not exist").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : process does not exist");
    }
    else if (!_userParallelProcesses.contains(processB))
    {
        cerr << QString("Critical error : within connection %1, the second process does not exist").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : process does not exist");
    }

    for (int i = 0; i < processB->size(); ++i)
    {
        MasterConnection c;

        c.processA = processA->id();
        c.portA = portA;
        c.processB = processB->ids()[i];
        c.portB = portB;
        c.lossy = lossyConnection;

        if (_pendingConnections.contains(c))
        {
            cerr << QString("Error : within connection %1, connection duplication found").arg(_pendingConnections.size()).toStdString();
            throw Exception("Connection error : connection duplication found");
        }

        _pendingConnections.push_back(c);
    }
}

void modulight::Application::connect(user_interface::ParallelProcess *processA, const QString &portA,
                          user_interface::Process *processB, const QString &portB,
                          bool lossyConnection)
{
    if (!_userParallelProcesses.contains(processA))
    {
        cerr << QString("Critical error : within connection %1, the first process does not exist").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : process does not exist");
    }
    else if (!_userProcesses.contains(processB))
    {
        cerr << QString("Critical error : within connection %1, the second process does not exist").arg(_pendingConnections.size()).toStdString();
        throw Exception("Connection error : process does not exist");
    }

    for (int i = 0; i < processA->size(); ++i)
    {
        MasterConnection c;

        c.processA = processA->ids()[i];
        c.portA = portA;
        c.processB = processB->id();
        c.portB = portB;
        c.lossy = lossyConnection;

        if (_pendingConnections.contains(c))
        {
            cerr << QString("Error : within connection %1, connection duplication found").arg(_pendingConnections.size()).toStdString();
            throw Exception("Connection error : connection duplication found");
        }

        _pendingConnections.push_back(c);
    }
}

void modulight::Application::setArgument(user_interface::Process *process, const QString &arg, const QString &value)
{
    if (!containsProcess(process->id()))
    {
        qDebug() << QString("Critical error : on setArgument, process does not exist");
        throw Exception("Argument error : invalid process");
    }

    Process & p = processById(process->id());

    p.args.addString(arg, value);
}

void modulight::Application::setArgument(user_interface::Process * process, const QString &arg, const char *value)
{
    setArgument(process, arg, QString(value));
}

void modulight::Application::setArgument(user_interface::Process * process, const QString &arg, int value)
{
    if (!containsProcess(process->id()))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    Process & p = processById(process->id());

    p.args.addInt(arg, value);
}

void modulight::Application::setArgument(user_interface::Process * process, const QString &arg, float value)
{
    if (!containsProcess(process->id()))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    Process & p = processById(process->id());

    p.args.addFloat(arg, value);
}

void modulight::Application::setArgument(user_interface::Process *process, const QString &arg, bool value)
{
    if (!containsProcess(process->id()))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    Process & p = processById(process->id());

    p.args.addBool(arg, value);
}

void modulight::Application::setArgument(user_interface::ParallelProcess *process, const QString &arg, const QString &value)
{
    if (!_userParallelProcesses.contains(process))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    for (int i = 0; i < process->size(); ++i)
    {
        Process & p = processById(process->ids()[i]);
        p.args.addString(arg, value);
    }
}

void modulight::Application::setArgument(user_interface::ParallelProcess *process, const QString &arg, const char *value)
{
    setArgument(process, arg, QString(value));
}

void modulight::Application::setArgument(user_interface::ParallelProcess *process, const QString &arg, int value)
{
    if (!_userParallelProcesses.contains(process))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    for (int i = 0; i < process->size(); ++i)
    {
        Process & p = processById(process->ids()[i]);
        p.args.addInt(arg, value);
    }
}

void modulight::Application::setArgument(user_interface::ParallelProcess *process, const QString &arg, float value)
{
    if (!_userParallelProcesses.contains(process))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    for (int i = 0; i < process->size(); ++i)
    {
        Process & p = processById(process->ids()[i]);
        p.args.addFloat(arg, value);
    }
}

void modulight::Application::setArgument(user_interface::ParallelProcess *process, const QString &arg, double value)
{
    if (!_userParallelProcesses.contains(process))
    {
        cerr << "Critical error : on setArgument, process does not exist" << endl;
        throw Exception("Argument error : invalid process");
    }

    for (int i = 0; i < process->size(); ++i)
    {
        Process & p = processById(process->ids()[i]);
        p.args.addDouble(arg, value);
    }
}

void modulight::Application::allowProcessToAlterNetwork(user_interface::Process * process)
{
    if (!containsProcess(process->id()))
    {
        cerr << "Critical error : on allowModuleToAlterNetwork, process does not exist" << endl;
        throw Exception("allowModuleToAlterNetwork error : invalid process");
    }

    Process & p = processById(process->id());

    p.canAlterNetwork = true;
}

void modulight::Application::start()
{
    if (_started)
    {
        cerr << "Error : the application had already been started" << endl;
        return;
    }

    _started = true;

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 1)
    {
        cerr << "Error : the application should not be launched in parallel (try mpirun -np 1)" << endl;
        MPI_Finalize();
        return;
    }

    try
    {
        QString out1;
        QString out2;
        int task = _argumentHandler->handle(out1, out2); // todo, run on constructor (for arguments)

        if ((task & ArgumentHandler::EXIT) == 0) // all but EXIT
        {
            if (task & ArgumentHandler::HOST_FILE)
                _hostfile.loadFilename(out1);

            if (!_processes.empty())
            {
                launchProcesses();
                sendArguments();
                receiveDescriptions();
                handleInstanceInformation();
                checkPorts();

                if (task & (ArgumentHandler::DOT | ArgumentHandler::DOT_SIMPLE))
                {
                    Network network = initialNetwork();
                    QString dotString;

                    if (task == ArgumentHandler::DOT)
                        dot::networkToDot(network, dotString);
                    else
                        dot::networkToDotSimplified(network, dotString);

                    QFile file(out1);
                    if (file.open(QIODevice::WriteOnly))
                    {
                        file.write(dotString.toUtf8());
                        cout << QString("Dot description written in %1").arg(out1).toStdString() << endl;
                    }
                    else
                        cerr << QString("Cannot write file %1").arg(out1).toStdString() << endl;

                    abortLaunch();
                }
                else
                {
                    handleOrders();
                    splitCommunicators();
                    startProcesses();

                    dynamicLoop();
                }
            }
            else
                cout << "Information : no process to launch" << endl;
        }
    }
    catch (const Exception & e)
    {
        cerr << "Application caught an exception : " << e.what() << endl;
    }
    catch (const std::exception & e)
    {
        cerr << "Application caught a std::exception : " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Application caught an unknown type (not an Exception nor a std::exception)" << endl;
    }

    MPI_Finalize();
}

void modulight::Application::launchProcesses()
{
    QMap<QString, int> _commandCounter;

    int processNumber = _processes.size();

    char ** commands = new char*[processNumber];
    int * maxProcs = new int[processNumber];
    MPI_Info * infos = new MPI_Info[processNumber];
    int * errcodes = new int[processNumber];

    QVector<QByteArray> hosts;
    QVector<QByteArray> qbaCommands;
    QByteArray hostQBA = QString("host").toUtf8();

    bool executablesUpdated = false;

    for (int i = 0; i < processNumber; ++i)
    {
        Process & p = _processes[i];

        if (!_reachableExecutables.contains(p.command))
        {
            bool fail = true;

            if (!executablesUpdated)
            {
                executablesUpdated = true;
                _reachableExecutables.update();

                if (_reachableExecutables.contains(p.command))
                    fail = false;
            }

            if (fail)
            {
                throw Exception(QString("The process \"%1\" cannot be executed, launch aborted\n%2").arg(
                                    p.command, "Is your MODULIGHT_PATH environment variable set correctly?"));
            }
        }

        QByteArray qba = _reachableExecutables.absoluteFilename(p.command).toUtf8();

        qbaCommands.push_back(qba);

        commands[i] = qbaCommands.last().data();

        maxProcs[i] = 1;

        int instance;

        if (!_commandCounter.contains(p.command))
            _commandCounter[p.command] = instance = 0;
        else
            instance = ++_commandCounter[p.command];

        QString host = _hostfile.hostOf(p.command, instance);
        hosts.push_back(host.toUtf8());

        MPI_Info_create(&infos[i]);
        MPI_Info_set(infos[i], hostQBA.data(), hosts.back().data());
    }

    cout << "Spawning " << processNumber << " processes...";
    cout.flush();

    MPI_Comm children;

    MPI_Comm_spawn_multiple(processNumber, commands, MPI_ARGVS_NULL,
                            maxProcs, infos, 0, MPI_COMM_WORLD,
                            &children, errcodes);

    cout << " done" << endl;

    for (int i = 0 ; i < _processes.size(); ++i)
    {
        _processes[i].rank = i;
        _processes[i].comm = children;
    }

    delete[] maxProcs;
    delete[] errcodes;

    for (int i = 0; i < processNumber; ++i)
    {
        MPI_Info_free(&infos[i]);
    }

    delete[] infos;
    delete[] commands;
}

void modulight::Application::sendArguments()
{
    for (int i = 0; i < _processes.size(); ++i)
    {
        QString arguments = _processes[i].args.toXML();
        sendQString(arguments, _processes[i].rank, tag::ARGUMENTS, _processes[i].comm);
    }
}

void modulight::Application::receiveDescriptions()
{
    for (int i = 0; i < _processes.size(); ++i)
    {
        Process & p = _processes[i];

        cout << "Waiting for module description #" << p.id << " (" << p.command.toStdString() << ")...";
        cout.flush();

        QString xmlModuleDescription;
        recvQString(xmlModuleDescription, p.rank, tag::MODULE_DESCRIPTION, p.comm, MPI_STATUS_IGNORE);

        cout << " received" << endl;

        xml::readModuleDescription(xmlModuleDescription, p.description);
    }
}

void modulight::Application::handleInstanceInformation()
{
    QMap<QString, int> instanceCounter; // module name -> instance count
    // todo : set instanceCounter as an attribute, to avoid calculation on dynamic spawn

    for (int i = 0; i < _processes.size(); ++i)
    {
        QString moduleName = _processes[i].description.name;

        if (!instanceCounter.contains(moduleName))
            instanceCounter[moduleName] = 0;
        else
            instanceCounter[moduleName]++;

        _processes[i].instanceNumber = instanceCounter[moduleName];
    }

    for (int i = 0; i < _processes.size(); ++i)
        MPI_Send(&_processes[i].instanceNumber, 1, MPI_INT,
                 _processes[i].rank, tag::INSTANCE_INFO,
                 _processes[i].comm);
}

void modulight::Application::checkPorts()
{
    for (int i = 0; i < _pendingConnections.size(); ++i)
    {
        MasterConnection & c = _pendingConnections[i];

        const Process & a = processById(c.processA);
        const Process & b = processById(c.processB);

        if (!a.description.outputPorts.contains(c.portA))
        {
            cerr << QString("Error : invalid connection #%1 : %2 has no port %3").arg(i).arg(a.description.name).arg(c.portA).toStdString() << endl;
            throw Exception("Module description mismatches connections");
        }
        else if (!b.description.inputPorts.contains(c.portB))
        {
            cerr << QString("Error : invalid connection #%1 : %2 has no port %3").arg(i).arg(b.description.name).arg(c.portB).toStdString() << endl;
            throw Exception("Module description mismatches connections");
        }
    }

    cout << "Port check successful" << endl;
}

void modulight::Application::handleOrders()
{
    QMap<Process, Sequence> map;

    // Let's create orders of each process
    for (int i = 0; i < _pendingConnections.size(); ++i)
    {
        Process & pA = processById(_pendingConnections[i].processA);
        Process & pB = processById(_pendingConnections[i].processB);

        if (!map.contains(pA))
            map[pA] = Sequence();

        if (!map.contains(pB))
            map[pB] = Sequence();

        Connection cB;
        cB.isConnect = true;
        cB.isLossy = _pendingConnections[i].lossy;
        cB.localPortName = _pendingConnections[i].portB;
        cB.remoteIP = pA.description.ip;
        cB.syncPort = pA.description.syncPort;
        cB.remoteAbbrevName = QString("%1%2:%3").arg(pA.description.name).arg(pA.instanceNumber).arg(_pendingConnections[i].portA);

        if (_pendingConnections[i].lossy)
            cB.remotePort = pA.description.outputPorts[_pendingConnections[i].portA].lossyPort;
        else
            cB.remotePort = pA.description.outputPorts[_pendingConnections[i].portA].losslessPort;

        map[pB].connections.append(cB);

        Connection cA;
        cA.isConnect = false;
        cA.isLossy = _pendingConnections[i].lossy;
        cA.localPortName = _pendingConnections[i].portA;
        cA.remoteAbbrevName = QString("%1%2:%3").arg(pB.description.name).arg(pB.instanceNumber).arg(_pendingConnections[i].portB);
        map[pA].connections.append(cA);
    }

    int recvBuf[map.size()];
    MPI_Request requests[map.size()];

    // Let's send to every process its sequence
    QMapIterator<Process, Sequence> it(map);
    for (int i = 0; it.hasNext() ; ++i)
    {
        it.next();

        QString xmlString;
        xml::writeSequence(xmlString, it.value());

        sendQString(xmlString, it.key().rank, tag::SEQUENCE, it.key().comm);
        MPI_Irecv(&recvBuf[i], 1, MPI_INT, it.key().rank, tag::SEQUENCE_DONE, it.key().comm, &requests[i]);
    }

    // The master must wait that every implied process have done its connections
    MPI_Waitall(map.size(), requests, MPI_STATUSES_IGNORE);

    while (!_pendingConnections.isEmpty())
    {
        _currentConnections.append(_pendingConnections.first());
        _pendingConnections.pop_front();
    }

    cout << "Initial connections done" << endl;
}

void modulight::Application::startProcesses()
{
    cout << "Running processes" << endl;

    for (int i = 0; i < _processes.size(); ++i)
        MPI_Send(&i, 1, MPI_INT, _processes[i].rank,
                 tag::START_ORDER, _processes[i].comm);
}

void modulight::Application::abortLaunch()
{
    for (int i = 0; i < _processes.size(); ++i)
        MPI_Send(&i, 1, MPI_INT, _processes[i].rank,
                 tag::ABORT_LAUNCH, _processes[i].comm);
}

void modulight::Application::splitCommunicators()
{
    cout << "Splitting into MPI worlds...";
    cout.flush();

    QMap<Process, QPair<int, int> > map;

    for (int i = 0; i < _processes.size(); ++i)
        map[_processes[i]] = QPair<int, int>(i+1, 0);

    for (int i = 0; i < _userParallelProcesses.size(); ++i)
    {
        Process & p0 = processById(_userParallelProcesses[i]->ids()[0]);

        for (int j = 1; j < _userParallelProcesses[i]->size(); ++j)
        {
            Process & p = processById(_userParallelProcesses[i]->ids()[j]);
            map[p] = QPair<int, int>(map[p0].first, j);
        }
    }

    int buf[2];

    QMapIterator<Process, QPair<int, int> > it(map);
    while (it.hasNext())
    {
        it.next();
        buf[0] = it.value().first;
        buf[1] = it.value().second;

        MPI_Send(buf, 2, MPI_INT, it.key().rank, tag::SPLIT, it.key().comm);
    }

    MPI_Comm comm;
    MPI_Comm_split(_processes[0].comm, 0, 0, &comm);

    cout << " done" << endl;
}

void modulight::Application::waitForFinished()
{
    int buf[_processes.size()];
    MPI_Request requests[_processes.size()];
    MPI_Status status[_processes.size()];

    for (int i = 0; i < _processes.size(); ++i)
    {
        MPI_Irecv(&buf[i], 1, MPI_INT, _processes[i].rank,
                  tag::MODULE_FINISHED, _processes[i].comm, &requests[i]);
    }

    int everyoneFinished = 0;
    while (!everyoneFinished)
    {
        MPI_Testall(_processes.size(), requests, &everyoneFinished, status);

        if (!everyoneFinished)
            usleep(1000);
    }
}

void modulight::Application::dynamicLoop()
{
    bool isFinished = false;
    int msToSleep = 5;

    while (!isFinished)
    {
        for (int i = 0; i < _processes.size(); ++i)
        {
            Process p = _processes[i];
            MPI_Status status;
            int received;

            if (_processes[i].isFinished)
                continue;

            MPI_Iprobe(p.rank, MPI_ANY_TAG, p.comm, &received, &status);

            if (received)
            {
                tag::Tag t = (tag::Tag) status.MPI_TAG;

                if (t == tag::MODULE_FINISHED) // The modules wants to finish
                {
                    cout << QString("%1%2 is over").arg(p.description.name).arg(p.instanceNumber).toStdString() << endl;

                    int u;
                    MPI_Recv(&u, 1, MPI_INT, p.rank, tag::MODULE_FINISHED, p.comm, MPI_STATUS_IGNORE);

                    // Let's destroy every connection which implies the finishing module

                    QVector<MasterConnection> cToDelete;
                    for (int j = 0; j < _currentConnections.size(); ++j)
                        if (p.id == _currentConnections[j].processA || p.id == _currentConnections[j].processB)
                            cToDelete.append(_currentConnections[j]);

                    QMap<Process, DynamicOrderSequence> map;

                    for (int j = 0; j < cToDelete.size(); ++j)
                    {
                        const MasterConnection & c = cToDelete[j];

                        Process & pA = processById(c.processA);
                        Process & pB = processById(c.processB);

                        if (!map.contains(pA))
                            map[pA] = DynamicOrderSequence();

                        if (!map.contains(pB))
                            map[pB] = DynamicOrderSequence();

                        DynamicOrder orderA;
                        DynamicOrder orderB;

                        orderA.type = OrderType::OUTPUT_DISCONNECT;
                        orderA.localPortName = c.portA;
                        orderA.remoteAbbrevName = QString("%1%2:%3").arg(pB.description.name).arg(pB.instanceNumber).arg(c.portB);

                        orderB.type = OrderType::INPUT_DISCONNECT;
                        orderB.localPortName = c.portB;
                        orderB.remoteAbbrevName = QString("%1%2:%3").arg(pA.description.name).arg(pA.instanceNumber).arg(c.portA);
                        orderB.remoteIP = pA.description.ip;
                        orderB.syncPort = pA.description.syncPort;

                        if (c.lossy)
                            orderB.remotePort = pA.description.outputPorts[c.portA].lossyPort;
                        else
                            orderB.remotePort = pA.description.outputPorts[c.portA].losslessPort;

                        map[pA].orders.append(orderA);
                        map[pB].orders.append(orderB);
                    }

                    MPI_Request requests[map.size()];
                    int recvBuf[map.size()];

                    QMapIterator<Process, DynamicOrderSequence> it(map);
                    for (int j = 0; it.hasNext(); ++j)
                    {
                        it.next();

                        QString xmlOrders;
                        xml::writeDynamicOrders(xmlOrders, it.value());

                        sendQString(xmlOrders, it.key().rank, tag::DYNAMIC_ORDER, it.key().comm);

                        MPI_Irecv(&recvBuf[j], 1, MPI_INT, it.key().rank, tag::ORDER_DONE, it.key().comm, &requests[j]);
                    }

                    MPI_Waitall(map.size(), requests, MPI_STATUSES_IGNORE);

                    // Let's remove the connections
                    for (int j = 0; j < cToDelete.size(); ++j)
                    {
                        int index = _currentConnections.indexOf(cToDelete[j]);

                        if (index != -1)
                            _currentConnections.remove(index);
                        else
                            cerr << QString("Warning : cannot effectively remove a connection from the currentConnections, indexOf failed...").toStdString() << endl;
                    }

                    MPI_Send(&u, 1, MPI_INT, p.rank, tag::FINISH_ORDER, p.comm);

                    _processes[i].isFinished = true;

                    isFinished = true;

                    for (int j = 0; j < _processes.size(); ++j)
                        if (!_processes[j].isFinished)
                            isFinished = false;
                }
                else if (t == tag::NETWORK_REQUEST)
                {
                    int u;
                    MPI_Recv(&u, 1, MPI_INT, p.rank, tag::NETWORK_REQUEST, p.comm, MPI_STATUS_IGNORE);

                    if (p.canAlterNetwork)
                    {
                        Network network = currentNetwork();
                        QString xmlString;

                        xml::writeNetwork(xmlString, network);

                        sendQString(xmlString, p.rank, tag::NETWORK, p.comm);
                    }
                    else
                        sendQString("", p.rank, tag::NETWORK, p.comm);
                }
                else if (t == tag::DYNAMIC_REQUEST)
                {
                    QString xmlString;
                    int response = 1;

                    recvQString(xmlString, p.rank, tag::DYNAMIC_REQUEST, p.comm, MPI_STATUS_IGNORE);

                    if (p.canAlterNetwork)
                    {
                        DynamicRequestSequence requests;
                        xml::readDynamicRequests(xmlString, requests);

                        for (int j = 0; j < requests.requests.size(); ++j)
                        {
                            const DynamicRequest & r = requests.requests[j];

                            switch (r.type)
                            {
                            case RequestType::ADD_CONNECTION:
                            {
                                //QTime time;
                                //time.start();

                                if (!handleAddConnection(r))
                                    response = 0;

                                //cout << "Master took " << time.elapsed() << " ms to handle a AddConnection" << endl;
                            } break;
                            case RequestType::REMOVE_CONNECTION:
                            {
                                //QTime time;
                                //time.start();

                                if (!handleRemoveConnection(r))
                                    response = 0;

                                //cout << "Master took " << time.elapsed() << " ms to handle a RemoveConnection" << endl;
                            } break;
                            case RequestType::ADD_MODULE:
                            {
                                //QTime time;
                                //time.start();

                                if (!handleAddModule(r))
                                    response = 0;

                                //cout << "Master took " << time.elapsed() << " ms to handle a AddModule" << endl;
                            } break;
                            case RequestType::REMOVE_MODULE:
                            {
                                //QTime time;
                                //time.start();

                                if (!handleRemoveModule(r, p.id))
                                    response = 0;

                                //cout << "Master took " << time.elapsed() << " ms to handle a RemoveModule" << endl;
                            } break;
                            default:
                                break;
                            }
                        }
                    }
                    else
                    {
                        cerr << QString("Request received from %1%2, which is not allowed to alter network").arg(p.description.name).arg(p.instanceNumber).toStdString() << endl;
                        response = 0;
                    }

                    MPI_Send(&response, 1, MPI_INT, p.rank, tag::REQUEST_RESULT, p.comm);
                }
                else
                    throw Exception(QString("Invalid tag received from %1%2 (%3)").arg(p.description.name).arg(p.instanceNumber).arg((int)t));
            }
        }

        usleep(msToSleep * 1000);
    }

    cout << "Application is over" << endl;
}

bool modulight::Application::handleAddConnection(const DynamicRequest &r)
{
    if (!containsProcess(r.sourceName, r.sourceInstance) ||
        !containsProcess(r.destinationName, r.destinationInstance))
    {
        cerr << QString("Invalid ADD_CONNECTION request : no such process (%1%2:%3->%4%5:%6)").arg(
                    r.sourceName).arg(r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                    r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

        return false;
    }
    else
    {
        const Process & a = processByNameAndInstance(r.sourceName, r.sourceInstance);
        const Process & b = processByNameAndInstance(r.destinationName, r.destinationInstance);

        if (!a.description.outputPorts.contains(r.sourcePort) ||
            !b.description.inputPorts.contains(r.destinationPort))
        {
            cerr << QString("Invalid ADD_CONNECTION request : no such port (%1%2:%3->%4%5:%6)").arg(
                        r.sourceName).arg(r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                        r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

            return false;
        }
        else
        {
            if (isCurrentlyConnected(a.id, r.sourcePort, b.id, r.destinationPort))
            {
                cerr << QString("Invalid ADD_CONNECTION request : already exist (%1%2:%3->%4%5:%6)").arg(
                            r.sourceName).arg(r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                            r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

                return false;
            }

            DynamicOrder orderA, orderB;
            DynamicOrderSequence sequenceA, sequenceB;
            QString xmlA, xmlB;

            orderA.type = OrderType::ACCEPT;
            orderA.localPortName = r.sourcePort;
            orderA.remoteAbbrevName = QString("%1%2:%3").arg(r.destinationName).arg(r.destinationInstance).arg(r.destinationPort);
            orderA.lossyConnection = r.lossyConnection;

            orderB.type = OrderType::CONNECT;
            orderB.localPortName = r.destinationPort;
            orderB.remoteAbbrevName = QString("%1%2:%3").arg(r.sourceName).arg(r.sourceInstance).arg(r.sourcePort);
            orderB.remoteIP = a.description.ip;
            orderB.syncPort = a.description.syncPort;
            orderB.lossyConnection = r.lossyConnection;

            if (r.lossyConnection)
                orderB.remotePort = a.description.outputPorts[r.sourcePort].lossyPort;
            else
                orderB.remotePort = a.description.outputPorts[r.sourcePort].losslessPort;

            /*if (r.lossyConnection)
                cout << "Master is adding a lossy connection : " << a.description.outputPorts[r.sourcePort].lossyPort
                     << " instead of " << a.description.outputPorts[r.sourcePort].losslessPort << endl;
            else
                cout << "Master is adding a lossless connection : " << a.description.outputPorts[r.sourcePort].losslessPort
                     << " instead of " << a.description.outputPorts[r.sourcePort].lossyPort << endl;
            */


            sequenceA.orders.append(orderA);
            sequenceB.orders.append(orderB);

            xml::writeDynamicOrders(xmlA, sequenceA);
            xml::writeDynamicOrders(xmlB, sequenceB);

            // Orders are sent to each node
            sendQString(xmlA, a.rank, tag::DYNAMIC_ORDER, a.comm);
            sendQString(xmlB, b.rank, tag::DYNAMIC_ORDER, b.comm);

            int recvBuf[2];
            MPI_Request requests[2];

            MPI_Irecv(&recvBuf[0], 1, MPI_INT, a.rank, tag::ORDER_DONE, a.comm, &requests[0]);
            MPI_Irecv(&recvBuf[1], 1, MPI_INT, b.rank, tag::ORDER_DONE, b.comm, &requests[1]);

            // Let's wait for each node response
            MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

            MasterConnection c;
            c.processA = a.id;
            c.processB = b.id;
            c.portA = r.sourcePort;
            c.portB = r.destinationPort;
            c.lossy = r.lossyConnection;

            if (!c.lossy)
                cout << QString("New connection : %1%2:%3->%4%5:%6").arg(r.sourceName).arg(
                        r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                        r.destinationInstance).arg(r.destinationPort).toStdString() << endl;
            else
                cout << QString("New connection : %1%2:%3->%4%5:%6 (lossy)").arg(r.sourceName).arg(
                        r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                        r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

            _currentConnections.append(c);
            return true;
        }
    }
}

bool modulight::Application::handleRemoveConnection(const DynamicRequest &r)
{
    if (!containsProcess(r.sourceName, r.sourceInstance) ||
            !containsProcess(r.destinationName, r.destinationInstance))
    {
        cerr << QString("Invalid REMOVE_CONNECTION request : no such port (%1%2:%3->%4%5:%6)").arg(
                    r.sourceName).arg(r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                    r.destinationInstance).arg(r.destinationPort).toStdString() << endl;
        return false;
    }
    else
    {
        const Process & a = processByNameAndInstance(r.sourceName, r.sourceInstance);
        const Process & b = processByNameAndInstance(r.destinationName, r.destinationInstance);

        if (!a.description.outputPorts.contains(r.sourcePort) ||
            !b.description.inputPorts.contains(r.destinationPort))
        {
            cerr << QString("Invalid REMOVE_CONNECTION request : no such port (%1%2:%3->%4%5:%6)").arg(
                        r.sourceName).arg(r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                        r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

            return false;
        }
        else
        {
            if (!isCurrentlyConnected(a.id, r.sourcePort, b.id, r.destinationPort))
            {
                cerr << QString("Invalid REMOVE_CONNECTION request : no such connection (%1%2:%3->%4%5:%6)").arg(
                            r.sourceName).arg(r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                            r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

                return false;
            }

            DynamicOrder orderA, orderB;
            DynamicOrderSequence sequenceA, sequenceB;
            QString xmlA, xmlB;

            orderA.type = OrderType::OUTPUT_DISCONNECT;
            orderA.localPortName = r.sourcePort;
            orderA.remoteAbbrevName = QString("%1%2:%3").arg(r.destinationName).arg(r.destinationInstance).arg(r.destinationPort);

            orderB.type = OrderType::INPUT_DISCONNECT;
            orderB.localPortName = r.destinationPort;
            orderB.remoteAbbrevName = QString("%1%2:%3").arg(r.sourceName).arg(r.sourceInstance).arg(r.sourcePort);
            orderB.remoteIP = a.description.ip;
            orderB.syncPort = a.description.syncPort;

            MasterConnection c;
            c.processA = a.id;
            c.processB = b.id;
            c.portA = r.sourcePort;
            c.portB = r.destinationPort;

            int index = _currentConnections.indexOf(c);

            if (index == -1)
                throw Exception("Trying to remove an unknown connection");

            if (_currentConnections[index].lossy)
                orderB.remotePort = a.description.outputPorts[r.sourcePort].lossyPort;
            else
                orderB.remotePort = a.description.outputPorts[r.sourcePort].losslessPort;

            orderA.lossyConnection = _currentConnections[index].lossy;
            orderB.lossyConnection = _currentConnections[index].lossy;

            sequenceA.orders.append(orderA);
            sequenceB.orders.append(orderB);

            xml::writeDynamicOrders(xmlA, sequenceA);
            xml::writeDynamicOrders(xmlB, sequenceB);

            // Orders are sent to each node
            sendQString(xmlA, a.rank, tag::DYNAMIC_ORDER, a.comm);
            sendQString(xmlB, b.rank, tag::DYNAMIC_ORDER, b.comm);

            int recvBuf[2];
            MPI_Request requests[2];

            MPI_Irecv(&recvBuf[0], 1, MPI_INT, a.rank, tag::ORDER_DONE, a.comm, &requests[0]);
            MPI_Irecv(&recvBuf[1], 1, MPI_INT, b.rank, tag::ORDER_DONE, b.comm, &requests[1]);

            // Let's wait for each node response
            MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

            _currentConnections.remove(index);

            cout << QString("Connection removed : %1%2:%3->%4%5:%6").arg(r.sourceName).arg(
                        r.sourceInstance).arg(r.sourcePort).arg(r.destinationName).arg(
                        r.destinationInstance).arg(r.destinationPort).toStdString() << endl;

            return true;
        }
    }
}

bool modulight::Application::handleAddModule(const DynamicRequest &r)
{
    if (!_reachableExecutables.contains(r.path))
    {
        _reachableExecutables.update();

        if (!_reachableExecutables.contains(r.path))
        {
            cerr << QString("Invalid ADD_MODULE request : no such executable (%1").arg(r.path).toStdString() << endl;
            return false;
        }
    }

    int pid = addProcess(r.path)->id();

    Process & p = processById(pid);
    p.instanceNumber = 0;
    p.rank = 0;
    p.args.setMap(r.args);

    for (int i = 0; i < _processes.size(); ++i)
    {
        if (_processes[i].command == p.command)
        {
            if (_processes[i].id != p.id &&
                    _processes[i].instanceNumber >= p.instanceNumber)
                p.instanceNumber = _processes[i].instanceNumber + 1;
        }
    }

    QByteArray command = _reachableExecutables.absoluteFilename(p.command).toUtf8();
    QByteArray hostQBA = QString("host").toUtf8();
    QByteArray hostValueQBA;
    MPI_Info info;
    int errcode;

    if (r.host == "useStaticParameters")
        hostValueQBA = _hostfile.hostOf(p.command, p.instanceNumber).toUtf8();
    else
        hostValueQBA = r.host.toUtf8();

    MPI_Info_create(&info);
    MPI_Info_set(info, hostQBA.data(), hostValueQBA.data());

    MPI_Comm_spawn(command.data(), MPI_ARGV_NULL, 1, info, 0, MPI_COMM_WORLD, &p.comm, &errcode);

    MPI_Info_free(&info);

    QString arguments = p.args.toXML();
    sendQString(arguments, p.rank, tag::ARGUMENTS, p.comm);

    QString xmlDescription;
    recvQString(xmlDescription, p.rank, tag::MODULE_DESCRIPTION, p.comm, MPI_STATUS_IGNORE);
    xml::readModuleDescription(xmlDescription, p.description);

    MPI_Send(&p.instanceNumber, 1, MPI_INT, p.rank, tag::INSTANCE_INFO, p.comm);

    int u = 42;
    MPI_Send(&u, 1, MPI_INT, p.rank, tag::START_ORDER, p.comm);

    cout << QString("A module has been spawned (%1%2)").arg(p.description.name).arg(p.instanceNumber).toStdString() << endl;
    return true;
}

bool modulight::Application::handleRemoveModule(const DynamicRequest &r, int requestSourcePID)
{
    if (!containsProcess(r.moduleName, r.moduleInstance))
    {
        cerr << QString("Invalid REMOVE_MODULE request : no such process (%1%2)").arg(r.moduleName).arg(r.moduleInstance).toStdString() << endl;
        return false;
    }

    Process & p = processByNameAndInstance(r.moduleName, r.moduleInstance);

    if (p.id == requestSourcePID)
    {
        cerr << QString("Invalid REMOVE_MODULE request : suicide is forbidden").toStdString() << endl;
        return false;
    }

    // Let's destroy every connection which implies the module to kill
    QVector<MasterConnection> cToDelete;
    for (int i = 0; i < _currentConnections.size(); ++i)
        if (p.id == _currentConnections[i].processA || p.id == _currentConnections[i].processB)
            cToDelete.append(_currentConnections[i]);

    /*QString s = QString("In order to remove the process %1%2, these connections must be closed: (").arg(r.moduleName).arg(r.moduleInstance);
    if (!cToDelete.empty())
    {
        for (int i = 0; i < cToDelete.size(); ++i)
        {
            MasterConnection c = cToDelete[i];
            const Process & pA = processById(c.processA);
            const Process & pB = processById(c.processB);

            if (i != 0)
                s += ", ";
            s += QString("%1%2:%3->%4%5:%6").arg(pA.description.name).arg(pA.instanceNumber).arg(c.portA).arg(
                                                 pB.description.name).arg(pB.instanceNumber).arg(c.portB);
        }
    }
    cout << s.toStdString() << ')' << endl;*/
    QMap<Process, DynamicOrderSequence> map;

    // Let's fill the map correctly
    for (int i = 0; i < cToDelete.size(); ++i)
    {
        const MasterConnection & c = cToDelete[i];

        Process & pA = processById(c.processA);
        Process & pB = processById(c.processB);

        if (pA.id == requestSourcePID || pB.id == requestSourcePID)
        {
            Process & source = processById(requestSourcePID);

            cerr << QString("Invalid REMOVE_MODULE request : killing %1%2 implies a deadlock with the request source (%3%4) because they are connected").arg(
                        p.description.name).arg(p.instanceNumber).arg(source.description.name).arg(source.instanceNumber).toStdString() << endl;

            return false;
        }

        if (!map.contains(pA))
            map[pA] = DynamicOrderSequence();

        if (!map.contains(pB))
            map[pB] = DynamicOrderSequence();

        DynamicOrder orderA;
        DynamicOrder orderB;

        orderA.type = OrderType::OUTPUT_DISCONNECT;
        orderA.localPortName = c.portA;
        orderA.remoteAbbrevName = QString("%1%2:%3").arg(pB.description.name).arg(pB.instanceNumber).arg(c.portB);
        orderA.lossyConnection = c.lossy;

        orderB.type = OrderType::INPUT_DISCONNECT;
        orderB.localPortName = c.portB;
        orderB.remoteAbbrevName = QString("%1%2:%3").arg(pA.description.name).arg(pA.instanceNumber).arg(c.portA);
        orderB.remoteIP = pA.description.ip;
        orderB.syncPort = pA.description.syncPort;
        orderB.lossyConnection = c.lossy;

        if (c.lossy)
            orderB.remotePort = pA.description.outputPorts[c.portA].lossyPort;
        else
            orderB.remotePort = pA.description.outputPorts[c.portA].losslessPort;


        map[pA].orders.append(orderA);
        map[pB].orders.append(orderB);
    }

    // Let's send its orders to every implied process
    MPI_Request requests[map.size()];
    int recvBuf[map.size()];

    QMapIterator<Process, DynamicOrderSequence> it(map);
    for (int i = 0; it.hasNext(); ++i)
    {
        it.next();

        QString xmlOrders;
        xml::writeDynamicOrders(xmlOrders, it.value());

        sendQString(xmlOrders, it.key().rank, tag::DYNAMIC_ORDER, it.key().comm);

        MPI_Irecv(&recvBuf[i], 0, MPI_INT, it.key().rank, tag::ORDER_DONE, it.key().comm, &requests[i]);
    }

    MPI_Waitall(map.size(), requests, MPI_STATUSES_IGNORE);

    // Let's remove the connections
    for (int i = 0; i < cToDelete.size(); ++i)
    {
        int index = _currentConnections.indexOf(cToDelete[i]);

        if (index != -1)
            _currentConnections.remove(index);
        else
            cerr << QString("Warning : cannot effectively remove a connection from the currentConnections, indexOf failed...").toStdString() << endl;
    }

    // Let's send the kill order
    DynamicOrder killOrder;
    DynamicOrderSequence killOrderSequence;
    QString xmlKillOrder;
    int u;

    killOrder.type = OrderType::DESTROY;
    killOrderSequence.orders.append(killOrder);
    xml::writeDynamicOrders(xmlKillOrder, killOrderSequence);

    sendQString(xmlKillOrder, p.rank, tag::DYNAMIC_ORDER, p.comm);

    // Let's wait module's destruction confirmation
    MPI_Recv(&u, 1, MPI_INT, p.rank, tag::ORDER_DONE, p.comm, MPI_STATUS_IGNORE);

    // Let's put this process as finished
    // It is done to ensure the next currentNetwork() call won't include the killed process

    int index = _processes.indexOf(p);

    if (index != -1)
        _processes[index].isAboutToBeFinished = true;
    else
        cerr << QString("Warning : Cannot effectively set process %1%2 as finished, indexOf failed...").arg(r.moduleName).arg(r.moduleInstance).toStdString() << endl;

    cout << QString("Process killed (%1%2)").arg(r.moduleName).arg(r.moduleInstance).toStdString() << endl;
    return true;
}

bool modulight::Application::containsProcess(int pid) const
{
    for (int i = 0; i < _processes.size(); ++i)
        if (_processes[i].id == pid)
            return true;

    return false;
}

bool modulight::Application::containsProcess(const QString &name, int instance) const
{
    for (int i = 0; i < _processes.size(); ++i)
        if (_processes[i].description.name == name && _processes[i].instanceNumber == instance)
            return true;

    return false;
}

modulight::Process & modulight::Application::processById(int pid)
{
    Process p;
    p.id = pid;

    return _processes[_processes.indexOf(p)];
}

const modulight::Process & modulight::Application::processById(int pid) const
{
    Process p;
    p.id = pid;

    return _processes[_processes.indexOf(p)];
}

bool modulight::Application::isCurrentlyConnected(int pidA, const QString &portA, int pidB, const QString &portB)
{
    for (int i = 0; i < _currentConnections.size(); ++i)
    {
        const MasterConnection & c = _currentConnections[i];

        if (c.processA == pidA && c.portA == portA &&
            c.processB == pidB && c.portB == portB)
            return true;
    }

    return false;
}

modulight::Process & modulight::Application::processByNameAndInstance(const QString &name, int instance)
{
    for (int i = 0; i < _processes.size(); ++i)
        if (_processes[i].description.name == name && _processes[i].instanceNumber == instance)
            return _processes[i];

    throw Exception(QString("Within processByNameAndInstance, %1%2 does not exist").arg(name).arg(instance));
}

const modulight::Process & modulight::Application::processByNameAndInstance(const QString &name, int instance) const
{
    for (int i = 0; i < _processes.size(); ++i)
        if (_processes[i].description.name == name && _processes[i].instanceNumber == instance)
            return _processes[i];

    throw Exception(QString("Within processByNameAndInstance, %1%2 does not exist").arg(name).arg(instance));
}

modulight::Network modulight::Application::initialNetwork() const
{
    Network n;

    for (int i = 0; i < _processes.size(); ++i)
    {
        const Process & p = _processes[i];
        NetworkModuleDescription d;

        d.name = p.description.name;
        d.instance = p.instanceNumber;
        d.inputPorts = p.description.inputPorts;

        QMapIterator<QString, ModuleDescription::OutputPort> it(p.description.outputPorts);
        while (it.hasNext())
        {
            it.next();
            d.outputPorts.append(it.key());
        }

        n.modules.append(d);
    }

    for (int i = 0; i < _pendingConnections.size(); ++i)
    {
        const MasterConnection & c = _pendingConnections[i];
        NetworkConnection c2;

        c2.sourceName = processById(c.processA).description.name;
        c2.sourceInstance = processById(c.processA).instanceNumber;
        c2.sourcePort = c.portA;

        c2.destinationName = processById(c.processB).description.name;
        c2.destinationInstance = processById(c.processB).instanceNumber;
        c2.destinationPort = c.portB;

        n.connections.append(c2);
    }

    return n;
}

modulight::Network modulight::Application::currentNetwork() const
{
    Network n;

    for (int i = 0; i < _processes.size(); ++i)
    {
        if (!_processes[i].isFinished && !_processes[i].isAboutToBeFinished)
        {
            const Process & p = _processes[i];
            NetworkModuleDescription d;

            d.name = p.description.name;
            d.instance = p.instanceNumber;
            d.inputPorts = p.description.inputPorts;

            QMapIterator<QString, ModuleDescription::OutputPort> it(p.description.outputPorts);
            while (it.hasNext())
            {
                it.next();
                d.outputPorts.append(it.key());
            }

            n.modules.append(d);
        }
    }

    for (int i = 0; i < _currentConnections.size(); ++i)
    {
        const MasterConnection & c = _currentConnections[i];
        NetworkConnection c2;

        c2.sourceName = processById(c.processA).description.name;
        c2.sourceInstance = processById(c.processA).instanceNumber;
        c2.sourcePort = c.portA;

        c2.destinationName = processById(c.processB).description.name;
        c2.destinationInstance = processById(c.processB).instanceNumber;
        c2.destinationPort = c.portB;

        n.connections.append(c2);
    }

    return n;
}
