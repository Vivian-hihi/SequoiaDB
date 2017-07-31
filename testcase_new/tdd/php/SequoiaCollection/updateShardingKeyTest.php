<?php
class cl_record_Test extends PHPUnit_Framework_TestCase
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

   public function test_Connect()
   {
      $db = new Sequoiadb();
      $err = $db -> connect( $this->address ) ;
      $this -> assertEquals( 0, $err['errno'], '数据库连接错误( 参数: '.$this->address.' )' ) ;
      return $db ;
   }

   /**
    * @depends test_Connect
    */
   public function test_isStandlone( $db )
   {
      $cursor = $db -> list( SDB_LIST_SHARDS ) ;
      $err = $db -> getError() ;
      if( $err['errno'] == -159 )
      {
         return true ;
      }
      $this -> assertEquals( 0, $err['errno'], '创建cl错误' ) ;
      $this -> assertNotEmpty( $cursor, '创建cl错误' ) ;
      return false ;
   }

   /**
    * @depends test_Connect
    */
   public function test_select_cs( $db )
   {
      $cs = $db -> selectCS( 'test_foo' ) ;
      $err = $db -> getError() ;
      $this -> assertEquals( 0, $err['errno'], '创建cs错误' ) ;
      $this -> assertNotEmpty( $cs, '创建cs错误' ) ;
      return $cs ;
   }


   /**
    * @depends test_Connect
    * @depends test_select_cs
    * @depends test_isStandlone
    */
   public function test_split( $db, $cs, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $groupList = array() ;
         $cursor = $db -> list( SDB_LIST_GROUPS, '{ $and: [ { GroupName:{ $ne: "SYSCatalogGroup" } }, { GroupName: { $ne: "SYSCoord" } } ] }', array( 'GroupName' => 1 ) ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], '获取group列表错误' ) ;
         $this -> assertNotEmpty( $cursor, '获取group列表错误' ) ;
         while( $record = $cursor -> next() )
         {
            array_push( $groupList, $record['GroupName'] ) ;
         }

         if( count( $groupList ) < 2 )
         {
            return ;
         }

         $cl = $cs -> createCL( 'update_sharding_key',
                                 array( 'ShardingKey' => array( 'a' => 1 ),
                                        'ShardingType' => 'range',
                                        'Group' => $groupList[0] ) ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], '创建cl错误' ) ;
         $this -> assertNotEmpty( $cl, '创建cl错误' ) ;

         $err = $cl -> insert( '{ a : 1 }' ) ;
         $this -> assertEquals( 0, $err['errno'], '插入记录错误' ) ;

         $err = $cl -> split( $groupList[0], $groupList[1], 50 ) ;
         $this -> assertEquals( 0, $err['errno'], 'split错误' ) ;

         return $cl ;
      }
   }


   /**
    * @depends test_Connect
    * @depends test_select_cs
    * @depends test_split
    * @depends test_isStandlone
    */
   public function test_update( $db, $cs, $cl, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $groupList = array() ;
         $cursor = $db -> list( SDB_LIST_GROUPS, '{ $and: [ { GroupName:{ $ne: "SYSCatalogGroup" } }, { GroupName: { $ne: "SYSCoord" } } ] }', array( 'GroupName' => 1 ) ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], '获取group列表错误' ) ;
         $this -> assertNotEmpty( $cursor, '获取group列表错误' ) ;
         while( $record = $cursor -> next() )
         {
            array_push( $groupList, $record['GroupName'] ) ;
         }

         if( count( $groupList ) < 2 )
         {
            return ;
         }

         $err = $cl -> update( '{ $set: { a : 1 } }', null, null, 0x00008000 ) ;
         $this -> assertEquals( -178, $err['errno'], '更新记录有问题' ) ;

         $err = $cl -> update( '{ $set: { a : 1 } }', null, null, constant("SDB_FLG_UPDATE_KEEP_SHARDINGKEY") ) ;
         $this -> assertEquals( -178, $err['errno'], '更新记录有问题' ) ;
      }
   }


   /**
    * @depends test_Connect
    * @depends test_select_cs
    * @depends test_split
    * @depends test_isStandlone
    */
   public function test_upsert( $db, $cs, $cl, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $groupList = array() ;
         $cursor = $db -> list( SDB_LIST_GROUPS, '{ $and: [ { GroupName:{ $ne: "SYSCatalogGroup" } }, { GroupName: { $ne: "SYSCoord" } } ] }', array( 'GroupName' => 1 ) ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], '获取group列表错误' ) ;
         $this -> assertNotEmpty( $cursor, '获取group列表错误' ) ;
         while( $record = $cursor -> next() )
         {
            array_push( $groupList, $record['GroupName'] ) ;
         }

         if( count( $groupList ) < 2 )
         {
            return ;
         }

         $err = $cl -> upsert( '{ $set: { a : 1 } }', null, null, null, 0x00008000 ) ;
         $this -> assertEquals( -178, $err['errno'], 'upsert记录有问题' ) ;

         $err = $cl -> upsert( '{ $set: { a : 1 } }', null, null, null,
                                constant("SDB_FLG_UPDATE_KEEP_SHARDINGKEY") ) ;
         $this -> assertEquals( -178, $err['errno'], 'upsert记录有问题' ) ;
      }
   }


   /**
    * @depends test_Connect
    * @depends test_select_cs
    * @depends test_split
    * @depends test_isStandlone
    */
   public function test_findAndUpdate( $db, $cs, $cl, $isStandlone )
   {
      if( $isStandlone == false )
      {
         $groupList = array() ;
         $cursor = $db -> list( SDB_LIST_GROUPS, '{ $and: [ { GroupName:{ $ne: "SYSCatalogGroup" } }, { GroupName: { $ne: "SYSCoord" } } ] }', array( 'GroupName' => 1 ) ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( 0, $err['errno'], '获取group列表错误' ) ;
         $this -> assertNotEmpty( $cursor, '获取group列表错误' ) ;
         while( $record = $cursor -> next() )
         {
            array_push( $groupList, $record['GroupName'] ) ;
         }

         if( count( $groupList ) < 2 )
         {
            return ;
         }

         $cursor = $cl -> findAndUpdate( '{ $set: { a : 1 } }', null, null, null, null,
                                      0, -1, 0x00008000 ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( -178, $err['errno'], 'findAndUpdate记录有问题' ) ;

         $cursor = $cl -> findAndUpdate( '{ $set: { a : 1 } }', null, null, null, null,
                                      0, -1, constant("SDB_FLG_UPDATE_KEEP_SHARDINGKEY") ) ;
         $err = $db -> getError() ;
         $this -> assertEquals( -178, $err['errno'], 'findAndUpdate记录有问题' ) ;
      }
   }


   /**
    * @depends test_Connect
    * @depends test_select_cs
    */
   public function test_clear( $db, $cs )
   {
      $err = $cs -> drop() ;
      $this -> assertEquals( 0, $err['errno'], '删除cs错误' ) ;
   }

}
?>
