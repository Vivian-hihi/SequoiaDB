/****************************************************
@description:      forceSession
@testlink cases:   seqDB-7635
@modify list:
        2016-6-13 wenjing wang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';
class forceSessionTest extends PHPUnit_Framework_TestCase
{
    protected $db;
    protected $testdb;
    protected function setUp()
    {
       $this->db = new Sequoiadb();
       $err = $this->db->connect( globalParameter::getHostName() , 
                                globalParameter::getCoordPort() ) ;     
       $this->assertEquals( 0, $err['errno'] , '连接coord失败') ;
       
       $cursor = $this->db->list( SDB_LIST_GROUPS );
       $err = $this->db->getError() ;
       if ( $err['errno'] != -159 ){
          while( $record = $cursor->next() ){
             if ( $record['GroupName'] == "SYSCatalogGroup" ||
                  $record['GroupName'] == "SYSCoord" )
             {
                continue;
             }
             var_dump( $record['Group'][0]['HostName'] );
             var_dump( $record['Group'][0]['Service'][0]["Name"] );
             
             $this->testdb = new Sequoiadb();
             $err = $this->testdb->connect( $record['Group'][0]['HostName'].":".$record['Group'][0]['Service'][0]["Name"] );
             $this->assertEquals( 0, $err['errno'], 
                   '连接'.$record['Group'][0]['HostName'].':'.$record['Group'][0]['Service'][0]["Name"].'失败' ) ;
             break;
          }
       }else{
          $this->assertEquals( -159, $err['errno'], 'list(SDB_LIST_GROUPS) 失败' );
          $this->testdb = $this->db;
       }
    }

    public function testForceSession()
    {
        $sessionID = -1 ;
        $cursor = $this->testdb->list( SDB_LIST_SESSIONS_CURRENT ) ;
        $err = $this->testdb->getError() ;
        $this->assertEquals( 0, $err['errno'], 'list( SDB_LIST_SESSIONS_CURRENT ) 失败' ) ;
        $find = False;
        while( $record = $cursor->next() )
        {
            $sessionID = $record['SessionID'] ;
            $find = True;
        }
        $this->assertEquals( $find, True, 'list(SDB_LIST_SESSIONS_CURRENT) 返回为空' ) ;
 
        $err = $this->testdb->forceSession( $sessionID ) ;
        $this -> assertEquals( -16, $err['errno'], 'forceSession错误' );
    }

    protected function tearDown()
    {
        $err = $this->db->close();
        $this->assertEquals( 0, $err['errno'] ) ;
    }
}
?>
