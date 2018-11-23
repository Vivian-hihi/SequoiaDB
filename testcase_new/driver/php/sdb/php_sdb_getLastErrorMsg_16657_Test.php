/****************************************************
@description:      check error msg
@testlink cases:   seqDB-16657
@modify list:
        2018-11-19 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class GetLastErrMsg16657 extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16657";
   private static $clName = "cl16657";
   private static $cl;
   private static $db;
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'. 
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );                     
                           
      $cs = self::$db -> selectCS( self::$csName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cl = $cs -> selectCL( self::$clName );	
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cl -> createIndex( array( 'a' => 1), "myIndex", true );
   }
   
   function test()
   {
      echo "\n---Begin to check getError.\n";
      self::$cl -> insert( array( 'a' => 1) );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cl -> insert( array( 'a' => 1) );
      $errMsg1 = self::$db -> getLastErrorMsg();
      self::checkErrorMsg( $errMsg1, -38, "insert the same record");
      self::$db -> getCL("text.text");
      $errMsg2 = self::$db -> getLastErrorMsg();
      self::checkErrorMsg( $errMsg2, -23, "get not exist cl");
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
   
   private static function checkErrno( $expErrno, $actErrno, $msg = "" )
   {
      if( $expErrno != $actErrno ) 
      {
         throw new Exception( "expect [".$expErrno."] but found [".$actErrno."]. ".$msg );
      }
   }

   private static function checkErrorMsg( $errorMsg, $expErrno, $msg = "" )
   {
      $actErrno = $errorMsg['errno'];
      if( $actErrno != $expErrno )
      {
         throw new Exception( "expect [".$expErrno."] but found [".$actErrno."]. ".$msg );
      }
   }
   
}
?>
