import java.applet.*;
import netscape.javascript.*;

public class JavaJSTest1 extends Applet {
    public void start() {
        try {
            JSObject window = JSObject.getWindow(this);
            System.out.println("Calling JavaScript new cities();");
            JSObject res = (JSObject) window.eval("new cities();");
            System.out.println("Finished calling JavaScript");
            printCities(res);
            res.setMember("b", "Belfast");
            System.out.println("Set cities.b to Belfast");
            printCities(res);
            if (!res.getMember("b").equals("Belfast")) {
                throw new RuntimeException("TEST FAILED");
            }
            System.out.println("Deleting cities.b");
            res.removeMember("b");
            printCities(res);
            try {
                res.getMember("b");
                throw new RuntimeException("TEST FAILED");
            } catch (JSException e) {
                // Member should not be present any more
            }
            System.out.println("Test passed.");
        } catch (JSException e) {
            e.printStackTrace();
        } catch (Exception e2) {
            e2.printStackTrace();
        }
    }

    private void printCities(JSObject cities) {
        printCity(cities, "a");
        printCity(cities, "b");
        printCity(cities, "c");
    }

    private void printCity(JSObject cities, String which) {
        System.out.print("cities." + which + " = ");
        try {
            System.out.println(cities.getMember(which));
        } catch (JSException e) {
            System.out.println("[undefined]");
        }
    }
}
