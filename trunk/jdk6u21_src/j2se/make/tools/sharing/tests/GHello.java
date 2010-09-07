
import java.awt.Font;
import java.awt.Frame;
import java.awt.Label;

public class GHello extends Frame {

    public static void main(String[] args) {
	System.out.println("Hello");

	new GHello().show();
	if (args.length == 1 && args[0].equals("quit")) {
	    try {
		Thread.currentThread().sleep(200);
	    } catch (InterruptedException e) {
	    }
	    System.exit(0);
	}
    }


    GHello() {
	Label label = new Label("Hello");
	label.setFont(new Font("Monospaced", Font.PLAIN, 144));
	add(label);
	pack();
    }
}

