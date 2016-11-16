/****************************************************
@description:      test task
@testlink cases:   seqDB-7695-7697
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
    private static $cs;
    private static $cl;
    private static $needtest;
    private static $fullname;
    
    private static function loadData()
    {
       $number = 10000000;
       $batchSize = 1000;
       $docs = array();
       for ($i = 0; $i < $number; $i += $batchSize)
       {
          for ($j = 0; $j < $batchSize ; $j++)
          {
             $doc = array();
             $doc['_id'] = $j;
             $doc['a'] = $j;
             $doc['b'] = 1;
             $doc['d'] = 'zzz';
             self::$cl->insert(doc);
          } 
          
          //self::$cl->bulkInsert($docs);
       }
    }
    
    private static function getSrcGroup($fullname)
    {
       $cursor = self::$db -> snapshot(SDB_SNAP_CATALOG, array('Name' => $fullname));
       while ($record = $cursor->next())
       {
          return $record['CataInfo']['GroupName'];
       }
       
       return "";
    }
    
    private static function getDestGroup($groupname)
    {
        $cursor = self::$db->list(SDB_LIST_GROUPS);
        while ($record = $cursor->next())
        {
           $name = $record['GroupName'];
           if ($name != "SYSSpare" && $name != $groupname &&
               $name != "SYSCatalogGroup" && $name != "SYSCoord")
           {
              break;
           }
        }
        
        return $name;
    }
    
    private static function asyncSplit($srcname, $destname)
    {
        if (empty($srcname) ||
            empty($destname))
        {
           return;
        }
        else
        {
           self::$cl -> splitAsync($srcname, $destname, 50);
           self::$needtest = true;
        }
    }
    
    protected function setUp()
    {
       if (common::IsStandlone(self::$db))
       {
          $this->markTestSkipped('database is standlone'); 
       }
    }
    
    public static function setUpBeforeClass()
    {
       self::$db = new SequoiaDB();
       $err = self::$db -> connect('localhost:11810');
       if (common::IsStandlone(self::$db)) return;
       
       self::$task = new Task(self::$db);
       $rnd = mt_rand(0,1000); 
       $name = 'php_test_'.$rnd;
       $err = self::$db ->createCS($name);
       self::$cs = self::$db -> selectCS($name);
       self::$cl = self::$cs->createCL($name, json_encode(array('ShardingType' => 'hash', 'ShardingKey' =>array('_id' => 1))));
       self::$fullname = $name.'.'.$name;
       self::loadData();
       $srcgroupname = self::getSrcGroup(self::$fullname);
       $destgroupname = self::getDestGroup($srcgroupname);
       
       self::asyncSplit($srcgroupname, $destgroupname);
    }

    public function testSelectParameter()
    {
       $selector = mt_rand(0,2);
       return $selector;
    }
    
    public function testlist()
    {
       if (!self::$needtest) return;
       $ret = self::$task->listbycondition(json_encode(array('Name' => $fullname)));
       $this->assertEquals(true, $ret);
       
       $ret = self::$task->listbycondition(array('Name' => $fullname));
       $this->assertEquals(true, $ret);
       
    }
    /**
     * @depends testSelectParameter
     *
     */
    public function testWait($selector)
    {
       $taskID = self::$task->getTaskId(array('Name' => $fullname));
       
       if ($selector == 0)
       {
          $ret = self::$task->wait(array($taskID));
       }
       else if ($selector == 1)
       {
          $ret = self::$task->wait(new SequoiaInt64($taskID));
       }
       else
       {
          $ret = self::$task->wait($taskID);
       }
       
       $this->assertEquals(0, $err);
    }
    
    /**
     * @depends testSelectParameter
     *
     */
    public function ntestcancle($selector)
    {
        $taskID = self::$task->getTaskId(array('Name' => $fullname));
        if ($selector == 0)
        {
          $ret = self::$task->cancle($taskID,false);
        }
        else
        {
          $ret = self::$task->cancle(new SequoiaInt64($taskID),true);
        }
        
        $this->assertEquals(0, $err);
    }

    public static function tearDownAfterClass()
    {
       if (common::IsStandlone(self::$db))
       {
          self::$db->close();
          return
       }
       else
       {
          $name = strtok(self::$fullname, '.'); 
          self::$db->dropCS($name);
          self::$db->close();
       }
    }
}
?>


