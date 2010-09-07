var completed = false;
var testcases;

onerror = err;

SECTION = "";
VERSION = "";
BUGNUMBER = "";
DEBUG = false;
TZ_DIFF = -8;

var GLOBAL = "[object Window]";

var HTML_OUTPUT = false;

function TestCase( n, d, e, a ) {
            this.name        = n;
            this.description = d;
            this.expect      = e;
            this.actual      = a;
            this.reason      = "";
            this.bugnumber   = BUGNUMBER;

            this.passed = getTestCaseResult( this.expect, this.actual );

}
function startTest() {
//    document.open();
}
function getTestCaseResult( expect, actual ) {
    //  because ( NaN == NaN ) always returns false, need to do
    //  a special compare to see if we got the right result.

        if ( actual != actual ) {
            if ( typeof actual == "object" ) {
                actual = "NaN object";
            } else {
                actual = "NaN number";
            }
        }

        if ( expect != expect ) {
            if ( typeof expect == "object" ) {
                expect = "NaN object";
            } else {
                expect = "NaN number";
            }
        }

        var passed = ( expect == actual ) ? true : false;

    //  if both objects are numbers, give a little leeway for rounding.
        if (    !passed
                && typeof(actual) == "number"
                && typeof(expect) == "number"
            ) {
                if ( Math.abs(actual-expect) < 0.0000001 ) {
                    passed = true;
                }
        }

    //  verify type is the same
        if ( typeof(expect) != typeof(actual) ) {
            passed = false;
        }

        return passed;
}

function writeTestCaseResult( expect, actual, string ) {
        var passed = getTestCaseResult( expect, actual );
        writeFormattedResult( expect, actual, string, passed );
        // The automation driver might not be available
        try {
            if (passed) {
                pass(string);
            } else {
                fail(string + ": expected " + expect);
            }
        } catch (e) {
        }
        return passed;
}
function writeFormattedResult( expect, actual, string, passed ) {
        var s = "<tt >" + string ;

        for ( k = 0;
              k <  (60 - string.length >= 0 ? 60 - string.length : 5) ;
              k++ ) {
            s += "&nbsp;" ;
        }

        s += "<b ><font color=" ;
        s += (( passed ) ? "'#009900' >&nbsp;PASSED!"
                    : "'#990000' >&nbsp;FAILED: expected </tt><tt >" +
                      expect ) + "</tt>";

        writeLineToLog( s + "</font ></b ></tt >" );

        return passed;
}
function writeExceptionToLog(e) {
    var prefix = "&nbsp;&nbsp;&nbsp;&nbsp;";
    var s = "<tt ><b ><font color='#FF0000' >EXCEPTION: </font ></b >";
    if (typeof e == "string") {
        s += "<br>\n" + prefix + e;
    } else {
        for (var i in e) {
            s += "<br>\n" + prefix + i + " = " + e[i];
        }
    }
    s += "</tt >";
    writeLineToLog(s);
    // The automation driver might not be available
    try {
        exception(getExceptionString(e));
    } catch (e) {
    }
}
function getExceptionString(e) {
    if (typeof e == "string") {
        return e;
    } else {
        var s = "";
        for (var i in e) {
            s += "" + i + " = " + e[i] + "\n";
        }
        return s;
    }
}

function writeLineToLog( string ) {
    document.write( string + "<br>\n" );
}
function writeHeaderToLog( string ) {
    document.write( "<h2>" + string + "</h2>\n" );
    // The automation driver might not be available
    try {
        setHeader(string);
    } catch (e) {
    }
}
function stopTest() {
//    document.write("<hr>");
//    document.close();
    completed = true;
}
function err( msg, page, line ) {
/*    testcases[tc].actual = "error";
    testcases[tc].reason = msg;
    writeTestCaseResult( testcases[tc].expect,
                         testcases[tc].actual,
                         testcases[tc].description +" = "+ testcases[tc].actual +
                         ": " + testcases[tc].reason );
    stopTest();*/
    return true;
}

function ToInteger( t ) {
    t = Number( t );

    if ( isNaN( t ) ){
        return ( Number.NaN );
    }
    if ( t == 0 || t == -0 ||
         t == Number.POSITIVE_INFINITY || t == Number.NEGATIVE_INFINITY ) {
         return 0;
    }

    var sign = ( t < 0 ) ? -1 : 1;

    return ( sign * Math.floor( Math.abs( t ) ) );
}
function Enumerate ( o ) {
    var p;
    for ( p in o ) {
        print( p +": " + o[p] );
    }
}
