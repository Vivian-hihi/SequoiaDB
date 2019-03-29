tdd/cpp: rg.cpp用例写死group等参数，手工跑不了，无法上ci

driver/java用例：
com.sequoiadb.auth  ----用例问题，需要修改用例（将localhost转成ip地址），责任人：陈思琴 已调整2017.1.19
com.sequoiadb.clustermanager  ----用例问题，需要修改用例（将localhost转成ip地址） 责任人：赵育  已调整2017.1.19

story/java用例：
com.sequoiadb.metadataConsistency.cluster  ---环境未清理干净/创建节点达到最大上限个数导致用例失败，待调整用例，责任人：黄晓妮

driver/python用例：用例中将ip地址写死了，责任人：吴艳
ssl_9650.py 
ssl_9651.py
index_9477.py

sdv/js/ssl用例：commlib中写死了ip地址，用例待修改，责任人：吴艳  已修改2017.1.21（余婷）
sdv_datasync_015_adddatanode.js  ---虚拟机同步太慢导致用例失败，是否修改用例？责任人：王文净
