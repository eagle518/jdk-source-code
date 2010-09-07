/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <vector>
#include <set>

#include "jqs_api_server.hpp"
#include "sockets.hpp"
#include "thread.hpp"
#include "os_defs.hpp"
#include "utils.hpp"
#include "service.hpp"
#include "prefetch.hpp"

using namespace std;

/*
 * Helper class, which is used to parse incoming data stream.
 */
class JQSAPIMessageReader {
    /*
     * The reader does NOT take ownership of the connection object.
     */
    Connection* connection;
    vector<char> buf;

    /*
     * Ensures that the buffer contains required number of bytes,
     * by reading from connection.
     * @throws SocketException
     * @throws InterruptedException
     * @throws TimeoutException
     * @throws SocketClosedException
     */
    void fillBuffer (size_t requiredDataLen) {
        while (buf.size () < requiredDataLen) {
            connection->wait(JQS_API_TIMEOUT);
            connection->receiveMessage(buf);
        }
    }

public:
    /*
     * The reader does NOT take ownership of the connection object.
     */
    JQSAPIMessageReader (Connection* connection)
        : connection (connection)
    {}
    
    /*
     * @throws SocketException
     * @throws InterruptedException
     * @throws TimeoutException
     * @throws SocketClosedException
     */
    uint32_t getUint32 () {
        uint32_t v = 0;

        fillBuffer (sizeof (v));

        // big-endian order
        for (int i = 0; i < sizeof (v); ++i) {
            v <<= 8;
            v |= buf[i];
        }
        buf.erase (buf.begin(), buf.begin() + sizeof (v));
        return v;
    }

};


//////////////////////////////////////////////////////////////////////////

class ConnectionThread;

/*
 * The server thread. Opens the server socket, waits for the incoming 
 * connections and manages the ConnectionThread objects created for 
 * each established connection.
 */
class ServerThread : public Thread {
    /*
     * The server socket which is used for listening and accepting
     * incoming connections.
     */
    ServerSocket* serv;

    /*
     * Synchronizes accesses to the server thread data.
     */
    CriticalSection connectionLock;

    typedef set<ConnectionThread*> Connections;
    
    /*
     * The list of active connections.
     */
    Connections activeConnections;
    /*
     * The list of connection objects which have finished communication.
     */
    Connections finishedConnections;

    /*
     * The flag indicating whether the server thread should stop.
     */
    volatile bool stopping;

    /*
     * Creates new connection thread for given connection, starts the thread and 
     * adds it to the set of active connections
     */
    void registerNewConnection(Connection* connection);

    /*
     * Ensures that finished connection threads are really finished and deletes 
     * connection thread objects.
     */
    void deleteFinishedConnections();

public:
    ServerThread() 
        : serv(NULL)
        , stopping(false)
    {}

    virtual ~ServerThread() {
        delete serv;
    }

    /*
     * Server thread body.
     */
    virtual void run();

    /*
     * Requests the server thread to stop.
     */
    void stop();

    /*
     * Connection threads use this method to notify server thread that 
     * the connection thread is almost finished.
     */
    void connectionFinished(ConnectionThread* connThread);

};

/*
 * The connection thread. Communicates with the client, reads requests, 
 * sends responses. When the communication finishes, the connection
 * thread object notifies the server thread that it is finished.
 */
class ConnectionThread : public Thread {
    ServerThread* serverThread;
    Connection* connection;

    /*
     * @throws SocketException
     * @throws TimeoutException
     */
    void sendStringResponse(JQSAPIMessageKind response, const char* data);

    /*
     * @throws SocketException
     * @throws TimeoutException
     */
    void sendUInt32Response(JQSAPIMessageKind response, uint32_t data);

public:
    ConnectionThread(ServerThread* serverThread_, Connection* conn)
        : connection(conn)
        , serverThread(serverThread_)
    {}

    ~ConnectionThread() {
        delete connection;
    }

    /*
     * Connection thread body.
     */
    virtual void run();

    /*
     * Requests the connection thread to stop.
     */
    void stop();

};


