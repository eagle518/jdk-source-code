/**
        File Name:      null-001.js
        Description:

        When accessing a Java field whose value is null, JavaScript should read
        the value as the JavaScript null object.

        To test this:

        1.  Call a java method that returns the Java null value
        2.  Check the value of the returned object, which should be null
        3.  Check the type of the returned object, which should be "object"

        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java null to JavaScript Object";
    var HEADER  = SECTION + " " + TITLE;

    var tc = 0;
    var testcasesI = new Array();

    startTest();
    writeHeaderToLog( HEADER );
//  display test information

    try {
        var choice = new app.Packages.java.awt.Choice();
    
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "var choice = new app.Packages.java.awt.Choice(); choice.getSelectedObjects()",
            null,
            choice.getSelectedObjects() );
    
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof choice.getSelectedObjects()",
            "object",
            typeof choice.getSelectedObjects() );
    
        test();
    } catch (e) {
        writeExceptionToLog(e);
    }

function CheckType( et, at ) {
}
function CheckValue( ev, av ) {
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
