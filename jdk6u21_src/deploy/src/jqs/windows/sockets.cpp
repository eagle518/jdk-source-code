/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sockets.hpp"
#include "print.hpp"

#include <sstream>

/*
 * Initializes sockets library.
 */
void initSocketLibrary() {
    WSADATA wsaData;

    // Initialize WinSock 2.2
    int res = WSAStartup(0x0202, &wsaData);
    if (res) {
        jqs_error("Unable to initialize socket library, error %d\n", res);
    }
}


/*
 * Cleanups sockets library before exiting.
 */
void cleanupSocketLibrary() {
    WSACleanup();
}


/////////////////////////////////////////////////////////////////////////////

/*
 * Creates socket event.
 * @throws SocketException.
 */
SocketEvent::SocketEvent() {
    event = WSACreateEvent();
    if (event == WSA_INVALID_EVENT) {
        throw SocketException("WSACreateEvent() failed");
    }
}

SocketEvent::~SocketEvent() {
    WSACloseEvent(event);
}

/*
 * Returns underlying system event object.
 */
WSAEVENT SocketEvent::getEvent() const {
    return event;
}


/*
 * Waits on the event, for the specified timeout.
 * @throws SocketException.
 * @throws TimeoutException.
 */
void SocketEvent::wait(unsigned int timeout) {
    DWORD dwTimeout = (timeout == INFINITE_TIMEOUT) ?
                           WSA_INFINITE : (DWORD)timeout;

    DWORD res = WSAWaitForMultipleEvents(1,
                                         &event,
                                         FALSE,
                                         dwTimeout,
                                         FALSE);
    switch (res) {
        case WSA_WAIT_EVENT_0:
            break;
        case WSA_WAIT_TIMEOUT:
            throw TimeoutException();
        default:
            throw SocketException("WSAWaitForMultipleEvents() failed");
    }

    if (!WSAResetEvent(event)) {
        throw SocketException("WSAResetEvent() failed");
    }
}


/*
 * Resets event.
 * @throws SocketException.
 */
void SocketEvent::reset() {
    if (!WSAResetEvent(event)) {
        throw SocketException("WSAResetEvent() failed");
    }
}

/*
 * Signals event, interrupts thread that is waiting on the event.
 * @throws SocketException.
 */
void SocketEvent::signal() {
    if (!WSASetEvent(event)) {
        throw SocketException("WSASetEvent() failed");
    }
}

/////////////////////////////////////////////////////////////////////////////

/*
 * Creates a TCP socket.
 * @throws SocketException
 */
Socket::Socket ()
    : interrupted(false)
{
    s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        throw SocketException("socket() failed");
    }
    timeout = INFINITE_TIMEOUT;
}

/*
 * Creates a new socket object using given initialized socket.
 * @throws SocketException
 */
Socket::Socket (SOCKET socket)
    : s(socket)
    , interrupted(false)
{
    if (s == INVALID_SOCKET) {
        throw SocketException("Socket creation failed: unable to use INVALID_SOCKET");
    }
    timeout = INFINITE_TIMEOUT;
}

/*
 * Closes socket.
 */
Socket::~Socket() {
    ::closesocket(s);
}

/*
 * Binds the socket to the specified address.
 * @throws AddressAlreadyInUseException
 * @throws SocketException
 */
void Socket::bind (SOCKADDR_IN* sa) {
    int res = ::bind(s,                          // Socket
                     (LPSOCKADDR)sa,             // Server address
                     sizeof(struct sockaddr));   // Length of server address structure
    if (res == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEADDRINUSE) {
            throw AddressAlreadyInUseException();
        }
        throw SocketException("bind() failed");
    }
}

/*
 * Places socket into a listen state.
 * @throws SocketException
 */
void Socket::listen () {
    int res = ::listen(s,                          // Socket
                       SOMAXCONN);                 // Backlog
    if (res == SOCKET_ERROR) {
        throw SocketException("listen() failed");
    }
}

/*
 * Waits for incoming connections.
 * @throws SocketException
 */
Connection* Socket::accept () {
    if (!select(status_readable))
        throw TimeoutException();

    SOCKADDR_IN sa;
    int sa_len = sizeof(struct sockaddr);
    SOCKET connectionSocket = ::accept(s,                // Socket
                                       (LPSOCKADDR)&sa,  // Incoming connection address
                                       &sa_len);         // Length of address structure
    if (connectionSocket == INVALID_SOCKET) {
        throw SocketException("accept() failed");
    }
    if (sa.sin_addr.s_addr != htonl(LOCALHOST)) {
        ::closesocket (connectionSocket);
        jqs_warn ("Connection from %d.%d.%d.%d refused\n", 
            sa.sin_addr.s_net,
            sa.sin_addr.s_host,
            sa.sin_addr.s_lh,
            sa.sin_addr.s_impno);
        throw SocketException("incomming connection refused\n");
    }
    return new Connection(connectionSocket);
}

/*
 * Connects the socket to the specified address.
 * @throws SocketException
 */
void Socket::connect (SOCKADDR_IN* sa) {
    int res = ::connect(s,                          // Socket
                        (LPSOCKADDR)sa,             // Server address
                        sizeof(struct sockaddr));   // Length of server address structure
    if (res == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAECONNREFUSED) {
            // Connection refused
            throw ConnectionRefusedException();
        }
        throw SocketException("connect() failed");
    }
}

/*
 * Sends message to the connected socket.
 * @throws SocketException
 * @throws TimeoutException
 */
