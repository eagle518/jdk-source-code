/**
        File Name:      char-002.js
        Description:

        Java fields whose value is char should be read as a JavaScript number.

        To test this:

        1.  Instantiate a Java object that has fields with char values,
            or reference a classes static field whose value is a char.
        2.  Reference the field, and set the value of a JavaScript variable
            to that field's value.
        3.  Check the value of the returned type, which should be "number"
        4.  Check the type of the returned type, which should be a number

        It is an error if the JavaScript variable is an object, or JavaObject
        whose class is java.lang.Character.

        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java char return value to JavaScript Object: Test #2";
    var HEADER  = SECTION + " " + TITLE;
    var tc = 0;
    var testcasesII = new Array();

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
    
        // Get File.separator char
    
        var os = new app.Packages.java.lang.String(app.Packages.java.lang.System.getProperty( "os.name" ));
        var v;
    

        if ( os.startsWith( "Windows" ) || os.startsWith( "OS/2" ) ) {
            v = 92;
        } else {
            // Assume Unix platforms
            v = 47;
        }

    
        java_array[i] = new JavaValue(  app.Packages.java.io.File.separatorChar   );
        test_array[i] = new TestValue(  "app.Packages.java.io.File.separatorChar", v );
    
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
    testcasesII[testcasesII.length] = new TestCase( SECTION,
                                                testval.description,
                                                testval.value,
                                                javaval.value );
    //  Check type, which should be E_TYPE
    testcasesII[testcasesII.length] = new TestCase( SECTION,
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
    for ( tc=0; tc < testcasesII.length; tc++ ) {
        testcasesII[tc].passed = writeTestCaseResult(
                            testcasesII[tc].expect,
                            testcasesII[tc].actual,
                            testcasesII[tc].description +" = "+
                            testcasesII[tc].actual );

        testcasesII[tc].reason += ( testcasesII[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcasesII );
}
