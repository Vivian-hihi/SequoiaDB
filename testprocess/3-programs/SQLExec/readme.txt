project SQLExec:
	to run sqlfiles.

run commond:
java -jar SQLExecTest.jar testcase [test.xml] [logdir] [log4j.properties] [start-sql-number]

	parameters:
		testcase: 		sqlfile to run
		test.xml:		test configuration file. default is test.xml
		logdir:			the dir which xml-log is generated at. default is current path
		log4j.properties:	log4j config file. default is log4j.properties
		start-sql-number:	the number of sql start to be executed. SqlStatements before the sql will not be execute.
					default value is 1.