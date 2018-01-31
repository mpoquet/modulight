#include <modulight/module.hpp>

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include <QStringList>
#include <QDebug>
#include <QTime>
#include <QNetworkInterface>
#include <QHostAddress>

#include <modulight/common/mpiutils.hpp>
#include <modulight/common/xml.hpp>
#include <modulight/common/tag.hpp>
#include <modulight/common/modulightexception.hpp>
#include <modulight/module/stamp.hpp>

using namespace std;
using namespace modulight::mpi_util;
using namespace modulight;
using namespace zmq;

modulight::Module::Module(const QString &moduleName, int argc, char **argv) :
    _name(moduleName),
    _instanceNumber(-1),
    _launchWithoutEnvironment(false),
    _state(ModuleState::UNINITIALIZED),
    _context(1),
    _syncRep(_context, ZMQ_REP)
{
    static bool first = true;

    if (!first)
    {
        error() << "Warning : you instanciated several modules in one execution."
                << " Only the first one will be able to communicate with its environment" << endl;
    }
    else
    {
        first = false;

        MPI_Init(&argc, &argv);

        /*int thread_support;
        MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &thread_support);

        if (thread_support != MPI_THREAD_FUNNELED)
        {
            switch(thread_support)
            {
            case MPI_THREAD_SINGLE:
                cout << "Information : module " << _name.toStdString() << " is launched without thread support" << endl;
                break;
            case MPI_THREAD_SERIALIZED:
                cout << "Information : module " << _name.toStdString() << " is launched in Serialized thread support" << endl;
                break;
            case MPI_THREAD_MULTIPLE:
                cout << "Information : module " << _name.toStdString() << " is launched in Multiple thread support" << endl;
                break;
            default:
                error() << "Invalid thread support value" << endl;
                throw Exception("Invalid thread support value");
            }
        }
*/

        MPI_Comm_get_parent(&_parent);

        if (_parent == MPI_COMM_NULL)
        {
            cout << "Information : module " << _name.toStdString() << " is not launched within a Modulight environment" << endl;
            _launchWithoutEnvironment = true;
        }
        else
        {
            int size;
            MPI_Comm_remote_size(_parent, &size);

            if (size != 1)
            {
                error() << "Error : parent remote size is not 1" << endl;
                throw Exception("MPI_Comm_remote_size(parent,...) return was not 1");
            }
            else
            {
                QString arguments;

                // Part of Modulight protocol : let's receive process arguments
                recvQString(arguments, 0, tag::ARGUMENTS, _parent, MPI_STATUS_IGNORE);
                _arguments.loadFromXML(arguments);

                // Let's get the local IPv4 address
                QVector<QString> addresses;
                foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
                {
                    if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                         addresses.append(address.toString());
                }

                if (addresses.size() <= 0)
                    throw Exception("There is no available IPv4 address");
                else if (addresses.size() > 1)
                    error() << "Warning : many IPv4 adresses are available. The first one will be chosen (" << addresses.first().toStdString() << ")" << endl;

                _ip = addresses.first();
                //cout << "IP : " << _ip.toStdString() << endl;

                // Let's configure and bind the sync socket
                int hwm = 0; // (HWM == 0) => data loss forbidden

                _syncRep.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(int));
                _syncRep.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(int));
                _syncRep.bind("tcp://*:0");

                // Let's get on which port the sync socket had been bound
                size_t bufSize = 100;
                char buf[bufSize];
                _syncRep.getsockopt(ZMQ_LAST_ENDPOINT, &buf, &bufSize);
                QRegExp regex("tcp://.*:(\\d{1,5})");

                if (!regex.exactMatch(QString(buf)))
                    throw Exception(QString("Regex failed in parsing bound port in \"%1\"").arg(QString(buf)));

                _syncRepPort = regex.cap(1).toInt();
                //cout << "Sync port : " << _syncRepPort << " from " << buf << endl;
            }
        }
    }
}

modulight::Module::~Module()
{
    QMapIterator<QString, InputPort> itIn(_inputPorts);
    while (itIn.hasNext())
    {
        itIn.next();

        delete itIn.value().sub;
        delete itIn.value().req;
    }

    QMapIterator<QString, OutputPort> itOut(_outputPorts);
    while (itOut.hasNext())
    {
        itOut.next();

        delete itOut.value().pub;
        delete itOut.value().rep;
    }

    if (_state != ModuleState::FINALIZED)
        finalize();

    int isMPIFinalized = 0;
    MPI_Finalized(&isMPIFinalized);

    if (!isMPIFinalized)
        MPI_Finalize();

    _inputPorts.clear();
    _outputPorts.clear();
}

bool modulight::Module::initialize()
{
    if (_state != ModuleState::UNINITIALIZED)
    {
        error() << "Invalid Module::initialize call : the Module has already been initialized" << endl;
        return false;
    }

    if (_launchWithoutEnvironment)
    {
        _state = ModuleState::RUNNING_WITHOUT_ENVIRONMENT;
        return true;
    }
    else
    {
        sendDescription();
        receiveInstanceInformation();

        createPollItems();
        fillCompletePortNames();

        if (receiveOrders())
        {
            _state = ModuleState::RUNNING;
            return true;
        }
        else
        {
            _state = ModuleState::LAUNCH_ABORTED;
            return false;
        }
    }
}

