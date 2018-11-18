/****************************************************
@description:      rename cs
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

class Rename16549 extends PHPUnit_Framework_TestCase
{
   private static $oldCSName = "cs16549old";
   private static $newCSName = "cs16549new";
   private static $clName = "cl16549";
   private static $cs;
   private static $db;
   private static $RenameUtils;
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'. 
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );                     
                           
      self::$cs = self::$db -> selectCS( self::$oldCSName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      self::$cs -> selectCL( self::$clName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      
      self::$RenameUtils = new RenameUtils();
       
   }
   
   function test()
   {
      echo "\n---Begin to rename cs.\n";
      
      self::$db -> renameCS( self::$oldCSName, self::$newCSName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      
      self::$RenameUtils -> checkRenameCS( self::$db, self::$oldCSName, self::$newCSName );
   }
   
   public static function tearDownAfterClass()
   {
      $err = self::$db -> dropCS( self::$newCSName );
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
