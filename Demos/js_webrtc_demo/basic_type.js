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


// variable: 1. var declare area; 2. var content; 3. member var
// 1:
var i = 10
i = "ten" // OK
// { } will not infect var, like below, refer <1>P62
var i = "1"
function () {
    var i;
    alert(i); // output "undefined", because var i is defined next line.
    var i = "2"
    alert(i); // output 2
}

// 2. Basic type and reference type:
// Does it just like C++ or Java's content and its pointer? Yes.

// 3. member var:
// For global var and local var, they could all be treated as member var of
// global object and local object(function, class, obect), Like python.


// function: 1. function defination
// 1. how to define function, JS is like python, function is an object and
// the function name or var or sth reference this object.
function func1(x, y) // this is basic useage
{
    return x+y;
}
var func2 = function(x, y) {return x+y;} // direct function variable, this is
                // common use.
var func3 = new Function(x, y, return x+y;); // call Function(this is a function
                // to construct a function) to construct a function
array.sort(function(x, y) {return x < y;});

// as function is an object, so function could be used like var, as below
function operator(op, p1, p2) // op=operator, p1=param1
{
    return op(p1, p2);
}
var res = operator(func, x, y);

TBD:P130