void modulight::Module::finalize()
{
    if (_state == ModuleState::RUNNING)
    {
        int i = 42;
        MPI_Request request;

        MPI_Isend(&i, 1, MPI_INT, 0, tag::MODULE_FINISHED, _parent, &request);

        bool canExit = false;
        while (!canExit)
        {
            int exitConfirmationReceived;
            MPI_Iprobe(0, tag::FINISH_ORDER, _parent, &exitConfirmationReceived, MPI_STATUS_IGNORE);

            if (exitConfirmationReceived)
                canExit = true;
            else
            {
                if (!checkDynamicOrders())
                    usleep(50 * 1000);
            }
        }

        MPI_Recv(&i, 1, MPI_INT, 0, tag::FINISH_ORDER, _parent, MPI_STATUS_IGNORE);
    }
    else if (_state == ModuleState::UNINITIALIZED)
        display() << "Going straight from Uninitialized to Finalized (have you forgot to call Module::initialize ?)" << endl;

    _state = ModuleState::FINALIZED;

    //display() << "Finalized" << endl;
}

bool modulight::Module::getNetwork(Network &network)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid getNetwork call : the process is not running" << endl;
        return false;
    }

    int i;
    MPI_Ssend(&i, 0, MPI_INT, 0, tag::NETWORK_REQUEST, _parent);

    QString networkString;
    recvQString(networkString, 0, tag::NETWORK, _parent, MPI_STATUS_IGNORE);

    if (networkString.isEmpty())
        return false;

    xml::readNetwork(networkString, network);
    return true;
}

bool modulight::Module::addConnection(const QString & sourceName, int sourceInstance, const QString & sourcePort,
    const QString & destinationName, int destinationInstance, const QString & destinationPort,
    bool lossyConnection)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid addConnection call : the process is not running" << endl;
        return false;
    }

    DynamicRequest r;
    r.type = RequestType::ADD_CONNECTION;
    r.lossyConnection = lossyConnection;

    r.sourceName = sourceName;
    r.sourceInstance = sourceInstance;
    r.sourcePort = sourcePort;

    r.destinationName = destinationName;
    r.destinationInstance = destinationInstance;
    r.destinationPort = destinationPort;

    return sendAndReceiveRequest(r);
}

bool modulight::Module::removeConnection(const QString &sourceName, int sourceInstance, const QString &sourcePort, const QString &destinationName, int destinationInstance, const QString &destinationPort)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid removeConnection call : the process is not running" << endl;
        return false;
    }

    DynamicRequest r;
    r.type = RequestType::REMOVE_CONNECTION;

    r.sourceName = sourceName;
    r.sourceInstance = sourceInstance;
    r.sourcePort = sourcePort;

    r.destinationName = destinationName;
    r.destinationInstance = destinationInstance;
    r.destinationPort = destinationPort;

    return sendAndReceiveRequest(r);
}

bool modulight::Module::applyDynamicRequestSequence(modulight::DynamicRequestSequence &sequence)
{
    QString xmlString;

    xml::writeDynamicRequests(xmlString, sequence);

    sendQString(xmlString, 0, tag::DYNAMIC_REQUEST, _parent);

    int i;
    MPI_Recv(&i, 1, MPI_INT, 0, tag::REQUEST_RESULT, _parent, MPI_STATUS_IGNORE);

    return i != 0;
}

bool modulight::Module::spawnProcess(const QString &command, const ArgumentWriter &writer, const QString & hostname)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid spawnProcess call : the process is not running" << endl;
        return false;
    }

    DynamicRequest r;
    r.type = RequestType::ADD_MODULE;

    r.path = command;
    r.args = writer.map();
    r.host = hostname;

    return sendAndReceiveRequest(r);
}

bool modulight::Module::killProcess(const QString &moduleName, int instance)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid killProcess call : the process is not running" << endl;
        return false;
    }

    DynamicRequest r;
    r.type = RequestType::REMOVE_MODULE;

    r.moduleName = moduleName;
    r.moduleInstance = instance;

    return sendAndReceiveRequest(r);
}

ostream & modulight::Module::display() const
{
    cout << _name.toStdString() << _instanceNumber << ' ';

    return cout;
}

ostream & modulight::Module::error() const
{
    cerr << _name.toStdString() << _instanceNumber << ' ';

    return cerr;
}

bool modulight::Module::checkDynamicOrders()
{
    //QTime time;
    //time.start();

    int receivedFromMaster;
    MPI_Iprobe(0, tag::DYNAMIC_ORDER, _parent, &receivedFromMaster, MPI_STATUS_IGNORE);

    if (receivedFromMaster)
    {
        handleDynamicOrders();
        //display() << "Dynamic order done, took " << time.elapsed() << " ms" << endl;
        return true;
    }

    return false;
}

