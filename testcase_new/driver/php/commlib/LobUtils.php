/****************************************************
@description:      lob operate, warp class
@testlink cases:   seqDB-7681-7689
@modify list:
        2016-4-27 wenjing wang init
        2018-3-14 huangxiaoni modify
****************************************************/
<?php
class LobUtils
{
   private $db;
   private $cl;
      
   public function __construct( $db, $cl )
   {
      $this -> db = $db;
      $this -> cl = $cl;
   }
      
   public function getOid()
   {
      $uniqid = uniqid();
      $md5 = md5( $uniqid );     
      $oid = substr($md5, 8);
      return $oid;
   }
   
   public function getBytes( $len )
   {
      $str = $this -> getRandomStr( $len );
      $bytes = array();
      for( $i = 0; $i < $len; $i++ )
      {
         $bytes[] = ord( $str[$i] );
      }
      return $bytes;
   }
   
   public function getRandomStr( $len )
   {
      $str = "";
      for( $i = 0; $i < $len; $i++ )
      {
         $str .= chr( mt_rand(33, 126) );
      }
      return $str;
   }
      
   public function checkLobExist( $oid )
   {
      $isExist = false;
      $cursor = $this -> cl -> listLob();
      if( 0 !== $this -> db -> getError()['errno'] )
      {
         throw new Exception( "failed to exec listLob." );
      }
          
      while( $record = $cursor -> next() )
      {
         if( $record['Oid'] == $oid )
         {
            $isExist = true;
            break;
         }
      }
      
      if( !isExist )
      {
         throw new Exception( "failed to check lob[".$oid."]." );
      }
   }
      
   public function checkLobContent( $oid, $readLen, $expStr )
   {
      $lobObj = $this -> cl -> openLob( $oid, SDB_LOB_READ );
      if( 0 !== $this -> db -> getError()['errno'] )
      {
         throw new Exception( "failed to open lob[".$oid."]." );
      }
            
      $readStr = $lobObj -> read( $readLen );
      if( 0 !== $this -> db -> getError()['errno'] )
      {
         throw new Exception( "failed to read lob[".$oid."]." );
      }
      //echo "----readStr-----\n";
      //var_dump($readStr);
      
      $err = $lobObj -> close();
      if( 0 !== $this -> db -> getError()['errno'] )
      {
         throw new Exception( "failed to close lob[".$oid."]." );
      }
      
      $actMd5 = md5( $readStr );
      $expMd5 = md5( $expStr );
      if( strcmp($actMd5, $expMd5 ) )
      {
         throw new Exception( "failed to check lob[".$oid."] \nactMd5 = ".$actMd5.", expMd5 = ".$expMd5 );
      }
   }
   
}
?>