//////////////////////////////////////////////////////////////////////////

/*
 * Creates new connection thread for given connection, starts the thread and 
 * adds it to the set of active connections
 */
void ServerThread::registerNewConnection(Connection* conn) {
    CriticalSection::Lock lock(connectionLock);
    if (stopping) {
        delete conn;
        return;
    }

    ConnectionThread* connThread = new ConnectionThread(this, conn);
    if (connThread->start()) {
        activeConnections.insert (connThread);
    } else {
        delete connThread;
    }
}

/*
 * Connection thread uses this method to notify server thread that 
 * the connection is finished
 */
void ServerThread::connectionFinished(ConnectionThread* connThread) {
    CriticalSection::Lock lock(connectionLock);

    size_t res = activeConnections.erase(connThread);
    assert (res == 1);

    finishedConnections.insert (connThread);
}

/*
 * Ensures that finished connection threads are really finished and deletes 
 * connection thread objects.
 */
void ServerThread::deleteFinishedConnections() {
    CriticalSection::Lock lock(connectionLock);

    for (Connections::iterator iter = finishedConnections.begin ();
         iter != finishedConnections.end (); ++iter)
    {
        ConnectionThread* connThread = *iter;
        connThread->waitToFinish();
        delete connThread;
    }
    finishedConnections.clear();
}

/*
 * Server thread body.
 */
void ServerThread::run() {
    try {
        serv = new ServerSocket(JQS_API_PORT);

        jqs_info (2, "JQS API Server started\n");

        while (!stopping) {
            deleteFinishedConnections();
            try {
                serv->wait(INFINITE_TIMEOUT);
                registerNewConnection(serv->accept());

            } catch (const InterruptedException&) {
                throw;

            } catch (const SocketException& e) {
                jqs_warn("%s (Socket error %d)\n", e.getMessage(), e.getSocketError());
            }
        }
    } catch (const InterruptedException&) {
        // stopping server

    } catch (const AddressAlreadyInUseException&) {
        jqs_error("Unable to create JQS API server: port %d is already used (JQS is already running?)\n", JQS_API_PORT);
    
    } catch (const SocketException& e) {
        jqs_error("Unable to create JQS API server: %s (Socket error %d)\n", e.getMessage(), e.getSocketError());
    }

    Connections active;
    {
        CriticalSection::Lock lock(connectionLock);
        active = activeConnections;
    }

    // ask all active connection threads to stop
    for (Connections::iterator iter = active.begin ();
         iter != active.end (); ++iter)
    {
        ConnectionThread* connThread = *iter;
        connThread->stop();
        connThread->waitToFinish();
    }

    deleteFinishedConnections();
    assert (activeConnections.empty () && finishedConnections.empty ());

    jqs_info (2, "JQS API Server stopped\n");
}

/*
 * Requests the server thread to stop.
 */
void ServerThread::stop() {
    CriticalSection::Lock lock(connectionLock);
    stopping = true;
    if (serv) {
        serv->interrupt();
    }
}


//////////////////////////////////////////////////////////////////////////

/*
 * @throws SocketException
 */
void ConnectionThread::sendStringResponse(JQSAPIMessageKind response, const char* data) {
    assert((response == JMK_JQSVersionResponse) || (response == JMK_JavaVersionResponse));

    JQSAPIMessage msg (response, (uint32_t)strlen(data));
    msg.append (data);
    connection->sendMessage(msg.getBytes ());
}

/*
 * @throws SocketException
 */
void ConnectionThread::sendUInt32Response(JQSAPIMessageKind response, uint32_t data) {
    assert(response == JMK_CapabilitiesResponse);

    JQSAPIMessage msg (response, sizeof(data));
    msg.append (data);
    connection->sendMessage(msg.getBytes ());
}

/*
 * Returns a capabilities bit vector filled according to JQS capabilities flags.
 */
