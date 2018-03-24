/****************************************************
@description:     test analyze inexistent cl
@testlink cases:  seqDB-14240
@modify list:
      2018-03-21  Suqiang Ling init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';
include_once Cur_Path.'/../commlib/analyzeUtils.php';

class TestAnalyzeCl14540 extends PHPUnit_Framework_TestCase
{
   private static $db;
   private static $csNameBase = "analyze14240";
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'. 
                           globalParameter::getCoordPort()) ;
      analyzeUtils::checkErrno( 0, self::$db -> getError()['errno'] );
   }
   
   function test()
   {
      $inexistClFullName = 'foolish_man.foolish_name_xcvebcjd';
      $err = self::$db -> analyze( array( "Collection" => $inexistClFullName ) );
      analyzeUtils::checkErrno( -23, $err['errno'] );
   }
   
   public static function tearDownAfterClass()
   {
      self::$db -> close();
   }
}
?>
