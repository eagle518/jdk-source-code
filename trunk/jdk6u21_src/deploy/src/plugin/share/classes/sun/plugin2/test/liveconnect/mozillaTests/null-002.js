/**
        File Name:      null-001.js
        Description:

        When accessing a null Java value, JavaScript should read
        the value as the JavaScript null object.

        To test this:

        1.  Call a java method that returns the Java null value
        2.  Check the value of the returned object, which should be null
        3.  Check the type of the returned object, which should be "object"

        NOTE that this test necessarily assumes the presence of a
        NullValueTest applet named "nullValueTest" on the surrounding web page.

        @author     kenneth.russell@sun.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java null to JavaScript Object - Test #2";
    var HEADER  = SECTION + " " + TITLE;

    var tc = 0;
    var testcasesI = new Array();

    startTest();
    writeHeaderToLog( HEADER );
//  display test information

    try {
        var val = nullValueTest.getNullBoolean();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "var val = nullValueTest.getNullBoolean();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );

        val = nullValueTest.getNullByte();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullByte();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );

        val = nullValueTest.getNullCharacter();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullCharacter();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );
    
        val = nullValueTest.getNullShort();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullShort();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );

        val = nullValueTest.getNullInteger();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullInteger();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );

        val = nullValueTest.getNullLong();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullLong();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );
    
        val = nullValueTest.getNullFloat();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullFloat();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );
    
        val = nullValueTest.getNullDouble();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullDouble();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );
    
        val = nullValueTest.getNullString();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullString();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );
    
        val = nullValueTest.getNullObject();
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "val = nullValueTest.getNullObject();",
            null,
            val );
        testcasesI[testcasesI.length] = new TestCase(
            SECTION,
            "typeof val",
            "object",
            typeof val );

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
