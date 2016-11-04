/*
 * =====================================================================================
 *
 *       Filename:  demo.cpp
 *
 *    Description:  Only pointer or reference support polymorphism
 *
 *        Version:  1.0
 *        Created:  03/27/2016 08:12:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>

class LibraryMaterials
{
public:
    void checkIn(void);
};

class Book : public LibraryMaterials
{
...
}

void
testFunction() {
    LibraryMaterials a; // alloc on stack
    Book b;

    // Oops: a is NOT a Book
    a = b;
    // Oops: can't invoke
    a.checkIn();

    // OK, use ancester reference
    LibraryMaterials &c = b;
    // OK
    c.checkIn();
}
