/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include "os_layer.hpp"
#include "print.hpp"

#include <string>
#include <sstream>
#include <vector>


/////////////////////////////////////////////////////////////////////////////

/*
 * Initializes sockets library.
 */
extern void initSocketLibrary();

/*
 * Cleanups sockets library before exiting.
 */
extern void cleanupSocketLibrary();


/*
 * Infinite timeout.
 */
const int INFINITE_TIMEOUT = INT_MAX;

/*
 * Localhost IP
 */
#define LOCALHOST            0x7f000001           /* 127.0.0.1 */

/////////////////////////////////////////////////////////////////////////////

class SocketException {
    std::string message;
    int socketError;
public:
    SocketException ()
        : socketError(ERROR_SUCCESS)
    {}

    SocketException (const char* msg)
        : message(msg)
        , socketError(WSAGetLastError())
    {}

    const char* getMessage() const {
        return message.c_str();
    }

    int getSocketError() const {
        return socketError;
    }
};

class TimeoutException : public SocketException {
public:
    TimeoutException ()
        : SocketException("operation timed out")
    {}
};

class AddressAlreadyInUseException : public SocketException {
};

class ConnectionRefusedException : public SocketException {
};

class SocketClosedException : public SocketException {
};

class InterruptedException : public SocketException {
};


/////////////////////////////////////////////////////////////////////////////

class SocketEvent {
    WSAEVENT event;

public:
    /*
     * Creates socket event.
     * @throws SocketException.
     */
    SocketEvent();

    virtual ~SocketEvent();

    /*
     * Returns underlying system event object.
     */
    WSAEVENT getEvent() const;


    /*
     * Waits on the event, for the specified timeout.
     * @throws SocketException.
     * @throws TimeoutException.
     */
    void wait(unsigned int timeout);

    /*
     * Resets event.
     * @throws SocketException.
     */
    void reset();

    /*
     * Signals event, interrupts thread that is waiting on the event.
     * @throws SocketException.
     */
    void signal();
};

/////////////////////////////////////////////////////////////////////////////

class Connection;

class Socket {
    SOCKET s;
    int timeout;
    SocketEvent socketEvent;
    volatile bool interrupted;

public:

    /*
     * Creates a TCP socket.
     * @throws SocketException
     */
    Socket ();

    /*
     * Creates a new socket object using given initialized socket.
     * @throws SocketException
     */
    Socket (SOCKET socket);

    /*
     * Closes socket.
     */
    virtual ~Socket();

    /*
     * Binds the socket to the specified address.
     * @throws AddressAlreadyInUseException
     * @throws SocketException
     */
    void bind (SOCKADDR_IN* sa);

    /*
     * Places socket into a listen state.
     * @throws SocketException
     */
    void listen ();

    /*
     * Waits for incoming connections.
     * @throws SocketException
     */
    Connection* accept ();

    /*
     * Connects the socket to the specified address.
     * @throws SocketException
     * @throws ConnectionRefusedException
     */
    void connect (SOCKADDR_IN* sa);

    /*
     * Sends message to the connected socket.
     * @throws SocketException
     * @throws TimeoutException
     */
    void send(const char* msg, int length);

    /*
     * Receives message from the connected socket.
     * @throws SocketException
     * @throws TimeoutException
     * @throws SocketClosedException
     */
    int recv(char* buf, int bufLen);

    /*
     * Returns the local address for the socket.
     * @throws SocketException.
     */
    void getLocalAddress(SOCKADDR_IN* sa);

    typedef enum {status_readable, status_writeable} socket_status;

    /*
     * Waits until the socket becomes readable or writable.
     * @returns false if timeout expires.
     * @throws SocketException on error.
     */
    bool select(socket_status status);

    /*
     * Waits until the socket becomes readable or writable.
     * The timeout value is specified in milliseconds.
     * @returns false if timeout expires.
     * @throws SocketException on error
     */
    bool select(socket_status status, int timeout);

    /*
     * Specifies set of network events to be associated with socket event object,
     * used in wait().
     * @throws SocketException
     */
    void eventSelect(long eventMask);

    /*
     * Sets the timeout for all socket read operations.
     * The timeout is specified in milliseconds.
     */
    void setTimeout(int tm);

    /*
     * Waits for the message  or incoming connection.
     * Timeout value is specified in milliseconds; for infinite timeout
     * use the special value INFINITE_TIMEOUT.
     * This wait can be interrupted from another thread by interrupt().
     * @throws SocketException.
     * @throws InterruptedException
     * @throws TimeoutException.
     */
    void wait(int timeout);

    /*
     * Interrupts wait().
     * @throws SocketException.
     */
    void interrupt();
};


class Connection : protected Socket {
public:
    /*
     * Establishes connection to the JQS server on the specified port.
     * @throws SocketException.
     */
    Connection (int port);

    /*
     * Creates a new connection using given connected socket.
     * @throws SocketException.
     */
    Connection (SOCKET s);

    virtual ~Connection() {}

    /*
     * Sends message.
     * @throws SocketException.
     */
    void sendMessage(const std::vector<char>& message);

    /*
     * Receives message.
     * The received message is appended into the end of the buffer.
     * Blocks until the message is available or timeout is expired.
     * @throws SocketException.
     * @throws TimeoutException if timeout expires.
     * @throws SocketClosedException
     */
    void receiveMessage(std::vector<char>& buffer);

    /*
     * Sets the timeout for all socket read operations.
     * The timeout is specified in milliseconds.
     */
    void setTimeout(int tm) {
        Socket::setTimeout(tm);
    }
  
    /*
     * Waits for the message  or incoming connection.
     * Timeout value is specified in milliseconds; for infinite timeout
     * use the special value INFINITE_TIMEOUT.
     * This wait can be interrupted from another thread by interrupt().
     * @throws SocketException.
     * @throws InterruptedException
     * @throws TimeoutException.
     */
    void wait(int timeout) {
        Socket::wait(timeout);
    }

    /*
     * Interrupts wait().
     * @throws SocketException.
     */
    void interrupt() {
        Socket::interrupt ();
    }
};


class ServerSocket : protected Socket {
public:
    /*
     * Creates a server socket to listen incoming connections on the specified port.
     * @throws AddressAlreadyInUseException
     * @throws SocketException.
     */
    ServerSocket(int port);

    /*
     * Closes server.
     */
    virtual ~ServerSocket() {}

    /*
     * Waits for incoming connections.
     * @throws SocketException
     */
    Connection* accept () {
        return Socket::accept();
    }
  
    /*
     * Waits for the message  or incoming connection.
     * Timeout value is specified in milliseconds; for infinite timeout
     * use the special value INFINITE_TIMEOUT.
     * This wait can be interrupted from another thread by interrupt().
     * @throws SocketException.
     * @throws InterruptedException
     * @throws TimeoutException.
     */
    void wait(int timeout) {
        Socket::wait(timeout);
    }

    /*
     * Interrupts wait().
     * @throws SocketException.
     */
    void interrupt() {
        Socket::interrupt ();
    }
};


#endif
