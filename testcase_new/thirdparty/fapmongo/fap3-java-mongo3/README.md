1 环境准备及执行测试
1.1 环境中已安装jdk1.7和maven

1.2 需要覆盖测试mongodb java driver版本如下：
v3.2.2、v3.3.0、v3.4.3、v3.5.0、v3.6.4、v3.7.1、v3.8.2、v3.9.1、3.10.0、3.11.2、3.12.2 

1.3 编辑配置文件./src/test/resources/mongodb.properties,需要填写以下配置项：
# spring数据库
mongodb.springDBName=mongodb3springtest
# java数据库
mongodb.javaDBName=mongodb3javatest
# 因为聚集操作在多个协调节点下会有问题，所以在spring用例在单个协调节点下执行。待问题解决后，再修改公共类
mongodb.singleUrl=localhost:11817
# 多个地址时，请以","进行分割，因测试代码未对mongodb.multiUrl格式进行校验，请正确填写
mongodb.multiUrl=localhost:11817,localhost:11917
# 每台机器最大的连接数
mongodb.connectionsPerHost=30
# 允许阻塞的连接数
mongodb.threadsAllowedToBlockForConnectionMultiplier=30
# 连接超时时间
mongodb.connectTimeout=60000
# 等待连接最大时间
mongodb.maxWaitTime=60000
mongodb.socketKeepAlive= true
mongodb.socketTimeout=60000

1.5 编辑pom.xml文件
修改${mongo-java-driver}参数，填写mongo-java-driver版本

1.4 执行测试
以下命令默认在pom.xml所在文件夹下执行
mvn surefire-report:report -DxmlFileName=testng.xml -Dmongo-java-driver=3.4.3 -DreportDir=$PROJECT_ROOT_PATH/report 


2 测试代码
2.1 整体结构
drwxr-xr-x 1 jars/---------运行测试用例所需要的依赖包
-rw-r--r-- 1 pom.xml-------通过maven的方式将jars/注入到工程
-rw-r--r-- 1 README.md
drwxr-xr-x 1 src/------测试代码

