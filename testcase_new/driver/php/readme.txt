CI测试：
cluster下的用例因为会增删节点，因此暂时屏蔽不测。
sdb下的用例sdb_backuptest_7701_2.php备份整个集群，暂时屏蔽不测。
sdb下的用例php_sdb_cancletask_notexist_7695_7.php数据量较大，暂时屏蔽不测。
ci测试时指定文件夹而不是具体文件，因此只能跑以Test.php结尾的用例。
如authenticateTest.php在ci上会执行，但是authenticate.php不会执行