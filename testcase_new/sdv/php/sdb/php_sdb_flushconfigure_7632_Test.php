/****************************************************
@description:      setSessionAttr
@testlink cases:   seqDB-7703 
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php
class flushConfigureTest extends PHPUnit_Framework_TestCase
{
    protected $db;
    protected function setUp()
    {
       $this->db = new Sequoiadb();
       $err = $this->db->connect( "localhost:11810" );
       $this->assertEquals( 0, $err['errno'] ) ;
    }
    
    public function testFlushConfigure()
    {
        $err = $this->db->flushConfigure(array( 'Global' => true ));
        $this->assertEquals( 0, $err['errno'] ) ;
    }
    
    protected function tearDown()
    {
        $err = $this->db->close();
        $this->assertEquals( 0, $err['errno'] ) ;
    }
}
?>