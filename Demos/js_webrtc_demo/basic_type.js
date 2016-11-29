// 1. <Javascript权威指南 第四版>, JS basic language reference.
// 2. http://www.w3school.com.cn/js/, start up guaid.

// Javascript is from C++ and Java, so the comment style and operator and loop
// style is the same.

// JS only use double, instead of int, long, double, so no need to declare
// type, Like python.
var a = 10.0;
var b = 0xABCD;
var string = a.toString(); // string = "10"
var string2 = b.toString(16); // string2 = "0xABCD"
// Special numerical constants:
infinity -Infinity NaN // etc, refer <1>P43
// string
var empty = ""
var str = "a" // this equals char
var str = "string"
// function
function square(x)
{
    return x*x;
}

var square2 = function{x} {return x*x;}
var y1 = square(x);
var y2 = square2(x);

TBD: P49
