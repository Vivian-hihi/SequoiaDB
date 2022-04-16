1.在`..\..\flink-base\pom.xml`中配置sequoiadb驱动依赖和并发框架依赖

2.使用如下命令编译测试代码

```shell
mvn clean install -Dmaven.test.skip=true
```

3.在flink中使用flink run指定编译后的jar包和testng.xml的路径以执行用例

```shell
${flink-path}/bin/flink run flink-test-3.6.jar testng.xml
```

4.执行用例前请确保flink的lib中包含如下依赖

```
flink-connector-jdbc_${version}.jar   	//flink jdbc连机器依赖
mysql-connector-java-${version}.jar 	//mysql jdbc依赖
sdb-connector-connector-${version}.jar 	//sequoiadb连接器依赖
sequoiadb-driver-{version}.jar        	//sequoiadb驱动依赖    
```


