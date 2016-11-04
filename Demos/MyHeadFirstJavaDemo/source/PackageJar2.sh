#!/bin/bash
# use this script to package all class files here
# please add manifast.txt first

javac -d ../classes org/batmancn/*.java
cd ../classes
jar -cvmf manifast.txt jar2.jar org
