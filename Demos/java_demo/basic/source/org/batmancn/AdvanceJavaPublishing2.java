/**
*   1. To demo << devide source and class files>>
*       - javac -d ../classes *.java
*/

import java.io.OutputStream;

class AdvanceJavaPublishingTow {
    public static void main (String args[]) {
        if (args.length == 0) {
            // length property is inner property of java array.

            // `==` operator in java, is to compare if the address
            //  of object is the same.
            // `equals()` function in java.object is also to compare
            //  address, but some java lib (such as inner data type
            //  like Date, Interger, String) rewrite this function.
            //  After rewrite, this function compare the content.
            // Notice: For other object defined by user or complex,
            //  `equals` still compare their address.
            System.out.println("AdvanceJavaPublishingOne, no input.");
        } else if (args.length == 1) {
            System.out.println("AdvanceJavaPublishingOne, one input = "
            + args[1] +".");
        } else {
            System.out.println("Too much input.");
        }
    }
}
