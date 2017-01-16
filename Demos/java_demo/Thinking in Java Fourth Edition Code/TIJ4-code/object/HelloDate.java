//: object/HelloDate.java
import java.util.*;

/** The first Thinking in Java example program.
 * Displays a string and today's date.
 * @author Bruce Eckel
 * @author www.MindView.net
 * @version 4.0
 *
 * Primitive Type: it's built-in type in java vm, not object.
 * Wrapper Type: wrapper of primitive type into object, so that
 *  the object could be used as object, like added into container
 *  such as Collection.
 *
 * primitive type - memory size - biggest value - smallest value - wrapper type:
 * boolean - - - Boolean
 * char - 16-bit(For global language support) - Unicode 0 - Unicode 2^(16-1)-1 - Charactor
 * byte - 8-bit - ... - Byte
 * short - 16-bit - ... - Short
 * int - 32-bit - ... - Integer
 * long - 64-bit - ... - Long
 * float - 64-bit - ... - Float
 * double - 64-bit - ... - Double
 * void - ... - Void
 *
 * Java also has BigInterger, BigDicimal. Java Array refer to 'arrayMethod'.
*/
public class HelloDate {
    /** Private method of array demo.
    */
    void arrayMethod() {
        // This is first choice to declare array, and all array name is pointer.
        // This is just declare an array name.
        Int [] int_array;
        // This is to alloc data.
        int_array = new Int[10];

        // Alos could be done by one statement.
        Int [] int_array_one = new Int[10];
        Int [] int_array_two = {1, 2, 3};
    }
  /** Entry point to class & application.
   * @param args array of string arguments
   * @throws exceptions No exceptions thrown
  */
  public static void main(String[] args) {
    System.out.println("Hello, it's: ");
    System.out.println(new Date());
  }
} /* Output: (55% match)
Hello, it's:
Wed Oct 05 14:39:36 MDT 2005
*///:~
