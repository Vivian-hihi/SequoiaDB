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
   protected static $db ;
   protected static $err ;
   protected static $nodes ;
   protected $testdb ;

   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb() ;
      self::$err = self::$db->connect( globalParameter::getHostName() ,
                                       globalParameter::getCoordPort() ) ;
      if ( self::$err['errno'] != 0 )
      {
         return ;
      }
      
      $cursor = self::$db->list( SDB_LIST_GROUPS );
      self::$err = self::$db->getError() ;
      self::$nodes = array() ;
      if ( self::$err['errno'] == 0 )
      {
         while( $record = $cursor->next() )
         {
            if ( $record['GroupName'] == "SYSCatalogGroup" ||
                 $record['GroupName'] == "SYSCoord" )
            {
               continue;
            }
            array_push( self::$nodes, 
                $record['Group'][0]['HostName'].":".$record['Group'][0]['Service'][0]["Name"] ) ;
         }
      }
   }
   
   protected function setUp()
   {
      if ( self::$err['errno'] != 0 )
      {
         $this->markTestSkipped( self::$err['errno'] ) ;
      }
      
      $this->testdb = new Sequoiadb();
      $err = $this->testdb->connect( self::$nodes[0] );
      $this->assertEquals( 0, $err['errno'],
                   '连接'.self::$nodes[0].'失败' ) ;
   }
    
   public function testForceSession()
   {
      $sessionID = -1 ;
      $cursor = $this->testdb->list( SDB_LIST_SESSIONS_CURRENT ) ;
      $curerr = $this->testdb->getError() ;
      $this->assertEquals( 0, $curerr['errno'], 'list( SDB_LIST_SESSIONS_CURRENT ) 失败' ) ;
      $find = False;
      while( $record = $cursor->next() )
      {
         $sessionID = $record['SessionID'] ;
         $find = True;
      }
      $this->assertEquals( $find, True, 'list(SDB_LIST_SESSIONS_CURRENT) 返回为空' ) ;
      $curerr = $this->testdb->forceSession( $sessionID ) ;
      $this -> assertEquals( -16, $curerr['errno'], 'forceSession错误' );
   }
   
   public function testForceSessionWithOption()
   {
      $cursor = $this->testdb -> execSQL( 'select * from $SNAPSHOT_SESSION where Status="Running" and Type="SyncClockWorker"' ) ;
      $isFind = false ;
      $nodename = '' ;

      while( $record = $cursor -> next() )
      {
         $nodename  = $record['NodeName'] ;
         $sessionID = $record['SessionID'] ;
         $isFind = true ;
         break ;
      }

      if( $isFind )
      {
         //有session
         $hostname = explode( ':', $nodename ) ;
         $svcname  = $hostname[1] ;
         $hostname = $hostname[0] ;
         $err = $this->testdb -> forceSession( $sessionID, array( 'HostName' => $hostname, 'svcname' => $svcname ) ) ;
         $this -> assertEquals( -63, $err['errno'], 'forceSession错误' ) ;
      }
   }
   
   protected function tearDown()
   {
      $err = $this->testdb->close();
   }
   
   public static function tearDownAfterClass()
   {
      $err = self::$db->close();
   }
};
?>