void modulight::Module::handleDynamicOrders()
{
    QString xmlString;
    DynamicOrderSequence sequence;

    recvQString(xmlString, 0, tag::DYNAMIC_ORDER, _parent, MPI_STATUS_IGNORE);

    xml::readDynamicOrders(xmlString, sequence);

    bool destroyOrderReceived = false;

    for (int i = 0; i < sequence.orders.size(); ++i)
    {
        const DynamicOrder & o = sequence.orders[i];

        switch(o.type)
        {
        case OrderType::ACCEPT:
            handleDynamicAccept(o);
            break;
        case OrderType::CONNECT:
            handleDynamicConnect(o);
            break;
        case OrderType::INPUT_DISCONNECT:
            handleDynamicInputDisconnect(o);
            break;
        case OrderType::OUTPUT_DISCONNECT:
            handleDynamicOutputDisconnect(o);
            break;
        case OrderType::DESTROY:
            handleDynamicDestroy();
            destroyOrderReceived = true;
            break;
        default:
            display() << "Invalid DynamicOrder received (" << (int)o.type << ')' << endl;
            break;
        }
    }

    int i;
    MPI_Send(&i, 0, MPI_INT, 0, tag::ORDER_DONE, _parent);

    // To avoid a deadlock between the destroyed process and the master
    // ORDER_DONE must be sent before MODULE_FINISH (which is done by finalize)
    if (destroyOrderReceived)
        finalize();
}

void modulight::Module::handleDynamicAccept(const DynamicOrder &o)
{
    if (o.lossyConnection)
        _outputPorts[o.localPortName].lossyRemotes[o.remoteAbbrevName] = true;
    else
        _outputPorts[o.localPortName].losslessRemotes.append(o.remoteAbbrevName);

    message_t msg;
    _syncRep.recv(&msg);
    _syncRep.send(msg);
}

void modulight::Module::handleDynamicConnect(const DynamicOrder &o)
{
    QString remote = QString("tcp://%1:%2").arg(o.remoteIP).arg(o.remotePort);
    QByteArray qbaRemote = remote.toUtf8();

    /*display() << "Connecting " << o.localPortName.toStdString() << " to "
              << qbaRemote.data() << endl;*/

    if (o.lossyConnection)
    {
        _inputPorts[o.localPortName].req->connect(qbaRemote.data());
        _inputPorts[o.localPortName].lossyRemotes.append(o.remoteAbbrevName);
    }
    else
    {
        _inputPorts[o.localPortName].sub->connect(qbaRemote.data());
        _inputPorts[o.localPortName].losslessRemotes.append(o.remoteAbbrevName);
    }

    socket_t req(_context, ZMQ_REQ);
    int hwm = 0;
    req.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(int));
    req.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(int));

    QByteArray qbaSync = QString("tcp://%1:%2").arg(o.remoteIP).arg(o.syncPort).toUtf8();
    req.connect(qbaSync.data());

    message_t msg;
    req.send(msg);
    req.recv(&msg);
}

void modulight::Module::handleDynamicOutputDisconnect(const DynamicOrder &o)
{
    /*display() << "Disconnecting " << o.localPortName.toStdString() << " to "
              << o.remoteAbbrevName.toStdString() << endl;*/

    if (o.lossyConnection)
        _outputPorts[o.localPortName].lossyRemotes.remove(o.remoteAbbrevName);
    else
        _outputPorts[o.localPortName].losslessRemotes.removeAll(o.remoteAbbrevName);

    message_t msg;
    _syncRep.recv(&msg);
    _syncRep.send(msg);
}

void modulight::Module::handleDynamicInputDisconnect(const DynamicOrder &o)
{
    QString remote = QString("tcp://%1:%2").arg(o.remoteIP).arg(o.remotePort);
    QByteArray qbaRemote = remote.toUtf8();

    /*display() << "Disconnecting " << o.localPortName.toStdString() << " from "
              << o.remoteAbbrevName.toStdString() << endl;*/

    if (o.lossyConnection)
    {
        zmq_disconnect(*_inputPorts[o.localPortName].req, qbaRemote.data());
        _inputPorts[o.localPortName].lossyRemotes.removeAll(o.remoteAbbrevName);
    }
    else
    {
        zmq_disconnect(*_inputPorts[o.localPortName].sub, qbaRemote.data());
        _inputPorts[o.localPortName].losslessRemotes.removeAll(o.remoteAbbrevName);
    }

    socket_t req(_context, ZMQ_REQ);
    int hwm = 0;
    req.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(int));
    req.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(int));

    QByteArray qbaSync = QString("tcp://%1:%2").arg(o.remoteIP).arg(o.syncPort).toUtf8();
    req.connect(qbaSync.data());

    message_t msg;
    req.send(msg);
    req.recv(&msg);
}

