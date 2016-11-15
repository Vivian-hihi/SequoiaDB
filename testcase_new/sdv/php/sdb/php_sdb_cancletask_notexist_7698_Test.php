/****************************************************
@description:      cancel not exist task
@testlink cases:   seqDB-7698
@modify list:
        2016-6-21 wenjing wang init
****************************************************/

<?php
   define('Cur_Path', dirname(__FILE__));
   include_once Cur_Path.'/lib/task.php';
   include_once Cur_Path.'/lib/comm.php';
class taskTest extends PHPUnit_Framework_TestCase
{
    private static $db;
    private static $task;
    public static function setUpBeforeClass()
    {
       self::$db = new SequoiaDB();
       $err = self::$db -> connect('localhost:11810');
       self::$task = new Task(self::$db);
    }
    
    public function setUp()
    {
       if (common::IsStandlone(self::$db))
       {
          $this->markTestSkipped('database is standlone'); 
       }
    }

    public function testListTaskOfNoTask()
    {
       $ret = self::$task->listAll();
       $this->assertEquals(0, $ret);
    }
    
    /**
     * @depends testListTaskOfNoTask
     *
     */
    public function testCancleOfNotExist()
    {
       $taskID = 1;
       $err = self::$task->cancle($taskID, true);
       $this->assertEquals(-173, $err);

       $err = self::$task->cancle(new SequoiaInt64( '1' ), true);
       $this->assertEquals(-173, $err);
    }

    public static function tearDownAfterClass()
    {
       self::$db->close();
    }
}
?>
