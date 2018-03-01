<?php
class SequoiaDB_Test extends PHPUnit_Framework_TestCase
{
   protected $hostname ;
   protected $port ;
   protected $address ;
   protected $arrayAddress ;

   protected function setUp()
   {
      $this -> hostname = empty( $_POST['hostname'] ) ? '127.0.0.1' : $_POST['hostname'] ;
      $this -> port = empty( $_POST['port'] ) ? '11810' : $_POST['port'] ;
      $this -> address = $this -> hostname.':'.$this -> port ;
      $this -> arrayAddress = array( $this -> address ) ;
   }

   public function test_connect()
   {
      //hostname:port
      $db = new SequoiaDB();
      $err = $db -> connect( $this->address ) ;
      $this -> assertEquals( 0, $err['errno'], '数据库连接错误( 参数: '.$this->address.' )' ) ;
      $db -> close() ;
      unset( $db ) ;

      //array( hostname, hostname:post )
      $db = new SequoiaDB();
      $err = $db -> connect( $this->arrayAddress ) ;
      $this -> assertEquals( 0, $err['errno'], '数据库连接错误( 数组参数: '.$this->arrayAddress[0].' )' ) ;
      return $db ;
   }
   
   /**
    * @depends test_connect
    */
   public function test_reset_snapshot( $db )
   {
      $cursor = $db -> snapshot( SDB_SNAP_DATABASE ) ;
      $err = $db -> getError() ;
      $this -> assertEquals( 0, $err['errno'], 'reset前获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $this -> assertNotEmpty( $cursor, 'reset前获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $old = $cursor -> next() ;
      $this -> assertNotEmpty( $old, 'reset前获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;

      $err = $db -> resetSnapshot() ;
      
      $this -> assertEquals( 0, $err['errno'], 'resetSnapshot错误' ) ;
      $cursor = $db -> snapshot( SDB_SNAP_DATABASE ) ;
      $err = $db -> getError() ;
      $this -> assertEquals( 0, $err['errno'], 'reset后获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $this -> assertNotEmpty( $cursor, 'reset后获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $new = $cursor -> next() ;
      $this -> assertNotEmpty( $new, 'reset后获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      
      if( array_key_exists( 'shardNetIn', $old ) )
      {
         $this -> assertLessThan( $old['shardNetIn'], $new['shardNetIn'], 'resetSnapshot没有生效(shardNetIn)' ) ;
      }
      else
      {
         $this -> assertLessThan( $old['svcNetIn'], $new['svcNetIn'], 'resetSnapshot没有生效(svcNetIn)' ) ;
      }
   }
   
   /**
    * @depends test_connect
    */
   public function test_reset_snapshot_with_options( $db )
   {
      $cursor = $db -> snapshot( SDB_SNAP_DATABASE ) ;
      $err = $db -> getError() ;
      $this -> assertEquals( 0, $err['errno'], 'reset前获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $this -> assertNotEmpty( $cursor, 'reset前获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $old = $cursor -> next() ;
      $this -> assertNotEmpty( $old, 'reset前获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;

      $err = $db -> resetSnapshot( array( 'Type' => "all" ) ) ;
      
      $this -> assertEquals( 0, $err['errno'], 'resetSnapshot错误' ) ;
      $cursor = $db -> snapshot( SDB_SNAP_DATABASE ) ;
      $err = $db -> getError() ;
      $this -> assertEquals( 0, $err['errno'], 'reset后获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $this -> assertNotEmpty( $cursor, 'reset后获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      $new = $cursor -> next() ;
      $this -> assertNotEmpty( $new, 'reset后获取snapshot '.SDB_SNAP_DATABASE.' 错误' ) ;
      
      if( array_key_exists( 'shardNetIn', $old ) )
      {
         $this -> assertLessThan( $old['shardNetIn'], $new['shardNetIn'], 'resetSnapshot with options 没有生效(shardNetIn)' ) ;
      }
      else
      {
         $this -> assertLessThan( $old['svcNetIn'], $new['svcNetIn'], 'resetSnapshot with options没有生效(svcNetIn)' ) ;
      }
   }

}
?>