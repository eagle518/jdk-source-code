/**
        File Name:      api-001.js
        Description:

        Tests certain problematic API calls and verifies they work.

        @author     kenneth.russell@sun.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java API calls: Test #1";
    var HEADER  = SECTION + " " + TITLE;

    var tc = 0;
    var testcasesI = new Array();

    startTest();
    writeHeaderToLog( HEADER );

    try {
        var date = new app.Packages.java.util.Date(01012006);

        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "var date = new app.Packages.java.util.Date(01012006);",
            "object",
            typeof date );

        var owner  = new app.Packages.javax.swing.JFrame();

        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "var owner = new Packages.javax.swing.JFrame();",
            "object",
            typeof owner );

        var dialog = new app.Packages.javax.swing.JDialog(owner);

        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "var dialog = new Packages.javax.swing.JDialog(owner);",
            "object",
            typeof dialog );

        var cl = app.Packages.java.lang.Class.forName("java.lang.Boolean");
        var name = cl.getName();

        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "Packages.java.lang.Class.forName(\"java.lang.Boolean\").getName()",
            "java.lang.Boolean",
            name );

        test();
    } catch (e) {
        writeExceptionToLog(e);
    }

function test() {
    for ( tc=0; tc < testcasesI.length; tc++ ) {
        testcasesI[tc].passed = writeTestCaseResult(
                            testcasesI[tc].expect,
                            testcasesI[tc].actual,
                            testcasesI[tc].description +" = "+
                            testcasesI[tc].actual );

        testcasesI[tc].reason += ( testcasesI[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcasesI );
}
