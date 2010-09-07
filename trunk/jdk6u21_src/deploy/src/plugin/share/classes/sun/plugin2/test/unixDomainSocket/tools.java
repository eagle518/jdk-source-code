
import java.io.*;
import java.net.*;
import java.nio.*;

public class tools {
   
    public static ByteBuffer getByteBuffer(String str)
    {
        return ByteBuffer.wrap(getBytes(str));
    }

    public static byte[] getBytes(String str)
    {
        byte[] bufByte;
        try {
            bufByte = str.getBytes("UTF-8");
        } catch (java.io.UnsupportedEncodingException e) {
            bufByte = str.getBytes();
        }
        return bufByte;
    }

    public static String getString(ByteBuffer buff)
    {
        byte[] bufByte = new byte[buff.limit()];
        buff.get(bufByte);
        String bufStr;
        try {
            bufStr = new String(bufByte, "UTF-8");
        } catch (java.io.UnsupportedEncodingException e) {
            bufStr = new String(bufByte);
        }
        return bufStr;
    }
}
    
