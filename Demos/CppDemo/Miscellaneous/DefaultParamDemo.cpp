/*
 * =====================================================================================
 *
 *       Filename:  DefaultParamDemo.cpp
 *
 *    Description:  How to use default value parameter.
 *
 *        Version:  1.0
 *        Created:  02/03/2016 07:27:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  batmancn ,
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>

// 1. C style
extern "C"
{
void func(int a, int b=30);

void func(int a, int b)
{
    // ...
}
}

using namespace std;

class Demo
{
public:
    void func(int a, int b = 30);
}

void Demo::func(int a, int b)
{
    // ...
}
