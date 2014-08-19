rm lobtest.o -rf
g++ -c ../../gtest/src/gtest_main.cc -I ../../gtest/include/ -g
g++ -c lobtest.cpp   -I ../../../../client/include/ -I ../../gtest/include/ -g
g++ -g -o lobtest lobtest.o ../../gtest/src/gtest-all.o gtest_main.o -L ../../../../client/lib/ -l staticsdbc -l pthread