void Socket::send(const char* msg, int length) {
    if (!select(status_writeable))
        throw TimeoutException();
    int res = ::send(s,                         // Socket
                     msg,                       // Data buffer
                     length,                    // Length of data
                     0);                        // Flags
    if (res == SOCKET_ERROR) {
        throw SocketException("send() failed");
    }
}

/*
 * Receives message from the connected socket.
 * @throws SocketException
 * @throws TimeoutException
 * @throws SocketClosedException
 */
int Socket::recv(char* buf, int bufLen) {
    if (!select(status_readable))
        throw TimeoutException();
    int res = ::recv(s,                   // Bound socket
                     buf,                 // Receive buffer
                     bufLen,              // Size of buffer in bytes
                     0);                  // Flags
    if (res == SOCKET_ERROR) {
        throw SocketException("recv() failed");
    }
    if (res == 0) {
        throw SocketClosedException();
    }
    return res;
}

/*
 * Returns the local address for the socket.
 * @throws SocketException.
 */
void Socket::getLocalAddress(SOCKADDR_IN* sa) {
    int sa_len = sizeof(struct sockaddr);
    int res = ::getsockname(s, (LPSOCKADDR)sa, &sa_len);
    if (res == SOCKET_ERROR) {
        throw SocketException("getsockname() failed");
    }
}

/*
 * Waits until the socket becomes readable or writable.
 * @returns false if timeout expires.
 * @throws SocketException on error.
 */
bool Socket::select(socket_status status) {
    return select(status, timeout);
}

/*
 * Waits until the socket becomes readable or writable.
 * The timeout value is specified in milliseconds.
 * @returns false if timeout expires.
 * @throws SocketException on error
 */
bool Socket::select(socket_status status, int timeout) {
    timeval tval;
    tval.tv_sec  = timeout / 1000;
    tval.tv_usec = (timeout % 1000) * 1000;
    timeval* tm = (timeout == INFINITE_TIMEOUT) ? 0 : &tval;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    fd_set* read_fds  = (status == status_readable)  ? &fds : 0;
    fd_set* write_fds = (status == status_writeable) ? &fds : 0;

    int res = ::select(1, read_fds, write_fds, 0, tm);
    if ((res == SOCKET_ERROR) || ((res != 0) && (res != 1))) {
        throw SocketException("select() failed");
    }
    return (res == 1);
}

/*
 * Specifies set of network events to be associated with socket event object,
 * used in wait().
 * @throws SocketException
 */
void Socket::eventSelect(long eventMask) {
    int res = WSAEventSelect(s, socketEvent.getEvent(), eventMask);
    if (res == SOCKET_ERROR) {
        throw SocketException("WSAEventSelect() failed");
    }
}


/*
 * Sets the timeout for all socket read operations.
 * The timeout is specified in milliseconds.
 */
void Socket::setTimeout(int tm) {
    timeout = tm;
}


/*
 * Waits for the message.
 * Timeout value is specified in milliseconds; for infinite timeout
 * use the special value INFINITE_TIMEOUT.
 * This wait can be interrupted from another thread by interrupt().
 * @throws SocketException.
 * @throws InterruptedException
 * @throws TimeoutException.
 */
void Socket::wait(int timeout) {
    socketEvent.reset();
    if (!interrupted) {
        if (!select(Socket::status_readable, 0)) {
            socketEvent.wait(timeout);
        }
    }
    if (interrupted) {
        interrupted = false;
        throw InterruptedException();
    }
}

/*
 * Interrupts wait().
 * @throws SocketException.
 */
void Socket::interrupt() {
    interrupted = true;
    socketEvent.signal();
}

//////////////////////////////////////////////////////////////////////////

/*
 * Establishes connection to the JQS server on the specified port.
 * @throws SocketException.
 */
Connection::Connection (int port) {
    // fill in the address structure
    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(LOCALHOST);
    sa.sin_port = htons(port);

    // connect to server
    connect(&sa);

    // associate read event with socket
    eventSelect(FD_READ);
}

/*
 * Creates a new connection using given connected socket.
 * @throws SocketException.
 */
Connection::Connection (SOCKET socket)
    : Socket(socket)
{
    // associate read event with socket
    eventSelect(FD_READ);
}


/*
 * Sends message.
 * @throws SocketException.
 */
void Connection::sendMessage(const std::vector<char>& message) {
    send(&(message[0]), (int)message.size());
}


/*
 * Receives message.
 * The received message is appended into the end of the buffer.
 * Blocks until the message is available or timeout is expired.
 * @throws SocketException.
 * @throws TimeoutException if timeout expires.
 * @throws SocketClosedException
 */
void Connection::receiveMessage(std::vector<char>& buffer) {
    const int receiveBufferSize = 65536;
    std::vector<char> receiveBufferVec(receiveBufferSize);
    char* receiveBuffer = &(receiveBufferVec[0]);

    // receive response
    int nbytes = recv (receiveBuffer, receiveBufferSize);

    buffer.insert(buffer.end(), receiveBufferVec.begin(), receiveBufferVec.begin() + nbytes);
}


/*
 * Creates a server socket to listen incoming connections on the specified port.
 * @throws AddressAlreadyInUseException
 * @throws SocketException.
 */
ServerSocket::ServerSocket(int port) {
    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(LOCALHOST);
    sa.sin_port = htons(port);

    bind(&sa);
    listen();

    // associate accept event with socket
    eventSelect(FD_ACCEPT);
}