void modulight::Module::handleDynamicDestroy()
{
    QString cc = currentConnectionsToString();

    if (!cc.isEmpty())
        display() << "Destroy order reveived, but there are connections left ("
                  << cc.toStdString() << ")" << endl;
    /*else
        display() << "Destroy order received" << endl;*/
}

bool modulight::Module::sendAndReceiveRequest(const DynamicRequest &r)
{
    DynamicRequestSequence sequence;
    sequence.requests.append(r);

    QString xmlString;

    xml::writeDynamicRequests(xmlString, sequence);

    sendQString(xmlString, 0, tag::DYNAMIC_REQUEST, _parent);

    int i;
    MPI_Recv(&i, 1, MPI_INT, 0, tag::REQUEST_RESULT, _parent, MPI_STATUS_IGNORE);

    return i != 0;
}

const modulight::ArgumentReader & modulight::Module::arguments() const
{
    return _arguments;
}

void modulight::Module::addInputPort(const QString &name)
{
    if (_state == ModuleState::UNINITIALIZED)
    {
        if (_launchWithoutEnvironment)
            return;

        if (_inputPorts.contains(name))
        {
            error() << "Invalid addInputPort call : the input port \"" << name.toStdString() << "\" already exists" << endl;
            return;
        }
        else if (_outputPorts.contains(name))
        {
            error() << "Invalid addInputPort call : the port \"" << name.toStdString() << "\" already exists as an output one";
            return;
        }

        InputPort ip;
        int hwm0 = 0;

        ip.sub = new zmq::socket_t(_context, ZMQ_SUB);
        ip.sub->setsockopt(ZMQ_RCVHWM, &hwm0, sizeof(int));
        ip.sub->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        ip.req = new zmq::socket_t(_context, ZMQ_REQ);

        _inputPorts[name] = ip;
    }
    else
        error() << "Invalid addInputPort call : the process has already been initialized" << endl;
}

void modulight::Module::addOutputPort(const QString &name)
{
    if (_state == ModuleState::UNINITIALIZED)
    {
        if (_launchWithoutEnvironment)
            return;

        if (_outputPorts.contains(name))
        {
            error() << "Invalid addOutputPort call : the output port \"" << name.toStdString() << "\" already exists" << endl;
            return;
        }
        else if (_inputPorts.contains(name))
        {
            error() << "Invalid addOutputPort call : the port \"" << name.toStdString() << "\" already exists as an input one";
            return;
        }

        OutputPort op;
        int hwm0 = 0;

        op.pub = new zmq::socket_t(_context, ZMQ_PUB);
        op.pub->setsockopt(ZMQ_SNDHWM, &hwm0, sizeof(int));
        op.pub->bind("tcp://*:0");

        op.rep = new zmq::socket_t(_context, ZMQ_REP);
        op.rep->bind("tcp://*:0");

        QRegExp regex("tcp://.*:(\\d{1,5})");
        size_t bufSize = 100;
        char buf[bufSize];

        op.pub->getsockopt(ZMQ_LAST_ENDPOINT, &buf, &bufSize);
        if (!regex.exactMatch(QString(buf)))
            throw modulight::Exception(QString("Regex failed in parsing bound port in \"%1\"").arg(QString(buf)));
        op.pubPort = regex.cap(1).toInt();

        op.rep->getsockopt(ZMQ_LAST_ENDPOINT, &buf, &bufSize);
        if (!regex.exactMatch(QString(buf)))
            throw modulight::Exception(QString("Regex failed in parsing bound port in \"%1\"").arg(QString(buf)));
        op.repPort = regex.cap(1).toInt();

        _outputPorts[name] = op;
    }
    else
        error() << "Invalid addOutputPort call : the process had already been initialized" << endl;
}

bool modulight::Module::readMessage(const QString &iport, MessageReader &reader)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid readMessage call : the process is not running" << endl;

        return false;
    }

    if (!_inputPorts.contains(iport))
    {
        error() << "Invalid readMessage call, no such input port (" << iport.toStdString() << ')' << endl;
        return false;
    }

    message_t msg;
    QString source;
    int moduleIteration;
    int portIteration;
    bool isReal;

    if (_inputPorts[iport].messageAvailableOnLossy)
    {
        try
        {
            _inputPorts[iport].req->recv(&msg);
            Stamp::extractUsefulInformationFromData((char*)msg.data(), moduleIteration, portIteration, isReal, source);

            _inputPorts[iport].req->recv(&msg);
            _inputPorts[iport].messageAvailableOnLossy = false;
        }
        catch(zmq::error_t &)
        {
            display() << "Exception caught in readMessage : bad recv call (bad REQ state)" << endl;
            return false;
        }
    }
    else if (_inputPorts[iport].messageAvailableOnLossless)
    {
        _inputPorts[iport].sub->recv(&msg);
        Stamp::extractUsefulInformationFromData((char*)msg.data(), moduleIteration, portIteration, isReal, source);

        _inputPorts[iport].sub->recv(&msg);
        _inputPorts[iport].messageAvailableOnLossless = false;
    }
    else
        return false;

    if (isReal)
    {
        reader.clear();
        reader.load((char*)msg.data(), msg.size(), iport, source, moduleIteration, portIteration);

        return true;
    }
    else
        return false;
}

