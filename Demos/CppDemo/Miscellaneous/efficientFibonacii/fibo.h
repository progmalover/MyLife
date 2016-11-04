/*
 * =====================================================================================
 *
 *       Filename:  fibo.h
 *
 *    Description:  Demo for http://www.cplusplus.com/articles/jT07M4Gy/
 *                  Notice we use c++11
 *
 *        Version:  1.0
 *        Created:  02/10/2016 12:11:27 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _fibo_h_
#define _fibo_h_
#include <vector>

class Fibonacii {
private:
    int size;
    std::vector<long> data;
public:
    Fibonacii(int s);
    void createSeries(void);
    void getData(void);
}
