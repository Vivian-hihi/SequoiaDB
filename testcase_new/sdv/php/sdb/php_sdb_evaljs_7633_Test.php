/****************************************************
@description:      evalJs
@testlink cases:   seqDB-7703 
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php
   define('Cur_Path', dirname(__FILE__));
   include_once Cur_Path.'/lib/comm.php';
class evalJsTest extends PHPUnit_Framework_TestCase
{
    protected $db;
    protected function setUp()
    {
       $this->db = new Sequoiadb();
       $err = $this->db->connect( "localhost:11810" );
       $this->assertEquals( 0, $err['errno'] ) ;
       if (common::IsStandlone($this->db)) 
       {
          $this->markTestSkipped('database is standlone'); 
       }
    }
    
    public function testEvalJS()
    {
       $err = $this->db->createJsProcedure( 'function sum( a,b ){ return a + b ; }' ) ;
       $this->assertEquals( 0, $err['errno'] ) ;
       
       $result = $this->db->evalJs( 'sum( 1, 2 );' ) ;
       $this->assertEquals( 3, $result ) ; 
       $err = $this->db->removeProcedure( 'sum' ) ;
       $this->assertEquals( 0, $err['errno'] ) ; 
    }
    
    protected function tearDown()
    {
        $err = $this->db->close();
        $this->assertEquals( 0, $err['errno'] ) ;
    }
}
?>