bool modulight::Module::readMessage(const QString &iport, char *data, unsigned int size)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid readMessage call : the process is not running" << endl;

        return false;
    }

    if (!_inputPorts.contains(iport))
    {
        error() << "Invalid readMessage call, no such input port (" << iport.toStdString() << ')' << endl;
        return false;
    }

     message_t msg;
     QString source;
     int moduleIteration;
     int portIteration;
     bool isReal;

    if (_inputPorts[iport].messageAvailableOnLossy)
    {
        try
        {
            _inputPorts[iport].req->recv(&msg);
            Stamp::extractUsefulInformationFromData((char*)msg.data(), moduleIteration, portIteration, isReal, source);

            if (isReal)
                _inputPorts[iport].req->recv(data, size);
            else
                _inputPorts[iport].req->recv(&msg);

            _inputPorts[iport].messageAvailableOnLossy = false;
        }
        catch(zmq::error_t &)
        {
            display() << "Exception caught in readMessage : bad recv call (bad REQ state)" << endl;
            return false;
        }
    }
    else if (_inputPorts[iport].messageAvailableOnLossless)
    {
        _inputPorts[iport].sub->recv(&msg);
        Stamp::extractUsefulInformationFromData((char*)msg.data(), moduleIteration, portIteration, isReal, source);

        if (isReal)
            _inputPorts[iport].sub->recv(data, size);
        else
            _inputPorts[iport].sub->recv(&msg);

        _inputPorts[iport].messageAvailableOnLossless = false;
    }
    else
        return false;

    if (isReal)
        return true;
    else
        return false;
}

void modulight::Module::updateMessageAvailability()
{
    checkDynamicOrders();

    zmq_poll(_pollItems.data(), _pollItems.size(), 0);

    QMutableMapIterator<QString, InputPort> it(_inputPorts);
    for (int i = 0; it.hasNext(); ++i)
    {
        it.next();

        if (_pollItems[i*2  ].revents & ZMQ_POLLIN)
            it.value().messageAvailableOnLossless = true;
        if (_pollItems[i*2+1].revents & ZMQ_POLLIN)
            it.value().messageAvailableOnLossy = true;
    }
}

void modulight::Module::handleOnRequestSends()
{
    message_t msg;

    QMapIterator<QString, InputPort> itIn(_inputPorts);
    while (itIn.hasNext())
    {
        itIn.next();

        if (!itIn.value().lossyRemotes.isEmpty())
            zmq_send(*itIn.value().req, itIn.value().completePortName.data(), itIn.value().completePortName.size(), ZMQ_DONTWAIT);
    }

    QMutableMapIterator<QString, OutputPort> itOut(_outputPorts);
    while (itOut.hasNext())
    {
        itOut.next();

        int lossyRemoteSize = itOut.value().lossyRemotes.size();

        for (int i = 0; i < lossyRemoteSize && itOut.value().rep->recv(&msg, ZMQ_DONTWAIT); ++i)
        {
            QString remote = QString::fromUtf8((char*)msg.data(), msg.size());

            if (!itOut.value().lossyRemotes.contains(remote))
            {
                error() << "Critical coherence error within modulight : request from an unknown source received (" << remote.toStdString() << ')' << endl;
                qDebug() << "Lossy : " << itOut.value().lossyRemotes;
                qDebug() << "Lossless : " << itOut.value().losslessRemotes;
            }
            else
            {
                if (!itOut.value().lossyRemotes[remote])
                {
                    itOut.value().rep->send(itOut.value().lastStamp.data(),
                                            itOut.value().lastStamp.size(),
                                            ZMQ_SNDMORE);

                    itOut.value().rep->send(itOut.value().lastMessageBuffer.data(),
                                            itOut.value().lastMessageBuffer.size());

                    itOut.value().lossyRemotes[remote] = true;
                }
                else
                {
                    Stamp fakeStamp(false, &itOut.value().completePortName,
                                    itOut.value().lastStamp.moduleIteration(),
                                    itOut.value().lastStamp.portIteration());

                    itOut.value().rep->send(fakeStamp.data(), fakeStamp.size(), ZMQ_SNDMORE);
                    itOut.value().rep->send(0, 0);
                }
            }
        }
    }
}

QString modulight::Module::currentConnectionsToString()
{
    QStringList connections;

    QMapIterator<QString, InputPort> itIn(_inputPorts);
    while (itIn.hasNext())
    {
        itIn.next();

        if (!itIn.value().losslessRemotes.isEmpty())
            connections.append(QString("L|%1<-(%2)").arg(itIn.key(), itIn.value().losslessRemotes.join(",")));

        if (!itIn.value().lossyRemotes.isEmpty())
            connections.append(QString("l|%1<-(%2)").arg(itIn.key(), itIn.value().lossyRemotes.join(",")));
    }

    QMapIterator<QString, OutputPort> itOut(_outputPorts);
    while (itOut.hasNext())
    {
        itOut.next();

        if (!itOut.value().losslessRemotes.isEmpty())
            connections.append(QString("L|%1->(%2)").arg(itOut.key(), itOut.value().losslessRemotes.join(",")));

        if (!itOut.value().lossyRemotes.isEmpty())
            connections.append(QString("l|%1->(%2)").arg(itOut.key(), QStringList(itOut.value().lossyRemotes.keys()).join(",")));
    }

    return connections.join(",");
}

