/****************************************************
@description:     test timestamp increment
@testlink cases:   seqDB-16646
@modify list:
        2018-11-19 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class TestTimestamp extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16646";
   private static $clName = "cl16646";
   private static $cs;
   private static $cl;
   private static $db;

   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'.
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cs = self::$db -> selectCS( self::$csName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cl = self::$cs -> selectCL( self::$clName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
   }

   function test()
   {
      echo "\n---Begin to test timestamp.\n";

      $maxTime = 2147483647;
      $minTime = -2147483648;
      self::$cl -> insert( array( 'time' => new SequoiaTimestamp( $maxTime, -1 ), 'No' => 1) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::checkIncResult( '2038-01-19-11.14.06.999999', 1);

      self::$cl -> insert( array( 'time' => new SequoiaTimestamp( $maxTime, 1000000 ), 'No' => 2) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::checkIncResult( '1901-12-14-04.45.52.000000', 2);

      self::$cl -> insert( array( 'time' => new SequoiaTimestamp( $minTime, -1 ), 'No' => 3) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::checkIncResult( '2038-01-19-11.14.07.999999', 3);

      self::$cl -> insert( array( 'time' => new SequoiaTimestamp( $minTime, 1000000 ), 'No' => 4) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::checkIncResult( '1901-12-14-04.45.53.000000', 4);
   }
   
   public static function tearDownAfterClass()
   {
      $err = self::$db -> dropCS( self::$csName );
      if ( $err['errno'] != 0 )
      {
         throw new Exception("failed to drop cs, errno=".$err['errno']);
      }

      self::$db->close();
   }

   function checkIncResult( $times, $No)
   {
      $cursor = self::$cl -> find( array( 'No' => $No ));
      if( empty( $cursor ))
      {
          throw new Exception( " no find record where No = ".$No );
      }
      while( $record = $cursor -> next() )
      {
         $actTime = $record[ 'time'];
         if( $actTime != $times )
         {
            throw new Exception( "check " . $No . " value error, exp: " . $times ." act: " . $actTime );
         }
      }
   }

   private static function checkErrno( $expErrno, $actErrno, $msg = "" )
   {
      if( $expErrno != $actErrno )
      {
         throw new Exception( "expect [".$expErrno."] but found [".$actErrno."]. ".$msg );
      }
   }
}

?>
