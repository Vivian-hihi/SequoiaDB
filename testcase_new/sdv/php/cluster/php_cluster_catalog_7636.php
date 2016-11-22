/****************************************************
@description:      test catalog group 
@testlink cases:   seqDB-7636
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/lib/ReplicaGroup.php';
class cataLogGroupTest extends PHPUnit_Framework_TestCase 
{
   private $db;
   public function setUp()
   {
      $this->db = new Sequoiadb();
      $err = self::$db -> connect(globalParameter::getHostName(), 
                                  globalParameter::getCoordPort()) ;
      if ( $err['errno'] != 0 )
      {
         echo "Failed to connect database, error code: ".$err['errno'] ;
         self::$skipTestCase = true ;
         return;
      } 
      $this->assertEquals( 0, $err['errno'] ) ;
   }
   
   public function testSelectParameter()
   {
      if (mt_rand(0,1) == 0)
      {
         return array('auth'=> true);
      }
      else
      {
         return json_encode(array('auth'=> true));
      }
      
   }
   
   /**
    * @depends testSelectParameter
    *
    */
   public function testCreateByfullParameters($options)
   {
      echo "testCreateByfullParameters";
      $group = new ReplicaGroup($this->db, "SYSCatalogGroup");
      $err = $group->create('r520-2', '30000', '/opt/sequoiadb/database/catalog/30000', $options);
      
      $this->assertEquals( 0, $err ) ;
      
      $nodes = $group->getNodes();
      $this->assertEquals( 1,  count($nodes)) ;
      $node = $nodes[0];
      $this->assertEquals( 'r520-2',  $node->getHostName()) ;
      $this->assertEquals( '30000',  $node->getServiceName()) ;
      
      $totalSleepTime = 20;
      $alreadySleepTime = 0;
      $nodedb = $node->connect();
      while(empty($nodedb)){
         sleep(1);
         $alreadySleepTime +=1;
         $ret = $node->connect();
         if ($alreadySleepTime > $totalSleepTime) 
            break;
      }
      $this->assertEquals( true,  !empty($nodedb)) ;
      $this->assertEquals(true,  $group->isCataLog()) ;
      $err = $group->stop();
      $this->assertEquals( $err,  0) ;
      
      $nodedb = $node->connect();
      $this->assertEquals( true, empty($nodedb)) ;
      
      $err = $group->start();
      $this->assertEquals( $err,  0) ;
      
      $nodedb = $node->connect();
      $this->assertEquals( true,  !empty($nodedb)) ;
      
      $err = $group->drop();
      $this->assertEquals( $err,  0) ;
      
      $tmp = $this->db->getGroup("SYSCatalogGroup");
      $err = $this->db->getError();
      $this->assertEquals( $err['errno'],  -180) ;
      
   }
   
   public function testCreateBymustParameters()
   {
      echo "testCreateBymustParameters";
      $group = new ReplicaGroup($this->db, "SYSCatalogGroup");
      $err = $group->create('r520-2', '11820', '/opt/sequoiadb/database/catalog/11820');
      $this->assertEquals( 0, $err ) ;
      
      $nodes = $group->getNodes();
      $this->assertEquals( 1,  count($nodes)) ;
      $node = $nodes[0];
      $this->assertEquals( 'r520-2',  $node->getHostName()) ;
      $this->assertEquals( '11820',  $node->getServiceName()) ;
      
      $nodedb = $node->connect();
      $this->assertEquals( true,  !empty($nodedb)) ;
      
      $this->assertEquals(true,  $group->isCataLog()) ;
      $err = $group->drop();
      $this->assertEquals( $err,  0) ;
   }
   
   protected function tearDown()
   {
      $groupObj = $this->db->getGroup("SYSCatalogGroup");
      if (!empty($groupObj)){
         $this->db->removeGroup("SYSCatalogGroup");
      }
      $err = $this->db->close();
      $this->assertEquals( 0, $err['errno'] ) ;
   }
}
?>
