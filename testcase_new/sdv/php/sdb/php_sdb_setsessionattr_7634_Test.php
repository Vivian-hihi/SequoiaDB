/****************************************************
@description:      setSessionAttr
@testlink cases:   seqDB-7703 
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php
class setSessionAttrTest extends PHPUnit_Framework_TestCase
{
    protected $db;
    protected $cl;
    protected function setUp()
    {
       $this->db = new Sequoiadb();
       $err = $this->db->connect( "localhost:11810" );
       $this->assertEquals( 0, $err['errno'] ) ;
       
       $suffix = uniqid();
       $cs = $this->db->selectCS('foo'.$suffix);
       if (empty($cs))
       {
          $err = $this->db->createCS('foo'.$suffix) ;
          $this->assertEquals( 0, $err['errno'] ) ;
          $cs = $this->db->selectCS('foo'.$suffix);
          if (empty($cs))
          {
             $err = $this->db->getError();
             $this->assertEquals( 0, $err['errno'] ) ;
          }
       }
        
       $this->cl = $cs->selectCL('bar'.$suffix);
       if (empty($this->cl))
       {
          $err = $cs-> createCL( 'bar'.$suffix);
          $this->assertEquals( 0, $err['errno'] ) ;
          $cl = $cs->selectCL('bar');
          if (empty($cl))
          {
             $err = $this->db->getError();
             $this->assertEquals( 0, $err['errno'] ) ;
          }
       }
    }
    
    public function testSetSessionAttr()
    {
        $err = $this->db->setSessionAttr( array( 'PreferedInstance' => 'm' ) ) ;
        $this->assertEquals( 0, $err['errno'] ) ;
        
        $key = "_id";
        $id = uniqid();
        $doc = vsprintf('{%s:"%s"}', array($key,$id));
       
        var_dump($doc);
        $err = $this->cl->insert( $doc);
        $this->assertEquals( 0, $err['errno'] ) ;
        
        $cursor = $this->cl -> find( $doc ) ;
        if( empty( $cursor ) ) 
        {
            $err = $db -> getError() ;
            $this->assertEquals( 0, $err['errno'] ) ;
            $this->assertFalse(empty($cursor));
        }
        
        while( $record = $cursor -> next() ) 
        {
            var_dump( $record ) ;
        }
       
    }
    
    protected function tearDown()
    {
        $err = $this->db->dropCS('foo');
        $err = $this->db->close();
        $this->assertEquals( 0, $err['errno'] ) ;
    }
}
?>