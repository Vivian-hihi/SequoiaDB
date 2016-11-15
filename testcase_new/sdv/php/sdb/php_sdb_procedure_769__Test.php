/****************************************************
@description:      Procedure test
@testlink cases:   seqDB-7693/7694
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php
   define('Cur_Path', dirname(__FILE__));
   include_once Cur_Path.'/lib/Procedure.php';
   include_once Cur_Path.'/lib/comm.php';
class ProcedureTest extends PHPUnit_Framework_TestCase
{
   private static $db;
   private static $procedure;
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      $err = self::$db->connect( "localhost:11810" );
     
      self::$procedure = new Procedure(self::$db);
   }
   
   public function setUp()
   {
      if (common::IsStandlone(self::$db))
      {
         $this->markTestSkipped('database is standlone'); 
      }
   }
   
   public function testCreate()
   {
      $err = self::$procedure->create('function sum( a,b ){ return a + b ; }');
      $this->assertEquals( 0, $err) ;
      
      $result = self::$procedure->exec('sum(1,2)');
      $this->assertEquals( 3, $result) ;
      
      $ret = self::$procedure->listbyname('sum');
      $this->assertEquals( true, $ret) ;
   }
   
   /**
    * @depends testCreate
    */
   public function testRemove()
   {
      $err = self::$procedure->remove('sum');
      $this->assertEquals( 0, $err) ;
      
      $err = self::$procedure->exec('sum(1,2)');
      $this->assertEquals( -152, $err) ;
   }
   
   /**
    * @depends testRemove
    */
   public function testList()
   {
      $ret = self::$procedure->listbyname('sum');
      $this->assertEquals( false, $ret) ;
   }
   
   public static function tearDownAfterClass()
   {
      self::$db->close();
   }
}
?>

