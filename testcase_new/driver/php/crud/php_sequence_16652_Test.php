/****************************************************
@description:     test auto sequence
@testlink cases:   seqDB-16652
@modify list:
        2018-12-07 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class TestSequence16652 extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16652";
   private static $clName = "cl16652";
   private static $cs;
   private static $cl;
   private static $db;
   private static $data;
   private static $skipTest = false;

   public static function setUpBeforeClass()
   {
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
      self::$cl = self::$cs -> selectCL( self::$clName, array( 'AutoIncrement' => array( 'Field' => 'a', 'MaxValue' => 20000 ) ) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::checkSnapshot( 20000 );
   }

   function test()
   {
      if( self::$skipTest)
      {
         return;
      }
      echo "\n---Begin to test bulkInsert.\n";
      self::$cl -> alter(  array( 'AutoIncrement' => array( 'Field' => 'a', 'MaxValue' => 1000 ) ) ) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::checkSnapshot( 1000 );

      //insert 30 record
      for( $i = 0; $i < 1000; $i++ )
      {
         self::$cl -> insert( array( 'c' => i ) );
         self::checkErrno( 0, self::$db -> getError()['errno'] );
      }

      //insert more than 30 records
      self::$cl -> insert( array( 'c' => i ) );
      self::checkErrno( -325, self::$db -> getError()['errno'] );
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
   }
   
   private static function checkErrno( $expErrno, $actErrno, $msg = "" )
   {
      if( $expErrno != $actErrno )
      {
         throw new Exception( "expect [".$expErrno."] but found [".$actErrno."]. ".$msg );
      }
   }

   private static function insertAndCheckRecord()
   {
      for( $i = 0; $i < 1000; $i++ )
      {
         self::$cl -> insert( array( "str" => "test increment" . $i) );
      }
      self::checkErrno( 0, self::$db -> getError()['errno'] );

      $cursor = self::$cl -> find( null, null, array( 'a' => 1 ) );
      if( empty( $cursor ) ) 
      {
         throw new Exception( "find error, cl is empty" );
      }
      $times = 1;
      while( $record = $cursor -> next() )
      {
         if( settype($record['a'], 'int') != $times || settype($record['b'], 'int') != $times )
         {
            throw new Exception( "check record error, exp: ". $times ." act: " ."a = ".$record['a'].", b = ".$record['b'] );
         }
         $times++;
      }
   }

   private static function checkSnapshot( $maxValue )
   {
      $fullName = self::$csName . '.' . self::$clName;
      $cursor = self::$db -> snapshot( SDB_SNAP_CATALOG, array( 'Name' => $fullName ) );
      if( empty( $cursor ) )
      {
         throw new Exception( "check cl sequence error,is empty." );
      }
      $sequenceID;
      while( $record = $cursor -> next() )
      {
         $sequenceID = $record['AutoIncrement'][0]['SequenceID'];
      }
      $cursor  = self::$db -> snapshot( 15, array( 'ID' => $sequenceID) );
      if( empty( $cursor ) )
      {
         throw new Exception( "select sequence error,is empty." );
      }
      $actMaxValue = $cursor -> next()['MaxValue'];
      if( $maxValue != settype($actMaxValue, 'int') )
      {
         throw new Exception( "check sequence maxValue, exp: ". $maxValue ." act: " . $actMaxValue );
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
