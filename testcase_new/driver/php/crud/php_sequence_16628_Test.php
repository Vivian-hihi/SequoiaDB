/****************************************************
@description:     test auto sequence
@testlink cases:   seqDB-16628
@modify list:
        2018-12-07 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class TestSequence16628 extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16628";
   private static $clName = "cl16628";
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
      $err = self::$cl -> createAutoIncrement( array('Field' => '', 'MaxValue' =>2000));
      self::checkErrno( -6, self::$db -> getError()['errno'] );

      $err = self::$cl -> createAutoIncrement( array('Field' => null, 'MaxValue' =>2000));
      self::checkErrno( -6, self::$db -> getError()['errno'] );

      $err = self::$cl -> createAutoIncrement( array( 'MaxValue' =>2000 ) );
      self::checkErrno( -6, self::$db -> getError()['errno'] );

      $err = self::$cl -> createAutoIncrement( array( 'Field' => 'a' ) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );

      $err = self::$cl -> createAutoIncrement( "{ 'Field':'b' }");
      self::checkErrno( 0, self::$db -> getError()['errno'] );

      self::insertAndCheckRecord();

      $err = self::$cl -> dropAutoIncrement( array( 'Field' => 'c' ) ) ;
      self::checkErrno( -333, self::$db -> getError()['errno'] );

      $err = self::$cl -> dropAutoIncrement( array( 'Field' => '' ) ) ;
      self::checkErrno( -6, self::$db -> getError()['errno'] );

      $err = self::$cl -> dropAutoIncrement( array( 'Field' => null ) ) ;
      self::checkErrno( -6, self::$db -> getError()['errno'] );

      $err = self::$cl -> dropAutoIncrement( array( 'Field' => 'a' ) ) ;
      self::checkErrno( -0, self::$db -> getError()['errno'] );
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
         if( intval($record['a'] -> __toString()) != $times || intval($record['b'] -> __toString()) != $times )
         {
            throw new Exception( "check record error, exp: ". $times ." act: " ."a = ".$record['a'].", b = ".$record['b'] );
         }
         $times++;
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
