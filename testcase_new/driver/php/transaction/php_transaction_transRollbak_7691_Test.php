/****************************************************
@description:      transactionBegin / transactionRollback
@testlink cases:   seqDB-7690
@input:        1 transactionBegin
               2 insert
               3 transactionRollback
               4 find
@output:     success
@modify list:
        2016-4-28 XiaoNi Huang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../func.php';

class TransRollback extends BaseOperator 
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
   
   function transAble( $clDB )
   {
      echo "\n---Begin to judge transaction.\n";
	  
      $this -> db -> transactionBegin();
      $clDB -> remove( '{a: "no" }' );
      $errno = $this -> getErrno();
      if( $errno === -253 )
      {
         return false;
      }
      return true;
   }
   
   function transBegin()
   {
      $this -> db -> transactionBegin();
   }
   
   function transRollbak()
   {
      $this -> db -> transactionRollback();
   }
   
   function createCL( $csName, $clName )
   {
      return $this -> commCreateCL( $csName, $clName, null, true );
   }
   
   function insertRecs( $clDB )
   {
      $clDB -> insert( '{a:1}' );
   }
   
   function findRecs( $clDB )
   {
      $cursor = $clDB -> find();
      $errno = $this -> getErrno();
      if( $errno !== 0 )
      {
         echo "\nFailed to find records. Errno: ". $errno ."\n";
      }
      
      $findRecsArray = array() ;
      while( $record = $cursor -> next() )
      {
         array_push( $findRecsArray, $record );
      }
      return $findRecsArray;
   }
   
   function dropCL( $csName, $clName,$ignoreNotExist )
   {
      $this -> commDropCL( $csName, $clName, $ignoreNotExist );
   }
   
}

class TestTransRollback extends PHPUnit_Framework_TestCase
{
   protected static $dbh;
   private static $csName;
   private static $clName;
   private static $clDB;
   
   public static function setUpBeforeClass()
   {
      self::$dbh = new TransRollback();
      
      echo "\n---Begin to ready parameter.\n";
      self::$csName = self::$dbh -> COMMCSNAME;
      self::$clName = self::$dbh -> COMMCLNAME;
      
      echo "\n---Begin to drop cl in the begin.\n";
      self::$dbh -> dropCL( self::$csName, self::$clName, true );
      
      echo "\n---Begin to create cl.\n";
      self::$clDB = self::$dbh -> createCL( self::$csName, self::$clName );
   }   
   
   public function setUp()
   {
      if( self::$dbh -> commIsStandlone() === true )
      {
         $this -> markTestSkipped( "Database is standlone." );
      }
      else if( self::$dbh -> transAble( self::$clDB ) === false )
      {
         $this -> markTestSkipped( "transaction is off." );
      }
   }
   
   function test_transBegin()
   {
      echo "\n---Begin to exec transBegin.\n";
      
      self::$dbh -> transBegin();
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
   function test_insert()
   {
      echo "\n---Begin to insert records.\n";
      
      self::$dbh -> insertRecs( self::$clDB );
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
   function test_findBeforeTrsRollbak()
   {
      echo "\n---Begin to find records before transRollbak.\n";
      
      $findRecsArray = self::$dbh -> findRecs( self::$clDB );
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( -29, $errno );
      
      $this -> assertCount( 1, $findRecsArray );
   }
   
   function test_transRollbak()
   {
      echo "\n---Begin to exec transRollbak.\n";
      
      self::$dbh -> transRollbak();
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
   function test_findAfterTrsRollbak()
   {
      echo "\n---Begin to find records after transRollbak.\n";
      
      $findRecsArray = self::$dbh -> findRecs( self::$clDB );
      
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( -29, $errno );
      
      $this -> assertCount( 0, $findRecsArray );
   }
   
   function test_dropCL()
   {
      echo "\n---Begin to drop cl in the end.\n";
      
      self::$dbh -> dropCL( self::$csName, self::$clName, false );
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
}
?>