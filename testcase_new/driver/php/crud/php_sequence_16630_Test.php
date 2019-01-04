/****************************************************
@description:     test auto sequence
@testlink cases:   seqDB-16630
@modify list:
        2018-12-07 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class TestSequence16630 extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16630";
   private static $clName = "cl16630";
   private static $fullName = "cs16630.cl16630";
   private static $sequenceName;
   private static $cs;
   private static $cl;
   private static $db;
   private static $skipTest = false;
   private static $beginTime;
   private static $endTime;

   public static function setUpBeforeClass()
   {
      date_default_timezone_set("Asia/Shanghai");
      self::$beginTime = microtime( true );
      echo "\n---Begin time: " . date( "Y-m-d H:i:s", self::$beginTime ) ."\n";
   
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'.
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$skipTest = self::isStandlone();  
      if( self::$skipTest)
      {  
         return;
      }
      self::$cs = self::$db -> selectCS( self::$csName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cl = self::$cs -> selectCL( self::$clName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
   }

   function test()
   {
      if( self::$skipTest)
      {
         return;
      }
      echo "\n---Begin to test bulkInsert.\n";
      $err = self::$cl -> createAutoIncrement( array( 'Field' => 'a', 'MaxValue' => 4096, 'MinValue' => 1024, 'StartValue' => 2048 ) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );

      self::checkSnapshot();

      self::checkList(); 
      
   }
   
   public static function tearDownAfterClass()
   {
      if( self::$skipTest)
      {
         return;
      }
      $err = self::$db -> dropCS( self::$csName );
      if ( $err['errno'] != 0 )
      {
         throw new Exception("failed to drop cs, errno=".$err['errno']);
      }

      self::$db->close();
      
      self::$endTime = microtime( true );
      echo "\n---End the Test,End time: " . date( "Y-m-d H:i:s", self::$endTime ) . "\n";
      echo "\n---Test 16630 spend time: " . ( self::$endTime - self::$beginTime ) . " seconds.\n";
   }
   
   private static function checkErrno( $expErrno, $actErrno, $msg = "" )
   {
      if( $expErrno != $actErrno )
      {
         throw new Exception( "expect [".$expErrno."] but found [".$actErrno."]. ".$msg );
      }
   }

   private static function checkSnapshot( )
   {  
      $cursor = self::$db -> snapshot( SDB_SNAP_CATALOG, array( 'Name' => self::$fullName ) );
      if( empty( $cursor ) )
      {  
         throw new Exception( "check cl sequence error,is empty." );
      }
      $sequenceID;
      while( $record = $cursor -> next() )
      {  
         $sequenceID = $record['AutoIncrement'][0]['SequenceID'];
         self::$sequenceName = $record['AutoIncrement'][0]['SequenceName'];
      }
      $cursor  = self::$db -> snapshot( 15, array( 'ID' => $sequenceID) );
      if( empty( $cursor ) )
      {  
         throw new Exception( "select sequence error,is empty." );
      }
      $record  = $cursor -> next();
      self::checkSnapshotValue( $record['Name'], self::$sequenceName );
      self::checkSnapshotValue( $record['Internal'], true );
      self::checkSnapshotValue( $record['Version'] -> __toString(), '0');
      self::checkSnapshotValue( intval( $record['CurrentValue'] -> __toString() ), 2048 );
      self::checkSnapshotValue( intval( $record['StartValue'] -> __toString() ), 2048 );
      self::checkSnapshotValue( intval( $record['MinValue'] -> __toString() ), 1024 );
      self::checkSnapshotValue( intval( $record['MaxValue'] -> __toString() ), 4096 );
      self::checkSnapshotValue( $record['Increment'], 1 );
      self::checkSnapshotValue( $record['CacheSize'], 1000 );
      self::checkSnapshotValue( $record['AcquireSize'], 1000 );
      self::checkSnapshotValue( $record['Cycled'], false );
      self::checkSnapshotValue( $record['Initial'], true );
      
   }

   private static function checkList()
   {
      $cursor = self::$db -> list( SDB_SNAP_SEQUENCES, array( 'Name' => self::$sequenceName ));
      if( empty( $cursor ) ) 
      {
         throw new Exception( 'list sequence error, is empty.');
      }
   }

   private static function checkSnapshotValue( $expValue, $actValue )
   {
      if( $expValue != $actValue )
      {
         throw new Exception( 'check sequence snapshot error, exp: ' .$expValue . ' act: ' .$actValue );
      }
   }

   private static function isStandlone()
   {
      self::$db -> list( SDB_LIST_GROUPS );
      $error = self::$db -> getError();
      if($error['errno'] === -159 )
      {
         echo "   Is standlone mode!! \n";
         return true;
      }else
      {
         return false;
      }
   }
}

?>
