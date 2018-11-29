系统配置
启动SequoiaS3系统之前需进行如下配置：
1.安装sdb数据库，配套版本3.0.0及后续版本
  1.1创建元数据的集合空间，例如：MetaCollectionSpace
  1.2创建对象数据的集合空间，例如：DataCollectionSpace
  1.3在元数据集合空间下，创建用户集合User，并对ID，Name，AccessKeyID创建唯一索引
  1.4在元数据集合空间下，创建桶集合BucketList，并对ID，Name创建唯一索引
  （可参考createCSCLexample.js安装脚本）
  1.5数据节点下配置transactionon=true，transisolation=1,并重启数据节点
2.参考/config/sample/中application.properties的修改jar包同级的config目录中application.properties配置文件
  2.1修改sdbs3.sequoiadb.url中的数据库IP地址和coord端口，与安装数据库的协调节点IP、端口保
  持一致。
  2.2修改元数据的集合空间名称和对象数据的集合空间，如果使用默认值则不需要修改，确保数据库
  中的集合空间名称与默认配置即可
  2.3修改服务端口号，此端口号为客户端连接sequoias3服务的端口号
  （可参考注释样例）
3.拷贝/config/sample/中logback.xml到config目录中，使用默认配置即可，

系统启动
配置修改完成后
1.在jar包所在目录执行sequoias3.sh start启动S3服务
2.也可以手动执行命令启动：java -jar sequoia-s3-xxx.jar

系统使用：
1.系统启动时需确保数据库已经正常运行，系统启动后会创建默认管理员用户，
您可以使用该默认管理员用户创建新的用户，然后使用新用户角色进行桶操作，
以及后续实现的对象操作功能。
默认管理员账户名：administrator
默认管理员AccessKeyID：ABCDEFGHIJKLMNOPQRST
设置Authorization的Header值为Credential=ABCDEFGHIJKLMNOPQRST/20180524/china/s3/aws4_request

错误排查：
1.如果系统启动后执行任何命令都出现“GetDBConnectFail”错误，需检查application.properties中
sdbs3.sequoiadb.url的IP、端口配置是否与数据库配置相匹配

2.如果初次启动系统出现1中的数据库连接错误，默认管理员用户创建失败，修正数据库连接错误后，
需重新启动系统以触发创建默认管理员用户操作。如出现“Collection space does not exist”或
“Collection does not exist”此类错误，请检查数据库中集合空间名称是否与配置文件中的集合空
间名称一致，用户集合名称是否为User，桶集合名称是否为BucketList


