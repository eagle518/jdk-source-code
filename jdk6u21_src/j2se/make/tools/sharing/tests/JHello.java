
import java.awt.Font;
import javax.swing.JFrame;
import javax.swing.JLabel;

public class JHello extends JFrame {

    public static void main(String[] args) {
	System.out.println("Hello");

	new JHello().show();
	if (args.length == 1 && args[0].equals("quit")) {
	    try {
		Thread.currentThread().sleep(1000);
	    } catch (InterruptedException e) {
	    }
	    System.exit(0);
	}
    }


    JHello() {
	JLabel jlabel = new JLabel("Hello");
	jlabel.setFont(new Font("Monospaced", Font.PLAIN, 144));
	getContentPane().add(jlabel);
	pack();
    }
}

