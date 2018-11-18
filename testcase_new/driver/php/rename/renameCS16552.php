/****************************************************
@description:      rename cs, check new csName
@testlink cases:   seqDB-16549
@input:        rename cs
@output:     success
@modify list:
        2018-11-13 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';
include_once Cur_Path.'/../commlib/RenameUtils.php';

class Rename16552 extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16552old";
   private static $clName = "cl16552";
   private static $cs;
   private static $db;
   private static $RenameUtils;
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'.
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );

      self::$cs = self::$db -> selectCS( self::$csName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cs -> selectCL( self::$clName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );

      self::$RenameUtils = new RenameUtils();
   }
   
   function test()
   {
      echo "\n---Begin to rename cs.\n";
      
      //use string starting with $
      self::$db -> renameCS( self::$csName, "$newCSName" );
      self::checkErrno( -6, self::$db -> getError()['errno'] );
      
      //use string contain .
      self::$db -> renameCS( self::$csName, "new.csName" );
      self::checkErrno( -6, self::$db -> getError()['errno'] );
      
      //use null cs name
      self::$db -> renameCS( self::$csName, "" );
      self::checkErrno( -6, self::$db -> getError()['errno'] );
      
      //use to long string
      $baseStr = "qwertyuiopasdfghjklzxcvbnm";
      $nameStr = "";
      for( $i=0; $i<40; $i++ )
      {
         $nameStr = $nameStr . $baseStr;
      }
      self::$db -> renameCS( self::$csName, $nameStr );
      self::checkErrno( -6, self::$db -> getError()['errno'] );
      
      //use 127 string
      $str = "a";
      $name = "";
      for( $i=0; $i<127; $i++)
      {
         $name = $name . $str;
      }
      self::$db -> renameCS( self::$csName, $name );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$RenameUtils -> checkRenameCS( self::$db, self::$csName, $name );
      self::$db -> renameCS( $name, self::$csName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      return;
      //use string starting with a%@!~=+-^&()
      self::$db -> renameCS( self::$csName, "a%@!~=+-^&()" );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$RenameUtils -> checkRenameCS( self::$db, self::$csName, "a%@!~=+-^&()" );
      self::$db -> renameCS( "a%@!~=+-^&()", self::$csName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      
      //use string starting with $
      self::$db -> renameCS( self::$csName, "SYSnewName" );
      self::checkErrno( -6, self::$db -> getError()['errno'] );
      
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
   
}
?>