void modulight::Module::wait()
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid wait call : the process is not running" << endl;
        return;
    }

    ++_iterationNumber;

    handleOnRequestSends();
    updateMessageAvailability();
}

bool modulight::Module::wait(const QString &iport, int msToWait, int msToSleep)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid wait call : the process is not running" << endl;
        return false;
    }

    if (!_inputPorts.contains(iport))
    {
        error() << "Invalid wait call, no such input port (" << iport.toStdString() << ')' << endl;
        return false;
    }

    // Converting ms to µs
    if (msToSleep > 0)
        msToSleep *= 1000;
    else if (msToSleep < 0)
        msToSleep = 1;

    ++_iterationNumber;

    if (msToWait <= -1)
    {
        while (true)
        {
            handleOnRequestSends();
            updateMessageAvailability();

            if (_inputPorts[iport].messageAvailableOnLossless || _inputPorts[iport].messageAvailableOnLossy)
                return true;

            if (msToSleep != 0)
                usleep(msToSleep);
        }
    }
    else if (msToWait == 0)
    {
        handleOnRequestSends();
        updateMessageAvailability();

        if (_inputPorts[iport].messageAvailableOnLossless || _inputPorts[iport].messageAvailableOnLossy)
            return true;
    }
    else
    {
        QTime time;
        time.start();

        while (time.elapsed() < msToWait)
        {
            handleOnRequestSends();
            updateMessageAvailability();

            if (_inputPorts[iport].messageAvailableOnLossless || _inputPorts[iport].messageAvailableOnLossy)
                return true;

            if (msToSleep != 0)
                usleep(msToSleep);
        }
    }

    return false;
}

bool modulight::Module::wait(const QList<QStringList> &iports, int msToWait, int msToSleep)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid wait call : the process is not running" << endl;
        return false;
    }

    for (int i = 0; i < iports.size(); ++i)
    {
        for (int j = 0; j < iports[i].size(); ++j)
        {
            if (!_inputPorts.contains(iports[i][j]))
            {
                error() << "Invalid wait call, no such input port (" << iports[i][j].toStdString() << ')' << endl;
                return false;
            }
        }
    }

    // Converting ms to µs
    if (msToSleep > 0)
        msToSleep *= 1000;
    else if (msToSleep < 0)
        msToSleep = 1;

    ++_iterationNumber;

    if (msToWait <= -1)
    {
        while (true)
        {
            handleOnRequestSends();
            updateMessageAvailability();

            for (int i = 0; i < iports.size(); ++i)
            {
                bool ok = true;
                for (int j = 0; j < iports[i].size(); ++j)
                {
                    if (!_inputPorts[iports[i][j]].messageAvailableOnLossless)
                    {
                        ok = false;
                        break;
                    }
                }

                if (ok)
                    return true;
            }

            if (msToSleep != 0)
                usleep(msToSleep);
        }
    }
    else if (msToWait == 0)
    {
        handleOnRequestSends();
        updateMessageAvailability();

        for (int i = 0; i < iports.size(); ++i)
        {
            bool ok = true;
            for (int j = 0; j < iports[i].size(); ++j)
            {
                if (!_inputPorts[iports[i][j]].messageAvailableOnLossless)
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
                return true;
        }
    }
    else
    {
        QTime time;
        time.start();

        while (time.elapsed() < msToWait)
        {
            handleOnRequestSends();
            updateMessageAvailability();

            for (int i = 0; i < iports.size(); ++i)
            {
                bool ok = true;
                for (int j = 0; j < iports[i].size(); ++j)
                {
                    if (!_inputPorts[iports[i][j]].messageAvailableOnLossless)
                    {
                        ok = false;
                        break;
                    }
                }

                if (ok)
                    return true;
            }

            if (msToSleep != 0)
                usleep(msToSleep);
        }
    }

    return false;
}

bool modulight::Module::messageAvailable(const QString & iport)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid messageAvailable call : the process is not running" << endl;
        return false;
    }

    if (!_inputPorts.contains(iport))
    {
        error() << "Invalid messageAvailable call : no such port (" << iport.toStdString() << ")" << endl;
        return false;
    }

    return _inputPorts[iport].messageAvailableOnLossless || _inputPorts[iport].messageAvailableOnLossy;
}

