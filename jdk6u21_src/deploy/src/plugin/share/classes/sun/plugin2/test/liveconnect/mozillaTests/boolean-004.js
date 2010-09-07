/**
        File Name:      boolean-005.js
        Description:

        A java.lang.Boolean object should be read as a JavaScript JavaObject.

        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect";
    var VERSION = "1_3";
    var TITLE   = "Java Boolean Object to JavaScript Object : Test #3";
    var HEADER  = SECTION + " " + TITLE;

    var tc = 0;
    var testcasesIV = new Array();

    startTest();
    writeHeaderToLog( HEADER );

    //  In all test cases, the expected type is "object"

    var E_TYPE = "object";

    //  The JavaScrpt [[Class]] of a JavaObject should be JavaObject"

    var E_JSCLASS = "[object JavaObject]";

    //  The Java class of this object is java.lang.Boolean.

    var E_JAVACLASS = java.lang.Class.forName( "java.lang.Boolean" );

    //  Create arrays of actual results (java_array) and expected results
    //  (test_array).

    var java_array = new Array();
    var test_array = new Array();

    try {
        var i = 0;
    
        // Test for java.lang.Boolean.FALSE, which is a Boolean object.
        java_array[i] = new JavaValue(  java.lang.Boolean.FALSE  );
        test_array[i] = new TestValue(  "java.lang.Boolean.FALSE",
                                        false );
    
        i++;
    
        // Test for java.lang.Boolean.TRUE, which is a Boolean object.
        java_array[i] = new JavaValue(  java.lang.Boolean.TRUE  );
        test_array[i] = new TestValue(  "java.lang.Boolean.TRUE",
                                        true );
    
        i++;
    
        for ( i = 0; i < java_array.length; i++ ) {
            CompareValues( java_array[i], test_array[i] );
    
        }
    
        test();
    } catch (e) {
        writeExceptionToLog(e);
    }

function CompareValues( javaval, testval ) {
    //  Check booleanValue()
    testcasesIV[testcasesIV.length] = new TestCase( SECTION,
                                                "("+testval.description+").booleanValue()",
                                                testval.value,
                                                javaval.value );
    //  Check typeof, which should be E_TYPE
    testcasesIV[testcasesIV.length] = new TestCase( SECTION,
                                                "typeof (" + testval.description +")",
                                                testval.type,
                                                javaval.type );
/*
    //  Check JavaScript class, which should be E_JSCLASS
    testcasesIV[testcasesIV.length] = new TestCase( SECTION,
                                                "(" + testval.description +").getJSClass()",
                                                testval.jsclass,
                                                javaval.jsclass );
*/
    //  Check Java class, which should equal() E_JAVACLASS
    testcasesIV[testcasesIV.length] = new TestCase( SECTION,
                                                "(" + testval.description +").getClass().equals( " + E_JAVACLASS +" )",
                                                true,
                                                javaval.javaclass.equals( testval.javaclass ) );

    // Check string representation
    testcasesIV[testcasesIV.length] = new TestCase( SECTION,
        "("+ testval.description+") + ''",
        testval.string,
        javaval.string );
}
function JavaValue( value ) {
    //  java.lang.Object.getClass() returns the Java Object's class.
    this.javaclass = value.getClass();


//  __proto__ of Java objects is not supported in LC2.
// Object.prototype.toString will show its JavaScript wrapper object.
//    value.__proto__.getJSClass = Object.prototype.toString;
//    this.jsclass = value.getJSClass();

    this.string = value + "";
    //print( this.string );
    this.value  = value.booleanValue();
    this.type   = typeof value;

    return this;
}
function TestValue( description, value ) {
    this.description = description;
    this.string = String( value );
    this.value = value;
    this.type =  E_TYPE;
    this.javaclass = E_JAVACLASS;
    this.jsclass = E_JSCLASS;
    return this;
}
function test() {
    for ( tc=0; tc < testcasesIV.length; tc++ ) {
        testcasesIV[tc].passed = writeTestCaseResult(
                            testcasesIV[tc].expect,
                            testcasesIV[tc].actual,
                            testcasesIV[tc].description +" = "+
                            testcasesIV[tc].actual );

        testcasesIV[tc].reason += ( testcasesIV[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcasesIV );
}
