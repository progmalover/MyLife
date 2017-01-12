// Java has no 'virtual' key word, as all method in java is virtual, excluding 'final' method.

class Base
{
    Base() {
        System.out.println("Base");
        return ;
    }
}

class Child extends Base
{
     Child() {
        super();
        System.out.println("Child");
        return ;
    }
}

class Main
{
    public static void main(String args[]) {
        Child c = new Child();
    }
}
