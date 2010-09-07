/* Generic program structure for establishing
   connection-oriented client-client environment. */

/* client program - 2 threads (Read and Write) ..
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

public class client1RWThreads implements Runnable {
   
    String pipeFileName;
    int sendLoops;
    ByteBuffer bufOut;
    long procIdMain;
    long threadIdMain;

    Object readerSyncLock = new Object();
    ByteBuffer bufIn;
    long procIdReader;
    long threadIdReader;
    UnixDomainSocket sockClient;

    public static void main(String[] args) {
        if(args.length<1) {
            System.out.println("usage: client1 <socket-file-name> [loops]");
            return;
        }
        String fileNameFQ = args[0];
        int loops=0;
        if(args.length>=2) {
            try {
                loops = Integer.parseInt(args[1]);
            } catch (Exception e) {
            }
        }
        if(loops<1) loops=1;

        client1RWThreads client = new client1RWThreads(fileNameFQ, loops);
        client.startReaderThread();
        client.sendStuff();
        System.out.println("Client .. sleep 1s ");
        try {
            Thread.sleep(1*1000); // sleep 1s ..
        } catch (Exception e) {}
        System.out.println("Client .. shutdown .. ");
        client.shutdown();
    }

    public client1RWThreads(String fname, int loops) {
        if(!UnixDomainSocket.isSupported()) {
            throw new RuntimeException("UnixDomainSocket not supported"); // bail out
        }
    
        pipeFileName = fname;
        sendLoops = loops;
        bufOut = ByteBuffer.allocateDirect(8*1024);
        procIdMain = Config.getNativePID();
        threadIdMain = Thread.currentThread().getId();

        bufIn  = ByteBuffer.allocateDirect(8*1024);
        procIdReader = -1;
        threadIdReader = -1;
        sockClient = null;
    }

    private void shutdown() {
        sockClient.close();
    }

    public void sendStuff() {
        // wait until the connection is established ..
        if(null==sockClient) {
            System.out.println("Client.Sender (pid "+procIdMain+":"+threadIdMain+"): Waiting for connection to server ...");
            synchronized(readerSyncLock) {
                while(null==sockClient) {
                    try {
                        readerSyncLock.wait();
                    } catch (InterruptedException ie) {}
                }
            }
        }
        System.out.println("Client.Sender (pid "+procIdMain+":"+threadIdMain+"): ready ..");

        int count = 0;
        String bufStr = "null";

        while ( count<sendLoops) {
            int n=0, nT=0;
            count++;
            String cmsg = new String("Client("+count+", (pid "+procIdMain+":"+threadIdMain+") echo: '"+bufStr+"'\n");
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
            System.out.println("(pid "+procIdMain+":"+threadIdMain+"): Send to server: "+nT+" bytes: <"+cmsg+">");
            Thread.yield(); // help a bit ..
        }
        System.out.println("Client.Sender (pid "+procIdMain+":"+threadIdMain+"): EOS .. "+count+"/"+sendLoops);
    }

    public void startReaderThread() {
        new Thread(this).start();
    }

    public void run() {
        try {
            synchronized(readerSyncLock) {
                procIdReader = Config.getNativePID();
                threadIdReader = Thread.currentThread().getId();
                System.out.println("Client.Reader (pid "+procIdReader+":"+threadIdReader+"): Waiting for connection to server ...");
                sockClient = UnixDomainSocket.CreateClientConnect(pipeFileName, false, 0);
                readerSyncLock.notifyAll();
            }
        } catch (UnixDomainSocketException e) {
            e.printStackTrace();
        }

        System.out.println("Client.Reader (pid "+procIdReader+":"+threadIdReader+"): Socket: "+sockClient);
        System.out.println("Client.Reader (pid "+procIdReader+":"+threadIdReader+"): Waiting for message from server ...");

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
                System.out.println("(pid "+procIdReader+":"+threadIdReader+"): Recvd from server: "+nT+" bytes: <"+bufStr+">");
            } catch (UnixDomainSocketException e) {
                e.printStackTrace();
                break; // EOS
            }
        }
        sockClient.close();
        System.out.println("Client.Reader (pid "+procIdReader+":"+threadIdReader+"): EOS");
    }
}

