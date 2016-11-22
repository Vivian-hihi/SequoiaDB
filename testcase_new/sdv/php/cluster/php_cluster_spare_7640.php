/****************************************************
@description:      test spare group operate
@testlink cases:   seqDB-7639
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/lib/ReplicaGroupMgr.php';

class spareRGTest extends PHPUnit_Framework_TestCase
{
   private static $db;
   private static $group;
   private static $node;
   public static function setUpBeforeClass()
   {
      echo "enter setUpBeforeClass\n";
      self::$db = new Sequoiadb();
      $err = self::$db -> connect(globalParameter::getHostName(), 
                                  globalParameter::getCoordPort()) ;
      if ( $err['errno'] != 0 )
      {
         echo "Failed to connect database, error code: ".$err['errno'] ;
         self::$skipTestCase = true ;
         return;
      } 
      self::$db->setSessionAttr(array('PreferedInstance' => 'm' )) ; 
      self::$group = new ReplicaGroup(self::$db, "SYSSpare");
      echo "leave setUpBeforeClass\n";
   }
   
   
   public function testSelectParameter()
   {
      $option = array('KeepData'=> true);
      if (mt_rand(0,1) == 0)
      {
         return $option;
      }
      else
      {
         return json_encode($option);
      }
      
   }
   
   private function findNode($nodes, $node)
   {
      for ($i = 0; $i < count($nodes); ++$i)
      { 
         $tmp = $nodes[$i];
         if ($tmp->getName() == $node->getName())
         {
            return true;
         }
      }
      
      return false;
   }
   
   /**
    * @depends testSelectParameter
    *
    */
   public function testDetachNode($options)
   {
      echo "enter testDetachNode \n";
      $err = self::$group->create();
      $this->assertEquals($err, 0) ;
      $err = self::$group->addNode("r520-2", "26070", '/data/disk1/sequoiadb/database/data/26070');
      $this->assertEquals($err, 0) ;
      echo "addNode successful\n"; 
      self::$node = self::$group->getNode("r520-2:26070");
      $this->assertEquals(true, !empty(self::$node)) ;
      
      $err = self::$node->start();
      $this->assertEquals($err, 0) ;
      
      $err = self::$group->detachNode(self::$node, $options);
      $this->assertEquals($err, 0) ;
      
      sleep(5);
      $nodes = self::$group->getNodes();
      //$this->assertEquals($this->findNode($nodes, self::$node), false); 
      
    
      return self::$node;
      
   }
   
   /**
    * @depends testSelectParameter
    * @depends testDetachNode 
    */
   public function testAttachNode($options, $node)
   {
      echo "enter testAttachNode\n";
      $groupMgr = new ReplicaGroupMgr(self::$db);
      $groups = $groupMgr->getDataGroups();
      
      $pos = mt_rand(0,count($groups)-1);
      
      $group = $groups[$pos];
      $nodeNum = $group->getNodeNum();
      
      $err = $group->attachNode($node, $options);
      $this->assertEquals($err, 0) ;
      //$this->assertEquals($nodeNum+1, $group->getNodeNum());
      $nodes = $group->getNodes();
      $this->assertEquals($this->findNode($nodes, $node), true);
      
   }

   public static function tearDownAfterClass()
   {
      //$err = self::$node->drop();
      //$err = self::$group->drop();
      $err = self::$db->close();
   }
}
?>
