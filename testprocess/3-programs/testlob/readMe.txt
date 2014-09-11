使用方法：

前提：在testlob所在目录需要创建2个文件夹。lobsource目录用于存放待读取的文件，lobout目录用于存放待保存的文件。

./testlob <-m mode> [-t totalNum] [-c connectionString] [-s dbName] [-n threadNumber]  [-i interval]
mode:
	0: 从操作系统中读取文件存储在db中
        1: 从db中读取文件保存至本地操作系统
        2: 模式0和模式1的混合，预计比例为7:3。比例在代码中写死

totalNum:
	操作的总次数（不是每个线程的操作次数）。

connectionString:
	连接字符串，例如192.168.20.164:11810

dbName:
	mongoDB中的dbName，sequoiaDB中的csName

threadNumber:
	执行线程数目

interval:
	每个线程执行多少次操作记录一下消耗时间