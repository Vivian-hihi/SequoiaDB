/****************************************************
@description:      rename cl
@testlink cases:   seqDB-16553
@input:        rename cs
@output:     success
@modify list:
        2018-11-13 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';
include_once Cur_Path.'/../commlib/RenameUtils.php';

class Rename16553 extends PHPUnit_Framework_TestCase
{
   private static $csName = 'cs16553';
   private static $oldCLName = 'cl16549old';
   private static $newCLName = 'cl16549new';
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
      self::$cs -> selectCL( self::$oldCLName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      
      self::$RenameUtils = new RenameUtils();
   }
   
   function test()
   {
      echo "\n---Begin to rename cs.\n";
      
      self::$cs -> renameCL( self::$oldCLName, self::$newCLName );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      
      self::$RenameUtils -> checkRenameCL( self::$db, self::$csName,  self::$oldCLName, self::$newCLName );
   }
   
   public static function tearDownAfterClass()
   {
      $err = self::$db -> dropCS( self::$csName );
      if ( $err['errno'] != 0 )
      {
         throw new Exception('failed to drop cs, errno='.$err['errno']);
      }
      echo "\n---End of the test.\n";
      self::$db->close();
   }
   
   private static function checkErrno( $expErrno, $actErrno, $msg = '' )
   {
      if( $expErrno != $actErrno ) 
      {
         throw new Exception( 'expect ['.$expErrno.'] but found ['.$actErrno.']. '.$msg );
      }
   }
   
}
?>