bool modulight::Module::messageAvailable(const QStringList &iports)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid messageAvailable call : the process is not running" << endl;
        return false;
    }

    for (int i = 0; i < iports.size(); ++i)
    {
        if (!_inputPorts.contains(iports[i]))
        {
            error() << "Invalid messageAvailable call : no such port (" << iports[i].toStdString() << ")" << endl;
            return false;
        }

        if (!_inputPorts[iports[i]].messageAvailableOnLossless && !_inputPorts[iports[i]].messageAvailableOnLossy)
            return false;
    }

    return true;
}

QStringList modulight::Module::portSources(const QString &port)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid portSources call : the process is not running" << endl;
        return QStringList();
    }

    return _inputPorts[port].losslessRemotes + _inputPorts[port].lossyRemotes;
}

MPI_Comm modulight::Module::mpiWorld() const
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid mpiWorld call : the process is not running" << endl;
        return MPI_COMM_WORLD;
    }

    return _world;
}

void modulight::Module::send(const QString &port, const MessageWriter &writer)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid send call : the process is not running" << endl;
        return;
    }

    if (_outputPorts.contains(port))
    {
        ++_outputPorts[port].iterationNumber;

        Stamp stamp(true, &_outputPorts[port].completePortName, _iterationNumber,
                    _outputPorts[port].iterationNumber);

        _outputPorts[port].pub->send(stamp.data(), stamp.size(), ZMQ_SNDMORE);
        _outputPorts[port].pub->send(writer.data(), writer.size());

        if (!_outputPorts[port].lossyRemotes.isEmpty())
        {
            _outputPorts[port].lastMessageBuffer.resize(writer.size());
            memcpy(_outputPorts[port].lastMessageBuffer.data(), writer.data(), writer.size());

            _outputPorts[port].lastStamp = stamp;

            QMutableMapIterator<QString, bool> it(_outputPorts[port].lossyRemotes);
            while (it.hasNext())
            {
                it.next();
                it.value() = false;
            }
        }
    }
    else
    {
        error() << "Invalid send call : no such port ("
                << port.toStdString() << ")" << endl;
    }
}

void modulight::Module::send(const QString &port, const char *data, unsigned int size)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid send call : the process is not running" << endl;
        return;
    }

    if (_outputPorts.contains(port))
    {
        ++_outputPorts[port].iterationNumber;

        Stamp stamp(true, &_outputPorts[port].completePortName, _iterationNumber,
                    _outputPorts[port].iterationNumber);
        _outputPorts[port].pub->send(stamp.data(), stamp.size(), ZMQ_SNDMORE);
        _outputPorts[port].pub->send(data, size);

        if (!_outputPorts[port].lossyRemotes.isEmpty())
        {
            _outputPorts[port].lastMessageBuffer.resize(size);
            memcpy(_outputPorts[port].lastMessageBuffer.data(), data, size);

            _outputPorts[port].lastStamp = stamp;

            QMutableMapIterator<QString, bool> it(_outputPorts[port].lossyRemotes);
            while (it.hasNext())
            {
                it.next();
                it.value() = false;
            }
        }
    }
    else
    {
        error() << "Invalid send call : no such port ("
                << port.toStdString() << ")" << endl;
    }
}

bool modulight::Module::isInputPortConnected(const QString &iport)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid isInputPortConnected call : the process is not running" << endl;
        return false;
    }

    checkDynamicOrders();

    if (!_inputPorts.contains(iport))
    {
        error() << "Invalid isInputPortConnected call : no such port ("
                << iport.toStdString() << ")" << endl;

        return false;
    }

    return !_inputPorts[iport].losslessRemotes.isEmpty() || !_inputPorts[iport].lossyRemotes.isEmpty();
}

bool modulight::Module::isOutputPortConnected(const QString &oport)
{
    if (_state != ModuleState::RUNNING)
    {
        if (_state != ModuleState::RUNNING_WITHOUT_ENVIRONMENT)
            error() << "Invalid isOutputPortConnected call : the process is not running" << endl;
        return false;
    }

    checkDynamicOrders();

    if (!_outputPorts.contains(oport))
    {
        error() << "Invalid isOutputPortConnected call : no such port ("
                << oport.toStdString() << ")" << endl;

        return false;
    }

    return !_outputPorts[oport].losslessRemotes.isEmpty() || !_outputPorts[oport].lossyRemotes.isEmpty();
}

QString modulight::Module::xmlDescription() const
{
    ModuleDescription description;

    description.name = _name;
    description.ip = _ip;
    description.syncPort = _syncRepPort;

    QMapIterator<QString, InputPort> itIn(_inputPorts);
    while (itIn.hasNext())
    {
        itIn.next();
        description.inputPorts.append(itIn.key());
    }

    QMapIterator<QString, OutputPort> itOut(_outputPorts);
    while (itOut.hasNext())
    {
        itOut.next();

        ModuleDescription::OutputPort o;
        o.losslessPort = itOut.value().pubPort;
        o.lossyPort = itOut.value().repPort;
        description.outputPorts[itOut.key()] = o;
    }

    QString s;

    xml::writeModuleDescription(s, description);

    return s;
}

