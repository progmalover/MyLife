/**
*   @author batmancn
*   1. To show how to use basic function of java.
*       - javac $this
*       - java Demo1
*       - Output: Demo1
*   2. If we use `javac BasicJavaHelloWorldDemo.java`, then we will get
*   Demo.class in this folder, it seems that `javac` has no option, same
*   as `-o` option of `g++`.
*/
package BasicJavaHelloWorldDemo;

import java.io.OutputStream;

class AccessControl {
    private void private_function() {
        System.out.println("This is private function.");
    }

    void also_public_function() {
        System.out.println("This is also public function.");
    }

    public void public_function() {
        System.out.println("This is public function.");
        this.private_function();
    }
}

class Demo1 {
    public static void main (String args[]) {
        System.out.println("Demo1");

        AccessControl ac = new AccessControl();
        //ac.private_function(); // compile error
        ac.also_public_function();
        ac.public_function();
    }
}
