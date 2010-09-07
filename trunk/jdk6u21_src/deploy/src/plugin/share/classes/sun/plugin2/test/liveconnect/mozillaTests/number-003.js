/**
        File Name:      number-003.js
        Description:

        Tests certain numerical boundary values passed from Java to JavaScript
        and ensures they are passed correctly.

        @author     kenneth.russell@sun.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java Number Primitive to JavaScript Object: Test #3";
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
    
        // Note that the parsing routines don't allow the high bit to be set,
        // so we have to use Long.valueOf() instead of Integer.valueOf() in some cases

        java_array[i] = new JavaValue(  app.Packages.java.lang.Integer.valueOf('3FFFFFFF', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Integer.valueOf('3FFFFFFF', 16).intValue()",
                                        1073741823 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Integer.valueOf('40000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Integer.valueOf('40000000', 16).intValue()",
                                        1073741824 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Integer.valueOf('7FFFFFFF', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Integer.valueOf('7FFFFFFF', 16).intValue()",
                                        2147483647 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('80000001', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('80000001', 16).intValue()",
                                        -2147483647 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('80000002', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('80000002', 16).intValue()",
                                        -2147483646 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('90000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('90000000', 16).intValue()",
                                        -1879048192 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('A0000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('A0000000', 16).intValue()",
                                        -1610612736 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('B0000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('B0000000', 16).intValue()",
                                        -1342177280 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('BFFFFFFF', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('BFFFFFFF', 16).intValue()",
                                        -1073741825 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('C0000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('C0000000', 16).intValue()",
                                        -1073741824 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('C0000001', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('C0000001', 16).intValue()",
                                        -1073741823 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('CFFFFFFF', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('CFFFFFFF', 16).intValue()",
                                        -805306369 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('D0000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('D0000000', 16).intValue()",
                                        -805306368 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('E0000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('E0000000', 16).intValue()",
                                        -536870912 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('F0000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('F0000000', 16).intValue()",
                                        -268435456 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('FE000000', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('FE000000', 16).intValue()",
                                        -33554432 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('FFFFFFF0', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('FFFFFFF0', 16).intValue()",
                                        -16 )
        i++;

        java_array[i] = new JavaValue(  app.Packages.java.lang.Long.valueOf('FFFFFFFF', 16).intValue() );
        test_array[i] = new TestValue(  "app.Packages.java.lang.Long.valueOf('FFFFFFFF', 16).intValue()",
                                        -1 )
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
