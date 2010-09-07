import java.applet.Applet;
import java.awt.BorderLayout;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
 
import netscape.javascript.JSObject;
 
public class RoundTripLiveConnectFromEDT extends Applet {
 
    private MyCanvas canvas;
    private String name;

    public void init() {
        name = getParameter("scripter_name");
        if (name == null) {
            name = "scripter";
        }
        this.setIgnoreRepaint(true);
        this.setBackground(Color.WHITE);
        this.canvas = new MyCanvas();
        this.setLayout(new BorderLayout());
        this.add(this.canvas, BorderLayout.CENTER);
    }
 
    public void start() {
        this.canvas.start();
    }
 
    public Graphics getGraphics() {
        return canvas.getGraphics();
    }
 
    private class MyCanvas extends Canvas {
 
        JSObject win;
 
        public void start() {
            win = JSObject.getWindow(RoundTripLiveConnectFromEDT.this);
        }
 
        public void paint(Graphics g) {
            render();
        }
 
        public void update(final Graphics g) {
            this.paint(g);
        }
 
        public void render() {
            if (win == null) {
                return;
            }
            int width = RoundTripLiveConnectFromEDT.this.getWidth();
            int height = RoundTripLiveConnectFromEDT.this.getHeight();
            Graphics2D g2d = (Graphics2D)getGraphics();
            g2d.setColor(Color.WHITE);
            g2d.fillRect(0, 0, width, height);
            win.eval("var g = document." + name + ".getGraphics();" +
                     "g.setColor(new java.awt.Color(0, 0, 220));" +
                     "for (var i = 0; i < 400; i += 50) { g.drawLine(i, 0, i, 400);}");
        }
    }
}
