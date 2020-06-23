1、环境中已安装jdk1.7和maven

2、需要覆盖测试mongodb java driver版本如下：
v2.14.2

3、编辑配置文件./src/test/resources/mongodb.properties,需要填写以下配置项： 
# mongodb节点主机
mongodb.host=localhost
# mongodb节点端口号
mongodb.port=11817

4、执行测试
mvn surefire-report:report -DxmlFileName=testng.xml -Dmongo-java-driver=2.14.2 -DreportDir=$PROJECT_ROOT_PATH/report 