static uint32_t getCapabilitiesBitVector() {
    uint32_t cap = 0;
    if(capabilityAdjustWorkingSetSize) {
        cap |= JCB_AdjustWorkingSetSize;
    }
    if(capabilityLockMemoryPages) {
        cap |= JCB_LockMemoryPages;
    }
    if(capabilityLargePages) {
        cap |= JCB_LargePages;
    }
    if(capabilityLowMemoryNotifications) {
        cap |= JCB_LowMemoryNotifications;
    }
    if(capabilitySetLibraryPath) {
        cap |= JCB_SetLibraryPath;
    }
    if(capabilityCheckPowerStatus) {
        cap |= JCB_CheckPowerStatus;
    }
    if(capabilityDeviceEventNotifications) {
        cap |= JCB_DeviceEventNotifications;
    }
    if(capabilityUserLogonNotifications) {
        cap |= JCB_UserLogonNotifications;
    }
    return cap;
}


/*
 * Connection thread body.
 */
void ConnectionThread::run() {
    JQSAPIMessageReader msgReader(connection);
    connection->setTimeout (JQS_API_TIMEOUT);
    jqs_info (4, "JQS API: new connection established\n");

    try {
        bool active = true;
        while (active) {
            uint32_t magic = msgReader.getUint32();
            if (magic != JQS_API_MESSAGE_MAGIC) {
                // non-JQS message found, closing communication
                jqs_warn ("JQS API: received message with incorrect magic %x (%x required)\n", magic, JQS_API_MESSAGE_MAGIC);
                break;
            }

            JQSAPIMessageKind kind = (JQSAPIMessageKind) msgReader.getUint32 ();

            uint32_t dataLen = msgReader.getUint32 ();
            if (dataLen != 0) {
                // invalid message length, closing communication
                jqs_warn ("JQS API: received message with incorrect data len %d (message kind %d)\n", dataLen, kind);
                break;
            }

            jqs_info (4, "JQS API: received message %d\n", kind);
            switch (kind) {
                case JMK_Pause:
                    pauseJQSService();
                    break;

                case JMK_Resume:
                    resumeJQSService();
                    break;

                case JMK_Notify:
                    notifyJQSService();
                    break;

                case JMK_JQSVersionRequest:
                    sendStringResponse (JMK_JQSVersionResponse, JQS_SERVICE_VERSION);
                    break;

                case JMK_JavaVersionRequest:
                    sendStringResponse (JMK_JavaVersionResponse, JQS_JAVA_VERSION);
                    break;

                case JMK_CapabilitiesRequest:
                    sendUInt32Response (JMK_CapabilitiesResponse, getCapabilitiesBitVector());
                    break;

                default:
                    // invalid message kind, closing communication
                    jqs_warn ("JQS API: received message with incorrect kind %d\n", kind);
                    active = false;
                    break;
            }
        }

    } catch (const InterruptedException&) {
        // stopping connection thread

    } catch (const SocketClosedException&) {
        // socket is closed

    } catch (const SocketException& e) {
        jqs_warn("Connection dropped due to error: %s (Socket error %d)\n", e.getMessage(), e.getSocketError());
    }

    jqs_info (4, "JQS API: connection dropped\n");
    assert (serverThread);
    serverThread->connectionFinished (this);
}

/*
 * Requests the connection thread to stop.
 */
void ConnectionThread::stop() {
    connection->interrupt();
}


//////////////////////////////////////////////////////////////////////////

/*
 * The JQS API server thread instance.
 */
static ServerThread* JQSAPIServerThread = NULL;

/*
 * Starts JQS API server thread which listens for the incoming connections
 * on JQS_API_PORT and starts the communication thread for each connection
 * occurred.
 * Returns true on success. All error messages are reported inside on failures.
 */
bool startJQSAPIServer() {
    assert (!JQSAPIServerThread);

    JQSAPIServerThread = new ServerThread();
    return JQSAPIServerThread->start();
}

/*
 * Stops all active communication threads and the server thread.
 */
void stopJQSAPIServer() {
    JQSAPIServerThread->stop();
    JQSAPIServerThread->waitToFinish();
    delete JQSAPIServerThread;
    JQSAPIServerThread = NULL;
}
