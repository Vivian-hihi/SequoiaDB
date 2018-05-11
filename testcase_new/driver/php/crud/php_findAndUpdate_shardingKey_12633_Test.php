/****************************************************
@description:      update shardingKey
@testlink cases:   seqDB-12633
@modify list:
        2017-11-28 xiaoni huang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../func.php';

class UpdateShardingKey12633 extends BaseOperator 
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
   
   function createCL( $csName, $clName )
   {
      $options = array( 'ShardingKey' => array( 'a' => 1 ), 'ShardingType' => 'range', 
                        'ReplSize' => 0 );
      return $this -> commCreateCL( $csName, $clName, $options, true );
   }
   
   function insertRecs( $clDB )
   {
      $recs = array( 'a' => 1, 'b' => 1 );
      $clDB -> insert( $recs );
   }
   
   function findAndUpdateRecs( $clDB )
   {
      $rule = array( '$inc' => array( 'a' => 1, 'b' => 1 ) );
      $cond = array( 'a' => 1 );
      $cursor = $clDB -> findAndUpdate( $rule, $cond, null, null, null, 0, -1, SDB_FLG_UPDATE_KEEP_SHARDINGKEY );
      $errno = $this -> getErrno();
      if( $errno !== -178 )
      {
         echo "\nFailed to findAndUpdate. Errno: ". $errno ."\n";
      }
      /*
      while( $tmpInfo = $cursor -> next() )
      {
         //ok
      }*/
   } 
   
   function findRecs( $clDB )
   {
      $findRecsArray = array() ;
      $cursor = $clDB -> find();
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

class TestUpdateShardingKey12633 extends PHPUnit_Framework_TestCase
{
   protected static $dbh;
   private static $csName;
   private static $clName;
   private static $clDB;
   
   public static function setUpBeforeClass()
   {
      self::$dbh = new UpdateShardingKey12633();
      
      echo "\n---Begin to ready parameter.\n";
      self::$csName = self::$dbh -> COMMCSNAME;
      self::$clName = self::$dbh -> COMMCLNAME.'12633';
      
      echo "\n---Begin to drop cl in the begin.\n";
      self::$dbh -> dropCL( self::$csName, self::$clName, true );
      
      echo "\n---Begin to create cl.\n";
      self::$clDB = self::$dbh -> createCL( self::$csName, self::$clName );
   }
   
   function test_insert()
   {
      echo "\n---Begin to insert records.\n";
      
      self::$dbh -> insertRecs( self::$clDB );
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( 0, $errno );
   }
   
   function test_findAndUpdate()
   {
      echo "\n---Begin to find And update records.\n";
      
      self::$dbh -> findAndUpdateRecs( self::$clDB );
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( -178, $errno );
   }
   /*
   function test_find()
   {
      echo "\n---Begin to find records after update.\n";
      
      $recsArray = self::$dbh -> findRecs( self::$clDB );
      
      $errno = self::$dbh -> getErrno();
      $this -> assertEquals( -29, $errno );
      
      $this -> assertCount( 1, $recsArray );
      $expValue = 2;  
      $this -> assertEquals( $expValue, $recsArray[0]['a'] );
      $this -> assertEquals( $expValue, $recsArray[0]['b'] );
   }*/
   
   public static function tearDownAfterClass()
   {
      echo "\n---Begin to drop cl in the end.\n";      
      self::$dbh -> dropCL( self::$csName, self::$clName, false );
   }
   
}
?>