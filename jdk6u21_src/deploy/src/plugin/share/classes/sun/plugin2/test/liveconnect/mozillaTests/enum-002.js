/**
    File Name:          enum-002.js
    Section:            LiveConnect
    Description:

    Tests enumeration of a java object.  This also does some
    conversions.

    Regression test for bug:

    http://scopus.mcom.com/bugsplat/show_bug.cgi?id=107638

    Author:             christine@netscape.com
    Date:               12 november 1997
*/
    var SECTION = "enum-002";
    var VERSION = "JS1_3";
    var TITLE   = "The variable statment";
    var HEADER  = SECTION + " " + TITLE;

    startTest();
    writeHeaderToLog( HEADER );

    var tc = 0;
    var testcases = new Array();

    try {
        var v = new app.Packages.java.util.Vector();
        v.addElement("TRUE");
    
        for (e = v.elements(), result = new Array(), i = 0 ; e.hasMoreElements();
            i++ )
        {
            result[i] = (new app.Packages.java.lang.Boolean(e.nextElement())).booleanValue();
        }
    
        for ( i = 0; i < result.length; i++ ) {
            testcases[testcases.length] = new TestCase( SECTION,
                "test enumeration of a java object:  element at " + i,
                "true",
                String( result[i] ) );
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
