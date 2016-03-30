<?php
class SequoiaDB_Domain_Test extends PHPUnit_Framework_TestCase
{
   protected $hostname ;
   protected $port ;
   protected $address ;

   protected function setUp()
   {
      $this -> hostname = empty( $_POST['hostname'] ) ? '127.0.0.1' : $_POST['hostname'] ;
      $this -> port = empty( $_POST['port'] ) ? '11810' : $_POST['port'] ;
      $this -> address = $this -> hostname.':'.$this -> port ;
   }

   public function test_connect()
   {
      $db = new SequoiaDB();
      $err = $db -> connect( $this->address ) ;
      $this -> assertEquals( 0, $err['errno'], '数据库连接错误( 数组参数: '.$this->address.' )' ) ;
      return $db ;
   }
   
   /**
    * @depends test_connect
    */
   public function test_isStandlone( $db )
   {
      $cursor = $db -> list( SDB_LIST_SHARDS ) ;
      $err = $db -> getError() ;
      if( $err['errno'] == -159 )
      {
         return true ;
      }
      $this -> assertEquals( 0, $err['errno'], 'list错误' ) ;
      $this -> assertNotEmpty( $cursor, 'list错误' ) ;
      return false ;
   }
   
   /**
    * @depends test_connect
    * @depends test_isStandlone
    */
   public function test_createDomain( $db, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $err = $db -> createDomain( 'myDomain', array( 'Groups' => array( "db1", "db2" ), 'AutoSplit' => true ) ) ;
         $this -> assertEquals( 0, $err['errno'], 'createDomain错误' ) ;
      }
   }

   /**
    * @depends test_connect
    * @depends test_isStandlone
    */
   public function test_listDomain( $db, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $cursor = $db -> listDomain() ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], 'listDomain错误' ) ;
         $this -> assertNotEmpty( $cursor, 'listDomain错误' ) ;
         $num = 0 ;
         while( $record = $cursor -> next() ) {
            ++$num ;
         }
         $this -> assertEquals( 1, $num, 'listDomain错误' ) ;
      }
   }
   
   /**
    * @depends test_connect
    * @depends test_isStandlone
    */
   public function test_getDomain( $db, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $domainObj = $db -> getDomain( 'myDomain' ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], 'getDomain错误' ) ;
         $this -> assertNotEmpty( $domainObj, 'getDomain错误' ) ;
         
         //这个应该是不存在的
         $domainObj = $db -> getDomain( 'myDomain1' ) ;
         $err = $db -> getError() ;
         $this -> assertNotEquals( 0, $err['errno'], 'getDomain错误' ) ;
         $this -> assertEmpty( $domainObj, 'getDomain错误' ) ;
      }
   }
   
   /**
    * @depends test_connect
    * @depends test_isStandlone
    */
   public function test_dropDomain( $db, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $err = $db -> dropDomain( 'myDomain' ) ;
         $this -> assertEquals( 0, $err['errno'], 'dropDomain错误' ) ;
      }
   }
}

?>