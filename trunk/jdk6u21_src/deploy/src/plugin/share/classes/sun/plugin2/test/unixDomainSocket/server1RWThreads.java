/* Generic program structure for establishing
   connection-oriented client-server environment. */

/* server program - 2 threads (Read and Write) ..
 *   - AF_UNIX is bidirectional,
 *     this example shows how data is read/write concurrently 
 *     from 2 threads using one socket.
 */

import java.io.*;
import java.net.*;
import java.nio.*;

import com.sun.deploy.config.Config;
import com.sun.deploy.net.socket.UnixDomainSocket;
import com.sun.deploy.net.socket.UnixDomainSocketException;

public class server1RWThreads implements Runnable {
   
    long t0; 
    ByteBuffer bufOut;
    long procIdMain;
    long threadIdMain;
    UnixDomainSocket sockServer;

    Object readerSyncLock = new Object();
    ByteBuffer bufIn;
    long procIdReader;
    long threadIdReader;
    UnixDomainSocket sockClient;

    public static void main(String[] args) {
        server1RWThreads srv = new server1RWThreads();
        srv.startReaderThread();
        srv.sendStuff();
    }

    public server1RWThreads() {
        init();
    }

    private void init()  
    {
        if(!UnixDomainSocket.isSupported()) {
            throw new RuntimeException("UnixSocket.unStreamSocketSupported(): not supported"); // bail out
        }
    
        bufOut = ByteBuffer.allocateDirect(8*1024);
        procIdMain = Config.getNativePID();
        threadIdMain = Thread.currentThread().getId();
        sockServer=null;
        t0 = System.currentTimeMillis();

        bufIn  = ByteBuffer.allocateDirect(8*1024);
        procIdReader = -1;
        threadIdReader = -1;
        sockClient = null;

        try {
            // sockServer = UnixDomainSocket.CreateServerBindListen(0, 1);
            sockServer = new UnixDomainSocket(0);
            System.out.println("Server socket: "+sockServer);
            sockServer.bind();
            sockServer.listen(1);
        } catch (UnixDomainSocketException e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage()); // bail out
        }

        System.out.println("Server socket created: (pid "+procIdMain+":"+threadIdMain+") - Socket: "+sockServer);
    }

    private void shutdown() {
        sockClient.close();
        sockServer.close();
    }

    public void sendStuff() {
        // wait until the connection is established ..
        if(null==sockClient) {
            System.out.println("Server.Sender (pid "+procIdMain+":"+threadIdMain+"): Waiting for client connection ...");
            synchronized(readerSyncLock) {
                while(null==sockClient) {
                    try {
                        readerSyncLock.wait();
                    } catch (InterruptedException ie) {}
                }
            }
        }
        System.out.println("Server.Sender (pid "+procIdMain+":"+threadIdMain+"): ready .. ");

        int count = 0;
        String bufStr = "null";

        while ( true ) {
            int n=0, nT=0;
            count++;
            String cmsg = new String("Server("+count+", (pid "+procIdMain+":"+threadIdMain+") echo: '"+bufStr+"'\n");
            bufOut.clear();
            bufOut.put(tools.getByteBuffer(cmsg));
            bufOut.flip();

            try {
                while ( bufOut.hasRemaining() && (n=sockClient.write(bufOut)) > 0) {
                    nT+=n;
                }
            } catch (UnixDomainSocketException e) {
                e.printStackTrace();
                break; // EOS
            }
            long dT = System.currentTimeMillis() - t0;
            System.out.println(dT+"ms (pid "+procIdMain+":"+threadIdMain+"): Send to client: "+nT+" bytes: <"+cmsg+">");
            System.out.flush();
            Thread.yield(); // help a bit ..
        }
        long dT = System.currentTimeMillis() - t0;
        System.out.println(dT+"ms Server.Sender (pid "+procIdMain+":"+threadIdMain+"): EOS");
        System.out.flush();
        shutdown();
    }

    public void startReaderThread() {
        new Thread(this).start();
    }

    public void run() {
        try {
            synchronized(readerSyncLock) {
                procIdReader = Config.getNativePID();
                threadIdReader = Thread.currentThread().getId();
                System.out.println("Server.Reader (pid "+procIdReader+":"+threadIdReader+"): Waiting for client to connect ...");
                sockClient = sockServer.accept();
                readerSyncLock.notifyAll();
            }
        } catch (UnixDomainSocketException e) {
            e.printStackTrace();
        }

        System.out.println("Server.Reader (pid "+procIdReader+":"+threadIdReader+"): Client Socket: "+sockClient);
        System.out.println("Server.Reader (pid "+procIdReader+":"+threadIdReader+"): Waiting for message from client ...");

        while ( true ) {
            String bufStr;

            Thread.yield(); // help a bit ..
            try {
                int n=0, nT=0;
                // blocking message read ..
                bufIn.clear();
                while( bufIn.hasRemaining() && (n=sockClient.read(bufIn)) > 0 ) {
                    nT+=n;
                }
                if(nT==0) {
                    continue; // no data
                }
                bufIn.flip();
                bufStr = tools.getString(bufIn);
                long dT = System.currentTimeMillis() - t0;
                System.out.println(dT+"ms (pid "+procIdReader+":"+threadIdReader+"): Recvd from client: "+nT+" bytes: <"+bufStr+">");
                System.out.flush();
            } catch (UnixDomainSocketException e) {
                e.printStackTrace();
                break; // EOS
            }
        }
        shutdown();
        long dT = System.currentTimeMillis() - t0;
        System.out.println(dT+"ms Server.Reader (pid "+procIdReader+":"+threadIdReader+"): EOS");
        System.out.flush();
    }
}

