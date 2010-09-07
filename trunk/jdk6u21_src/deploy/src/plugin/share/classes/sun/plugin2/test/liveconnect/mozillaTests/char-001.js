/**
        File Name:      char-001.js
        Description:

        Java methods that return a char value should be read as a
        JavaScript number.

        To test this:

        1.  Call a method that returns a char.
        2.  Set the value of a JavaScript variable to the char value.
        3.  Check the value of the returned type, which should be "number"
        4.  Check the type of the returned type, which should be the Unicode
            encoding of that character.

        It is an error if the JavaScript variable is an object, or JavaObject
        whose class is java.lang.Character.

        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java char return value to JavaScript Object: Test #1";
    var HEADER  = SECTION + " " + TITLE;

    var tc = 0;
    var testcasesI= new Array();

    startTest();
    writeHeaderToLog( HEADER );

//  In all cases, the expected type is "number"
    var E_TYPE = "number";

    //  Create arrays of actual results (java_array) and expected results
    //  (test_array).

    var java_array = new Array();
    var test_array = new Array();

    try {
        var i = 0;
    
        // Get a char using String.charAt()
    
        java_array[i] = new JavaValue(  (new app.Packages.java.lang.String( "JavaScript" )).charAt(0)   );
        test_array[i] = new TestValue(  "(new app.Packages.java.lang.String( 'JavaScript' )).charAt(0)",
                                         74 );
    
        i++;
    
        for ( i = 0; i < java_array.length; i++ ) {
            CompareValues( java_array[i], test_array[i] );
    
        }
    
        test();
    } catch (e) {
        writeExceptionToLog(e);
    }

function CompareValues( javaval, testval ) {
    //  Check value
    testcasesI[testcasesI.length] = new TestCase( SECTION,
                                                testval.description,
                                                testval.value,
                                                javaval.value );
    //  Check type, which should be E_TYPE
    testcasesI[testcasesI.length] = new TestCase( SECTION,
                                                "typeof (" + testval.description +")",
                                                testval.type,
                                                javaval.type );

}
function JavaValue( value ) {
    this.value  = value;
    this.type   = typeof value;
    return this;
}
function TestValue( description, value ) {
    this.description = description;
    this.value = value;
    this.type =  E_TYPE;
    return this;
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
    return (testcasesI);
}
