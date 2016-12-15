/****************************************************
@description:      deprecated interface, base case
@testlink cases:   seqDB-7719/ seqDB-7720
@input:        1 selectGroup 
               2 getNodeName 
@output:     success
@modify list:
        2016-4-29 XiaoNi Huang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../func.php';

class DepreOperator03 extends BaseOperator 
{  
   public function __construct()
   {
      parent::__construct();
   }
   
   function getErrno()
   {
      $this -> err = $this -> db -> getError();
      return $this -> err['errno'];
   }
   
   function selectGroup ( $groupNames )
   {
      return $this -> db -> selectGroup( $groupNames[0] );
   }
   
   function getMaster ( $rgDB )
   {
      return $rgDB -> getMaster();
   }
   
   function getNodeName( $nodeDB )
   {
      return $nodeDB -> getNodeName();
   }
   
}

class TestDepre03 extends PHPUnit_Framework_TestCase
{
   protected static $dbh;
   private static $groupNames;
   private static $rgDB;
   private static $nodeDB;
   
   public static function setUpBeforeClass()
   {
      self::$dbh = new DepreOperator03();
      
      if( self::$dbh -> commIsStandlone() === false )
      {
         
         self::$groupNames = self::$dbh -> commGetGroupNames();
      }
   }
   
   public function setUp()
   {
      if( self::$dbh -> commIsStandlone() === true )
      {
         $this -> markTestSkipped( "Database is standlone." );
      }
   }
   
   function test_selectGroup()
   {
      echo "\n---Begin to selectGroup.\n";
      self::$rgDB = self::$dbh -> selectGroup( self::$groupNames );
      
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
   function test_getMaster()
   {
      echo "\n---Begin to getMaster.\n";
      self::$nodeDB = self::$dbh -> getMaster( self::$rgDB );
      
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
   function test_getNodeName()
   {
      echo "\n---Begin to getNodeName.\n";
      $name = self::$dbh -> getNodeName( self::$nodeDB );
      
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
      $this -> assertNotEmpty( $name );
   }
}
?>