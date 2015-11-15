#!/bin/python
# -*- coding: utf-8 -*-
import random
import time

### 1. Read items
res = file('gd_predict.txt', 'w')

### 2. Algorithm, this should be cluster algorithm
#   for 20150101 ~ 20150107:
#       for 06 ~ 07:    #increament
#           print('\n')
#       for 13 ~ 16:    #decreament
#           print('\n')
#       for 17 ~ 19:    #increament
#            print('\n')
#       for 20 ~ 21:    #decreament
#           print('\n')
#       random pop X items into 1 record and output this record
try :
    scale = 3  # used for holiday
    scale2 = 2  # used for workday
    s2 = 3
    for k in range(1, 22):
        c = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        for i in range(20150101, 20150104): # holiday
            c[6] = random.randint(10, 200)
            c[7] = random.randint(200, 600*s2)
            c[8] = random.randint(400, 1000*s2*s2)
            c[9] = random.randint(400, 1000*s2)
            c[10] = random.randint(200, 600*s2)
            c[11] = random.randint(400, 1000*s2)
            c[12] = random.randint(400, 1500*s2*s2)
            c[13] = random.randint(200, 1000*s2)
            c[14] = random.randint(200, 700*s2)
            c[15] = random.randint(200, 700*s2)
            c[16] = random.randint(200, 700*s2)
            c[17] = random.randint(400, 1000*s2)
            c[18] = random.randint(600, 1500*s2*s2)
            c[19] = random.randint(200, 1000*s2*s2*s2)
            c[20] = random.randint(100, 600*s2)
            c[21] = random.randint(50, 200*s2)
            for j in range(6, 22):
                res.write('线路'+str(k)+','+str(i)+','+str(j)+','+str(c[j] * scale)+'\n')
#                res.write(('线路%d,%d,%d,%d\n', k, i, j, c[j])).decode('utf-8').encode('gbk')

        for i in range(20150104, 20150108): # workday
            c[6] = random.randint(10, 200)
            c[7] = random.randint(200, 600*s2)
            c[8] = random.randint(400, 1000*s2*s2)
            c[9] = random.randint(400, 1000*s2*s2)
            c[10] = random.randint(200, 600*s2)
            c[11] = random.randint(400, 1000*s2)
            c[12] = random.randint(400, 1500*s2*s2)
            c[13] = random.randint(200, 1000*s2)
            c[14] = random.randint(200, 700*s2)
            c[15] = random.randint(200, 700*s2)
            c[16] = random.randint(200, 700*s2)
            c[17] = random.randint(400, 1000*s2)
            c[18] = random.randint(600, 1500*s2*s2)
            c[19] = random.randint(200, 1000*s2*s2*s2)
            c[20] = random.randint(100, 600*s2)
            c[21] = random.randint(50, 200*s2)
            for j in range(6, 22):
                res.write('线路'+str(k)+','+str(i)+','+str(j)+','+str(c[j] * scale2)+'\n')

finally:
    res.flush()
    res.close()
