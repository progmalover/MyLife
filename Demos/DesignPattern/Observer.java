// Java has observer mode inside
import java.util.Observer;
import java.util.*;

// Param to transform information between teacher and student
class Param
{
    ...
}

// Noticer
class Teacher extends Observable
{
    // Teacher tell students to read this page
    public void beginRead(int page)
    {
        Param p = new Param(page);
        notifyObservers(p);
    }

    // Function notifyObservers do NOT care about what's class of param
    public void notifyObservers(Object b)
    {
        setChanged();   // This is function of parent class ???
        super.notifyObservers(b);
    }
}

// Observers
class Student implements Observer
{
    // construct function, we use this to force student to register into teacher
    public Student(Teacher t)
    {
        t.addObserver(this);
    }

    // Read page
    public void read(page)
    {
        ...
    }

    // This is function defined of Observer framework in Java
    @Framewrok
    public void update(Observable o, Object arg)
    {
        Param p = (Param)arg;
        read(arg.getPage());
    }
}

// Main class
public class ClassRoom
{
    public static void main(String[] args)
    {
        Teacher t = new Teacher();
        Student s[20];

        for(int i = 0; i < 20; i++)
        {
            s[i] = new Student(t);
            t.beginRead(10);    // Notify all students to read page 10
        }
    }
}
