package com.sequoiadb.test;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

public class Constants {

/* use in ci */
//	public final static String HOST                        = "rhel"; // power machine
//	public final static String HOST                        = "192.168.20.202";
	public final static int SERVER                         = 11810;
  	public final static String HOST                        = "192.168.20.46"; // change this for you own mechine (rhel ubutu-dev1)
  	public final static String HOST_NAME				   = "sdbserver1";
  	public final static String BACKUPPATH                  = "/opt/sequoiadb/database/test/backup";
  	
  	public final static String DATAPATH1                   = "/opt/sequoiadb/database/test/data1";
	public final static String DATAPATH2                   = "/opt/sequoiadb/database/test/data2";
	public final static String DATAPATH3                   = "/opt/sequoiadb/database/test/data3";
	public final static String DATAPATH4                   = "/opt/sequoiadb/database/test/data4";
	  
	
	// host/port
	public final static String _HOST                       = HOST;
	public final static int _SERVER                        = SERVER;
	public final static int SERVER1                        = 118500;
	public final static int SERVER2                        = 118600;
	public final static int SERVER3                        = 118700;
	public final static String COOR_NODE_CONN              = HOST+":"+ SERVER;
	public final static String DATA_NODE_CONN              = HOST+":"+"11850";
	public final static String COOR_HOST_NAME              = HOST;
	public final static String CATALOG_HOST_NAME           = HOST;
	public final static int COOR_SERVER_PORT               = 11810;
	public final static int CATALOG_SERVER_PORT            = 11830;
	
	
	// cs
	public final static String TEST_CS_NAME_1              = "testfoo";
	public final static String TEST_CS_NAME_2              = "testCS2";
	public final static String TEST_CS_NAME_3              = "SAMPLE";
	public final static String TEST_CS_NAME_4_128          = "01234567890123456789012345678901234567890123456789012345678901234"
															  + "567890123456789012345678901234567890123456789012345678901234567";
	public final static int	   TEST_CL_CREATE_AMOUNT	   = 5000;
	
	// cl
	public final static String TEST_CL_NAME_1              = "testbar";
	public final static String TEST_CL_NAME_2              = "testCL2";
	public final static String TEST_CL_NAME_3              = "employee";
	public final static String TEST_CL_NAME_4_CN		   = "集合空间";
	public final static String TEST_CL_NAME_5_128          = "01234567890123456789012345678901234567890123456789012345678901234"
															 + "567890123456789012345678901234567890123456789012345678901234567";
	public final static String TEST_CL_NAME_6_DOLLAR	   = "$testCL";
	public final static String TEST_CL_NAME_7_SYS		   = "SYStestCL";
	public final static String TEST_CL_NAME_8_DOT		   = "test.CL";
	public final static String TEST_MAINCL                 = "mainCL";
	public final static String TEST_CL_NAME_11             = "集合";
	
	// node
	public final static String TEST_GROUPNAME              = "group1";
	public final static String TEST_DB_PATH_1			   = "/opt/sequoiadb/database/test/db";
	public final static int    TEST_NODE_PORT			   = 51000;


	
	// rg
	public final static String TEST_RG_NAME				   = TEST_GROUPNAME;
	public final static String TEST_CATA_RG_NAME		   = "catalog" + TEST_GROUPNAME;
	public final static String TEST_CATA_RG_PATH		   = "/opt/sequoiadb/database/test/cataRG";
	public final static String TEST_FIELD_GROUPNAME	       = "GroupName";
	public final static String TEST_FIELD_PRIMARYNODE	   = "PrimaryNode";
	public final static int    TEST_NODE_CREATE_AMOUNT	   = 8;
	public final static String TEST_CATALOG_GROUP_NAME     = "SYSCatalogGroup";
	

	//dataSource
	public final static int TEST_MAX_CONNECTION_NUM		   = 15;
	

	private static Sequoiadb sdb;
	

}
