package sun.awt.X11;

import java.io.*;
import java.awt.image.*;
import javax.imageio.*;
import java.awt.*;

public class ToBin {
    public static void main(String[] args) throws Exception {
        BufferedImage im = ImageIO.read(System.in);
        BufferedImage bi = null;
        int iconWidth = im.getWidth(null);
        int iconHeight = im.getHeight(null);
        if (im != null && iconHeight != 0 &&  iconWidth != 0) {
            bi = new BufferedImage(iconWidth, iconHeight, BufferedImage.TYPE_INT_ARGB);
            Graphics g = bi.getGraphics(); 
            try {
                g.drawImage(im, 0, 0, iconWidth, iconHeight, null);
            } finally {
                g.dispose();
            }
        }
        DataBuffer srcBuf = bi.getData().getDataBuffer();
        int[] buf = ((DataBufferInt)srcBuf).getData();
        System.out.print(iconWidth + ",");
        System.out.println(iconHeight + ",");
        for (int i = 0; i < buf.length; i++) {
            System.out.print("0x" + Integer.toHexString(buf[i]) + ", ");
            if (i % 10 == 0) {
                System.out.println();
            }            
        }
    }
}
