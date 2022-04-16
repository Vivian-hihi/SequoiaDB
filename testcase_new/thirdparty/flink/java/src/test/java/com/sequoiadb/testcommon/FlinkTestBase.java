package com.sequoiadb.testcommon;

import org.testng.annotations.*;

public class FlinkTestBase {
    protected static String username;
    protected static String password;
    private static String coords_option;
    protected static String[] coords;
    private static String mysql_address;
    private static String mysql_username;
    private static String mysql_password;
    protected static final String jdbc_test_database_name = "jdbc_test";
    protected static final String sdb_test_database_name = "sdb_test";

    @Parameters({ "COORDS", "USERNAME", "PASSWORD", "MYSQLADDRESS",
            "MYSQLUSERNAME", "MYSQLPASSWORD" })
    @BeforeSuite(alwaysRun = true)
    public static void initSuite( String COORDS, String USERNAME,
            String PASSWORD, String MYSQLADDRESS, String MYSQLUSERNAME,
            String MYSQLPASSWORD ) {
        System.out.println( "initSuite....." );
        coords_option = COORDS;
        username = USERNAME;
        password = PASSWORD;
        mysql_address = MYSQLADDRESS;
        mysql_username = MYSQLUSERNAME;
        mysql_password = MYSQLPASSWORD;
        splitCoordsToArray();
    }

    @BeforeTest()
    public static synchronized void initTestGroups() {
    }

    @AfterTest()
    public static synchronized void finiTestGroups() {
    }

    @AfterSuite()
    public static void finiSuite() {
    }

    private static void splitCoordsToArray() {
        if ( coords_option.indexOf( "," ) > 0 ) {
            coords = coords_option.split( "," );
        } else {
            coords = new String[] { coords_option };
        }
    }

    protected static String getCoord() {
        int random_index = ( int ) ( Math.random() * coords.length );
        return coords[ random_index ];
    }

    protected static String getCoords() {
        return coords_option;
    }

    public static String getMysqlAddress() {
        return mysql_address;
    }

    public static String getMysqlUsername() {
        return mysql_username;
    }

    public static String getMysqlPassword() {
        return mysql_password;
    }
}
