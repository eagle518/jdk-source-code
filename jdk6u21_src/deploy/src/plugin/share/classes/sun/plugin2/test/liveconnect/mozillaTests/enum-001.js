/**
    File Name:          enum-001.js
    Section:            LiveConnect
    Description:

    Tests enumeration of a java object.

    Regression test for bug:

    http://scopus.mcom.com/bugsplat/show_bug.cgi?id=107638

    Author:             christine@netscape.com
    Date:               12 november 1997
*/
    var SECTION = "enum-001";
    var VERSION = "JS1_3";
    var TITLE   = "The variable statment";
    var HEADER  = SECTION + " " + TITLE;

    startTest();
    writeHeaderToLog( HEADER );

    var tc = 0;
    var testcases = new Array();

    try {
        var v = new app.Packages.java.util.Vector();
        v.addElement("PASSED!");
        v.addElement("PASSED!");
        v.addElement("PASSED!");
    
        for (e = v.elements(), result = new Array(), i = 0 ; e.hasMoreElements();
            i++ )
        {
            result[i] = String( e.nextElement() );
        }
    
        for ( i = 0; i < result.length; i++ ) {
            testcases[testcases.length] = new TestCase( SECTION,
                "test enumeration of a java object:  element at " + i,
                "PASSED!",
                result[i] );
        }
    
        test();
    } catch (e) {
        writeExceptionToLog(e);
    }

function test() {
    for ( tc=0; tc < testcases.length; tc++ ) {
        testcases[tc].passed = writeTestCaseResult(
                            testcases[tc].expect,
                            testcases[tc].actual,
                            testcases[tc].description +" = "+
                            testcases[tc].actual );

        testcases[tc].reason += ( testcases[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcases );
}
