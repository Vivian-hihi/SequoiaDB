/****************************************************
@description:      setSessionAttr
@testlink cases:   seqDB-7703 
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php

include_once dirname(__FILE__).'/../global.php';
class setSessionAttrTest extends PHPUnit_Framework_TestCase
{
    protected $db;
    protected $cl;
    protected function setUp()
    {
       $this->db = new Sequoiadb() ;
       $err = $this->db->connect( globalParameter::getHostName() , 
                                  globalParameter::getCoordPort() ) ;
       $this->assertEquals( 0, $err['errno'] ) ;
       
       $suffix = uniqid() ;
       $name = globalParameter::getChangedPrefix() ;
       $this->name = $name.$suffix ;
       $cs = $this->db->selectCS( $this->name ) ;
       if ( empty( $cs ) )
       {
          $err = $this->db->createCS( $this->name ) ;
          $this->assertEquals( 0, $err['errno'] ) ;
          $cs = $this->db->selectCS( $this->name ) ;
          if ( empty( $cs ) )
          {
             $err = $this->db->getError() ;
             $this->assertEquals( 0, $err['errno'] ) ;
          }
       }
        
       $this->cl = $cs->selectCL( $this->name ) ;
       if ( empty( $this->cl ) )
       {
          $err = $cs-> createCL( $this->name ) ;
          $this->assertEquals( 0, $err['errno'] ) ;
          $cl = $cs->selectCL( $this->name ) ;
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
        
        $find = false;
        $cursor = $this->cl->find( $doc ) ;
        while( $record = $cursor -> next() ) 
        {
            var_dump( $record ) ;
            $find = true;
        }
        
        $err = $this->db -> getError() ;
        if( $err['errno'] != -29 || !$find ) 
        {
           $this->assertEquals( 0, $err['errno'] ) ;
        }
       
    }
    
    protected function tearDown()
    {
        $err = $this->db->dropCS( $this->name );
        $this->assertEquals( 0, $err['errno'] ) ;
        $err = $this->db->close();
        $this->assertEquals( 0, $err['errno'] ) ;
    }
}
?>
