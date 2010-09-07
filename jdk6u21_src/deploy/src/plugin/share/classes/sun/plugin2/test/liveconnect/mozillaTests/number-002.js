/**
        File Name:      number-002.js
        Description:

        Accessing a Java field whose value is one of the primitive Java types
        below, JavaScript should read this as a JavaScript Number object.
            byte
            short
            int
            long
            float
            double
            char

        To test this:
        1.  Instantiate a new Java object that has a field whose type one of
            the above primitive Java types, OR get the value of a class's static
            field.
        2.  Check the value of the returned object
        3.  Check the type of the returned object, which should be "object"
        4.  Check the class of the returned object using Object.prototype.toString,
            which should return "[object Number]"

        It is an error if the type of the JavaScript variable is "number" or if
        its class is JavaObject.

        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java Number Primitive to JavaScript Object: Test #2";
    var HEADER  = SECTION + " " + TITLE;

    var tc = 0;
    var testcasesI = new Array();

    startTest();
    writeHeaderToLog( HEADER );

    //  In all test cases, the expected type is "object, and the expected
    //  class is "Number"

    var E_TYPE = "number";

    //  Create arrays of actual results (java_array) and expected results
    //  (test_array).

    var java_array = new Array();
    var test_array = new Array();

    try {
        var i = 0;
    
        //  Get a static java field whose type is byte.
    
        java_array[i] = new JavaValue(  app.Packages.java.lang.Byte.MIN_VALUE );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Byte.MIN_VALUE",
                                        -128 )
        i++;
    
        // Get a static java field whose type is short.
        java_array[i] = new JavaValue(  app.Packages.java.lang.Short.MIN_VALUE );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Short.MIN_VALUE",
                                        -32768 )
        i++;
    
        //  Get a static java field whose type is int.
    
        java_array[i] = new JavaValue( app.Packages.java.lang.Integer.MIN_VALUE );
        test_array[i] = new TestValue( "app.Packages.java.lang.Integer.MIN_VALUE",
                                       -2147483648 )
        i++;
    
    
        //  Instantiate a class, and get a field in that class whose type is int.
    
        var java_rect = new app.Packages.java.awt.Rectangle( 1,2,3,4 );
    
        java_array[i] = new JavaValue( java_rect.width );
        test_array[i] = new TestValue( "java_object = new app.Packages.java.awt.Rectangle( 1,2,3,4 ); java_object.width",
                                       3 );
        i++;
    
        //  Get a static java field whose type is long.
        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.MIN_VALUE );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.MIN_VALUE",
                                        -9223372036854776000 );
        i++;
    
        //  Get a static java field whose type is float.
        java_array[i] = new JavaValue(  app.Packages.java.lang.Float.MAX_VALUE );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Float.MAX_VALUE",
                                         3.4028234663852886e+38 )
        i++;
    
        //  Get a static java field whose type is double.
        java_array[i] = new JavaValue(  app.Packages.java.lang.Double.MAX_VALUE );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Double.MAX_VALUE",
                                         1.7976931348623157e+308 )
        i++;
    
        //  Get a static java field whose type is char.
        java_array[i] = new JavaValue(  app.Packages.java.lang.Character.MAX_VALUE );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Character.MAX_VALUE",
                                         65535 );
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
    //  Check type.

    testcasesI[testcasesI.length] = new TestCase( SECTION,
                                                "typeof (" + testval.description +")",
                                                testval.type,
                                                javaval.type );
}

function JavaValue( value ) {
    this.value  = value.valueOf();
    this.type   = typeof value;
    return this;
}
function TestValue( description, value, type  ) {
    this.description = description;
    this.value = value;
    this.type =  E_TYPE;
//    this.classname = classname;
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
    return ( testcasesI );
}