void modulight::Module::sendDescription()
{
    QString xml = xmlDescription();

    sendQString(xml, 0, tag::MODULE_DESCRIPTION, _parent);
}

void modulight::Module::receiveInstanceInformation()
{
    MPI_Recv(&_instanceNumber, 1, MPI_INT, 0, tag::INSTANCE_INFO, _parent, MPI_STATUS_IGNORE);
}

void modulight::Module::createPollItems()
{
    _pollItems.resize(_inputPorts.size() * 2);

    QMutableMapIterator<QString, InputPort> it(_inputPorts);
    for (int i = 0; it.hasNext(); ++i)
    {
        it.next();

        _pollItems[2*i  ].socket = *it.value().sub;
        _pollItems[2*i  ].events = ZMQ_POLLIN;

        _pollItems[2*i+1].socket = *it.value().req;
        _pollItems[2*i+1].events = ZMQ_POLLIN;

        it.value().messageAvailableOnLossless = false;
        it.value().messageAvailableOnLossy = false;
    }
}

void modulight::Module::fillCompletePortNames()
{
    QMutableMapIterator<QString, InputPort> itIn(_inputPorts);
    while (itIn.hasNext())
    {
        itIn.next();

        itIn.value().completePortName = QString("%1%2:%3").arg(_name).arg(_instanceNumber).arg(itIn.key()).toUtf8();
    }

    QMutableMapIterator<QString, OutputPort> itOut(_outputPorts);
    while (itOut.hasNext())
    {
        itOut.next();

        itOut.value().completePortName = QString("%1%2:%3").arg(_name).arg(_instanceNumber).arg(itOut.key()).toUtf8();
    }
}

bool modulight::Module::receiveOrders()
{
    MPI_Status status;
    bool startReceived = false;

    do
    {
        MPI_Probe(0, MPI_ANY_TAG, _parent, &status);

        if ((tag::Tag) status.MPI_TAG == tag::START_ORDER)
        {
            int i;
            MPI_Recv(&i, 1, MPI_INT, 0, tag::START_ORDER, _parent, &status);
            startReceived = true;
        }
        else if ((tag::Tag) status.MPI_TAG == tag::SEQUENCE)
        {
            QString xmlString;

            int bufSize;
            MPI_Get_count(&status, MPI_CHAR, &bufSize);

            recvQStringNoProbe(xmlString, bufSize, 0, tag::SEQUENCE, _parent, &status);

            Sequence seq;
            xml::readSequence(xmlString, seq);

            handleSequence(seq);
        }
        else if ((tag::Tag) status.MPI_TAG == tag::ABORT_LAUNCH)
        {
            int i;
            MPI_Recv(&i, 1, MPI_INT, 0, tag::ABORT_LAUNCH, _parent, &status);
            return false;
        }
        else if ((tag::Tag) status.MPI_TAG == tag::SPLIT)
        {
            int buf[2];
            MPI_Recv(buf, 2, MPI_INT, 0, tag::SPLIT, _parent, &status);
            MPI_Comm_split(_parent, buf[0], buf[1], &_world);
        }
        else
        {
            error() << "Module invalid tag : " << status.MPI_TAG << endl;
            throw Exception("Error in receive orders : invalid tag");
        }
    } while (!startReceived);

    return true;
}

void modulight::Module::handleSequence(const Sequence & seq)
{
    const QVector<Connection> & c = seq.connections;

    // Let's do all connections
    for (int i = 0; i < c.size(); ++i)
    {
        if (c[i].isConnect)
        {
            QString remote = QString("tcp://%1:%2").arg(c[i].remoteIP).arg(c[i].remotePort);
            QByteArray qbaRemote = remote.toUtf8();

            //display() << "Connecting " << c[i].localPortName.toStdString() << " to " << qbaRemote.data() << endl;

            if (c[i].isLossy)
            {
                _inputPorts[c[i].localPortName].req->connect(qbaRemote.data());
                _inputPorts[c[i].localPortName].lossyRemotes.append(c[i].remoteAbbrevName);
            }
            else
            {
                _inputPorts[c[i].localPortName].sub->connect(qbaRemote.data());
                _inputPorts[c[i].localPortName].losslessRemotes.append(c[i].remoteAbbrevName);
            }

            /*socket_t req(_context, ZMQ_REQ);

            QByteArray qbaSync = QString("tcp://%1:%2").arg(c[i].remoteIP).arg(c[i].syncPort).toUtf8();
            req.connect(qbaSync.data());

            message_t msg;
            req.send(msg);
            req.recv(&msg);*/
        }
        else
        {
            if (c[i].isLossy)
                _outputPorts[c[i].localPortName].lossyRemotes[c[i].remoteAbbrevName] = true;
            else
                _outputPorts[c[i].localPortName].losslessRemotes.append(c[i].remoteAbbrevName);

            /*message_t msg;
            _syncRep.recv(&msg);
            _syncRep.send(msg);*/
        }
    }

    int i = 0;
    MPI_Send(&i, 1, MPI_INT, 0, tag::SEQUENCE_DONE, _parent);
}
