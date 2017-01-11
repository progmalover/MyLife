

class Base
{
    public virtual void method() {
        System.out.println("Base");
    }
}

class Child implement Base
{
    public virtual void method() {
        System.out.println("Child");
    }
}
