#!/bin/bash
# use this script to package all class files here
# please add manifast.txt first

jar -cvmf manifast.txt jar1.jar *.class
