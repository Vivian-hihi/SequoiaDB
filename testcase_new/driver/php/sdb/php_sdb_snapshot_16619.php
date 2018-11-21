/****************************************************
@description:      check snapshot options
@testlink cases:   seqDB-16619
@modify list:
        2018-11-13 Luweikang init
****************************************************/
<?php
define('Cur_Path', dirname(__FILE__));
include_once Cur_Path.'/../global.php';

class Rename16619 extends PHPUnit_Framework_TestCase
{
   private static $csName = "cs16619";
   private static $clName = "cl16619";
   private static $csNum = 10;
   private static $db;
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'. 
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );                     
                           
      //create cs to query collectionspaces snapshot
      for($i = 0; $i < self::$csNum; $i++)
      {
         $cs = self::$db -> selectCS( self::$csName . "_" . $i);
         self::checkErrno( 0, self::$db -> getError()['errno'] );
         $cs -> selectCL( self::$clName );	
         self::checkErrno( 0, self::$db -> getError()['errno'] );
      }
   }
   
   function test()
   {
      $skipNum = 5;
      $returnNum = 1;
      $expCSName = self::$csName . "_" . $skipNum;
      echo "\n---Begin to check snapshot.\n";
      $csSnapCur = self::$db -> snapshot( SDB_SNAP_COLLECTIONSPACES, 
				array( "Name" => $expCSName ),
				array( "Collection" => array( "$include" => 1) ),
				array( "Name" => 1),
				null, $skipNum, $returnNum );
      if( empty( $csSnapCur ) ) 
      {
	 throw new Exception( $expCSName ." is not exist, check snapshot error");
      }
      //check snapshot cs name and returnNum
      $times = 0;
      while( $record = $csSnapCur -> next())
      {
         $clArr = $record["Collection"];
         $actCSName = explode( ".", $clArr[0]["Name"] )[0];
         if( strcmp( $actCSName, $expCSName ) !=0 )
	 {
            throw new Exception( "check cl full name error, exp: " . $expCSName .  "  act: " . $actCSName );
         }
         $times++;
      }
      if( $times != $returnNum)
      {
         throw new Exception( "check snapshot record num error, exp: ". $returnNum .", act: " . $times );
      }
   }
   
   public static function tearDownAfterClass()
   {
      for($i = 0; $i < 3; $i++)
      {
      	 $err = self::$db -> dropCS( self::$csName . "_" . $i );
      	 if ( $err['errno'] != 0 )
      	 {
             throw new Exception("failed to drop cs, errno=".$err['errno']);
      	 }
      }
      self::$db->close();
   }
   
   private static function checkErrno( $expErrno, $actErrno, $msg = "" )
   {
      if( $expErrno != $actErrno ) 
      {
         throw new Exception( "expect [".$expErrno."] but found [".$actErrno."]. ".$msg );
      }
   }
   
}
?>
