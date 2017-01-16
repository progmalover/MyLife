//: innerclasses/Callbacks.java
// Using inner classes for callbacks
//
// Enclosure is important, just like C/C++ callback function.
//
// Caller is the way we use callback in Java, just like C,
// Caller use 'go' function to kickoff callback pointer,
// then Caller.go use callbackReference's increment function,
// this callbackReference and its increment function could be
// implement by Callee1 or Callee2.
// So Callee1 and Callee2 is callback class, and its increment
// function is callback function.
//
// Callee1 did not use inner class.
// Callee2 is another one, use inner class.
//
// Different between Callee1 and Callee2 is public/private
// of increment function. Callee2 use enclosure to explose
// only one function 'getCallbackReference', Callee1 must
// explose 'increment'.
package innerclasses;
import static net.mindview.util.Print.*;

// This is callback interface, in Java, called enclosure.
interface Incrementable {
  void increment();
}

// Very simple to just implement the interface:
class Callee1 implements Incrementable {
  private int i = 0;
  public void increment() {
    i++;
    print(i);
  }
}

class MyIncrement {
  public void increment() { print("Other operation"); }
  static void f(MyIncrement mi) { mi.increment(); }
}

// If your class must implement increment() in
// some other way, you must use an inner class:
class Callee2 extends MyIncrement {
  private int i = 0;
  public void increment() {
    super.increment();
    i++;
    print(i);
  }
  private class Closure implements Incrementable {
    public void increment() {
      // Specify outer-class method, otherwise
      // you'd get an infinite recursion:
      Callee2.this.increment();
    }
  }
  Incrementable getCallbackReference() {
    return new Closure();
  }
}

class Caller {
  private Incrementable callbackReference;
  Caller(Incrementable cbh) { callbackReference = cbh; }
  void go() { callbackReference.increment(); }
}

public class Callbacks {
  public static void main(String[] args) {
    Callee1 c1 = new Callee1();
    Callee2 c2 = new Callee2();
    MyIncrement.f(c2);
    Caller caller1 = new Caller(c1);
    Caller caller2 = new Caller(c2.getCallbackReference());
    caller1.go();
    caller1.go();
    caller2.go();
    caller2.go();
  }
} /* Output:
Other operation
1
1
2
Other operation
2
Other operation
3
*///:~
