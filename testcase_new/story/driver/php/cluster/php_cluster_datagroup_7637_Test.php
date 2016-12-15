/****************************************************
@description:      test datagroup operate,
@testlink cases:   seqDB-7639/7641
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/lib/ReplicaGroup.php';
include_once Cur_Path.'/../global.php';
class dataGroupTest extends PHPUnit_Framework_TestCase
{
   private static $db;
   private static $catagroup;
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      $err = self::$db -> connect(globalParameter::getHostName(), 
                                  globalParameter::getCoordPort()) ;
      if ( $err['errno'] != 0 )
      {
         echo "Failed to connect database, error code: ".$err['errno'] ;
         self::$skipTestCase = true ;
         return;
      } 
      
      self::$catagroup=new ReplicaGroup(self::$db, "SYSCatalogGroup");
      $err = self::$catagroup->create('r520-2', '30000', '/opt/sequoiadb/database/catalog/30000');
      
   }
   
   public function testCreate()
   {
      $group = new ReplicaGroup(self::$db, "db1");
      $err = $group->create();
      $this->assertEquals( 0, $err['errno'] ) ;
      $name = $group->getName();
      $this->assertEquals( $name, "db1" ) ;
      
      $groupObj = self::$db->getGroup("db1");
      $this->assertEquals( true, !empty($groupObj) ) ;
      $getName = $groupObj->getName();
      $this->assertEquals( $getName, $group->getName()) ;
      return $group;
   }
   
   public function testSelectParameter()
   {
      if (mt_rand(0,1) == 0)
      {
         return array('weight'=> 100);
      }
      else
      {
         return json_encode(array('weight'=> 100));
      }
      
   }
   
   /**
    * @depends testCreate
    * @depends testSelectParameter
    */
   public function testAddNode($group, $options)
   {
      $ret = $group->addNode('r520-2', '11830', '/opt/sequoiadb/database/data/11830', $options);
      $this->assertEquals(0, $ret ) ;
      $node = $group->getNode('r520-2:11830');
      $err = $node->start();
      $this->assertEquals($err, 0);
      
      $nodedb = $node->connect();
      $this->assertEquals(true, !empty($nodedb));
      $cursor = $nodedb->list(SDB_LIST_CONTEXTS_CURRENT);
      $this->assertEquals(true, !empty($cursor));
      return $node;
   }
   
   /**
    * @depends testAddNode
    */
   public function testAddNodeRepeat($node)
   {
      $err = $node->create();
      $this->assertEquals($err, -157);
   }
   
   /**
    * @depends testCreate
    * @depends testAddNode
    */
   public function testRemoveNode($group, $node)
   { 
      $err = $node->stop();
      $nodedb = $node->connect();
      $this->assertEquals(true, empty($nodedb)); 
      $err = $node->drop();
      $this->assertEquals($err, -204); 
      $tmpNode = $group->getNode($node->getHostName().":".$node->getServiceName());
      $this->assertEquals(true, !empty($tmpNode));  
   }
  
   public static function tearDownAfterClass()
   {
      $cursor = self::$db->listGroup() ;
      while($record = $cursor->next()){
         
         if (strcmp($record["GroupName"],"SYSCatalogGroup") )
         {
            self::$db->removeGroup($record["GroupName"]);
         }
      }
      $err = self::$catagroup->drop();
      //$this->assertEquals( 0, $err ) ;
      $err = self::$db->close();
   }
}
?>
