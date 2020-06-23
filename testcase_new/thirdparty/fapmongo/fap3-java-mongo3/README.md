1、环境中已安装jdk1.7和maven

2、需要覆盖测试mongodb java driver版本如下：
v3.2.2、v3.3.0、v3.4.3、v3.5.0、v3.6.4、v3.7.1、v3.8.2、v3.9.1、3.10.0、3.11.2、3.12.2 

3、编辑配置文件./src/test/resources/mongodb.properties,需要填写以下配置项： 
# mongodb节点主机
mongodb.host=localhost
# mongodb节点端口号
mongodb.port=11817

4、执行测试
mvn surefire-report:report -DxmlFileName=testng.xml -Dmongo-java-driver=3.4.3 -DreportDir=$PROJECT_ROOT_PATH/report 