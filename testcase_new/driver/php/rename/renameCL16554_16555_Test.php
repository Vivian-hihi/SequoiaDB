/****************************************************
@description:      rename cs, old cs name not exist
@testlink cases:   seqDB-16554 seqDB-16555
@output:     success
@modify list:
        2018-11-13 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class Rename16554_16555 extends PHPUnit_Framework_TestCase
{
   private static $csName = 'cs16554';
   private static $clName1 = 'cl16554_1';
   private static $clName2 = 'cl16554_2';
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
      self::$cs -> selectCL( self::$clName1 );
      self::$cs -> selectCL( self::$clName2 );
      self::checkErrno( 0, self::$db -> getError()['errno'] );
      
   }
   
   function test()
   {
      echo "\n---Begin to rename cs.\n";
      
      self::$cs -> renameCL( 'clNameNotExist', self::$clName1 );
      self::checkErrno( -23, self::$db -> getError()['errno'] );
      
      self::$cs -> renameCL( self::$clName1, self::$clName1 );
      self::checkErrno( -22, self::$db -> getError()['errno'] );
      
      self::$cs -> renameCL( self::$clName1, self::$clName2 );
      self::checkErrno( -22, self::$db -> getError()['errno'] );
      
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
