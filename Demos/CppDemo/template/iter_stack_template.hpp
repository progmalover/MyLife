#ifndef ITER_STACK_TAMPLATE_H
#define ITER_STACK_TAMPLATE_H

// JAVA treat every thing of pointer, C++ does not do this, but JAVA is more
// effecient that store every this into heap. So it's better to use T as
// pointer, same like JAVA, otherwise StackTemplate will be too big.
template<class T, int ssize = 1000>
class StackTemplate {
    T stack[ssize]; // to store class T
    int top;    // to point the top
public:
    StackTemplate() : top(0) {}
    void push(const T &t)
    {
        demo_assert_log(top < ssize, "top=%d is oversize", top);
        stack[top++] = T;
    }
    T pop(void)
    {
        demo_assert_log(top > 0, "top=%d is too small", top);
        return stack[top--];
    }
    class iterator; // declare iter
    friend class iterator;  // declare friend to access private member. For iterator, he could access StackTemplate's member directly, that's the iterator usage
    class iterator {    // internal class of name space StackTemplate
        StackTemplate &s;   // point to class StackTemplate, here is tightly coupled, so using &
        int index;  // index of stack[ssize]
    public:
        iterator(StackTemplate &st) : s(st), index(0) {}
        iterator(StackTemplate &st, bool) : s(st), index(s.top) {}
        T operator*()
        {
            return s.stack[index];
        }
        T operator++()
        {
            demo_assert(index < s.top);
            return s.stack[index++];
        }
        // we do not need operator--
        iterator& operator+=(int amount)
        {
            demo_assert_log(index + amount < s.top);    // STL does not has this restriction!
            index += amount;
            return *this;
        }
        bool operator==(const iterator &iter) const
        {
            return index == iter.index;
        }
        bool operator!=(const iterator &iter) const
        {
            return index != iter.index;
        }
    };
    iterator begin() const
    {
        return iterator(*this);
    }
    iterator end() const
    {
        return iterator(*this, true);
    }
};

// Demo to use this is:
// int main()
// {
//     StackTemplate<Demo*> st;    // use JAVA style
//     st.push(d1);
//     st.push(d2);
//
//     for (StackTemplate::iterator iter = st.begin(); iter != st.end(); iter++) {
//         process(*iter)
//     }
//
//     st.pop();
//     st.pop();
//
//     return 0;
// }

#endif
