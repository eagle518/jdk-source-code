import java.applet.*;
import netscape.javascript.*;

public class JSJavaTest1 extends Applet {
    private boolean testFailed = false;
    private String firstFailingTest;

    public void startTest() {
        testFailed = false;
    }

    private void fail(String failingTestName, String errorMessage) {
        testFailed = true;
        firstFailingTest = failingTestName;
        System.out.println(errorMessage);
    }

    public void test5Hello(int number, String hello) {
        if (testFailed)
            return;

        if (number != 5) {
            fail("test5Hello", "Expected 5 for number, got " + number);
        }

        if (hello == null || (!hello.equals("hello"))) {
            fail("test5Hello", "Expected \"hello\", got " + hello);
        }
    }

    // Returns true if the test passed, false otherwise
    public boolean endTest() {
        if (testFailed) {
            System.err.println("TEST FAILED: first failing test = " + firstFailingTest);
        } else {
            System.out.println("Test passed.");
        }
        return !testFailed;
    }

    public String getFirstFailingTest() {
        return firstFailingTest;
    }
}
