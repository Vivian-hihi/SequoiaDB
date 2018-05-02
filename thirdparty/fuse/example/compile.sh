gcc -Wall StackFS_LowLevel_modified_for_yuweixing.c `pkg-config fuse --cflags --libs` -o StackFS_LowLevel_modified_for_yuweixing -I ../include/

gcc -Wall common.c StackFS_LowLevel_modified_for_yuweixing.c `pkg-config fuse --cflags --libs` -o StackFS_LowLevel_modified_for_yuweixing -I ../include/  -I '/home/users/yuweixing/driver/C&CPP/include' -L '/home/users/yuweixing/driver/C&CPP/lib/' -lsdbc -lrt 

./StackFS_LowLevel_modified_for_yuweixing -r rootdir/ stackdir/ --tracing
./StackFS_LowLevel_modified_for_yuweixing -r rootdir/ stackdir/ --tracing -o auto_unmount

g++ -Wall common.c StackFS_LowLevel_modified_for_yuweixing.cpp `pkg-config fuse --cflags --libs` -o StackFS_LowLevel_modified_for_yuweixing_cpp -I ../include/  -I '/home/users/yuweixing/driver/C&CPP/include' -L '/home/users/yuweixing/driver/C&CPP/lib/' -lsdbc -lrt
