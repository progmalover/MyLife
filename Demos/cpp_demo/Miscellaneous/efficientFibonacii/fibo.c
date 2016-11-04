/*
 * =====================================================================================
 *
 *       Filename:  fibo.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  02/10/2016 12:24:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "fibo.h"
#include <iostream>
#include <vector>
using namespace std;

// constructor
Fibonacii::Fibonacii(int s) {
    size = s;   // no need to write this->, except param is also size.
}

void Fibonacii::createSeries(void) {
    data.push_back(0);
    data.push_back(1);
    for (int i = 2; i < size; i++) {
        data.push_back(data[i-2], data[i-1]);
    }
}

void Fibonacii::getData() {
    for (long i: data) {    // this is C++11 style
        cout << i << endl;
    }
}
