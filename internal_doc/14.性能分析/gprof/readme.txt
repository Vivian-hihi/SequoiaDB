1. 编译

scons增加编译选项--gprof，只在linux下编译engine时生效。
scons --dd --gprof --engine

gprofile原生不支持多线程，需要编译一个动态库gprof-helper来支持。
编译gprof-helper.c:
gcc -shared -fPIC gprof-helper.c -o gprof-helper.so -lpthread -ldl
使用：通过LD_PRELOAD环境变量预加载gprof-helper.so
LD_PRELOAD=./gprof-helper.so your_program

2. 执行

gprofile要求应用程序正常退出，只有在程序正常退出后才会生成gmon.out文件。

所以可以执行下面命令启动sequoiadb：
LD_PRELOAD=<path to gprof-helper.so> ./sequoiadb --dbpath <path to database> -p 11810
完成操作后按ctrl+c退出，即可生成gmon.out文件。

3. 输出报告

gprof sequoiadb gmon.out -b > report.txt

gprof生成的报告对于sequoiadb来说太大，无法清晰的看出函数调用关系，此时可以使用gprof2dot.py和graphviz生成图形。
需要预先安装graphviz。

gprof2dot.py report.txt | dot -Tpng -o report.png
