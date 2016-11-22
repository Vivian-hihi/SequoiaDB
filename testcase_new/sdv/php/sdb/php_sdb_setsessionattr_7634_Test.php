/****************************************************
@description:      setSessionAttr
@testlink cases:   seqDB-7703 
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';
class setSessionAttrTest extends PHPUnit_Framework_TestCase
{
    protected $db;
    protected $cl;
    protected function setUp()
    {
       $this->db = new Sequoiadb() ;
       $err = self::$db->connect( globalParameter::getHostName() , 
                                  globalParameter::getCoordPort() ) ;
       $this->assertEquals( 0, $err['errno'] ) ;
       
       $suffix = uniqid() ;
       $name = globalParameter::getChangedPrefix() ;
       $cs = $this->db->selectCS( $name.$suffix ) ;
       if ( empty( $cs ) )
       {
          $err = $this->db->createCS( $name.$suffix ) ;
          $this->assertEquals( 0, $err['errno'] ) ;
          $cs = $this->db->selectCS( $name.$suffix ) ;
          if ( empty( $cs ) )
          {
             $err = $this->db->getError() ;
             $this->assertEquals( 0, $err['errno'] ) ;
          }
       }
        
       $this->cl = $cs->selectCL( $name.$suffix ) ;
       if ( empty( $this->cl ) )
       {
          $err = $cs-> createCL( $name.$suffix ) ;
          $this->assertEquals( 0, $err['errno'] ) ;
          $cl = $cs->selectCL( $name.$suffix ) ;
          if (empty($cl))
          {
             $err = $this->db->getError() ;
             $this->assertEquals( 0, $err['errno'] ) ;
          }
       }
    }
    
    public function testSetSessionAttr()
    {
        $err = $this->db->setSessionAttr( array( 'PreferedInstance' => 'm' ) ) ;
        $this->assertEquals( 0, $err['errno'] ) ;
        
        $key = "_id" ;
        $id = uniqid() ;
        $doc = vsprintf( '{%s:"%s"}', array( $key, $id ) ) ;
       
        var_dump( $doc );
        $err = $this->cl->insert( $doc );
        $this->assertEquals( 0, $err['errno'] ) ;
        
        $cursor = $this->cl->find( $doc ) ;
        if( empty( $cursor ) ) 
        {
            $err = $db -> getError() ;
            $this->assertEquals( 0, $err['errno'] ) ;
            $this->assertFalse( empty( $cursor ) );
        }
        
        while( $record = $cursor -> next() ) 
        {
            var_dump( $record ) ;
        }
       
    }
    
    protected function tearDown()
    {
        $err = $this->db->dropCS( 'foo' );
        $err = $this->db->close();
        $this->assertEquals( 0, $err['errno'] ) ;
    }
}
?>