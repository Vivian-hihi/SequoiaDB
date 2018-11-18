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
   private static $cs;
   private static $db;
   
   public static function setUpBeforeClass()
   {
      self::$db = new Sequoiadb();
      self::$db -> connect(globalParameter::getHostName().':'. 
                           globalParameter::getCoordPort()) ;
      self::checkErrno( 0, self::$db -> getError()['errno'] );                     
                           
      for($i = 0; $i < 3; $i++)
      {
         self::$cs = self::$db -> selectCS( self::$csName . "_" . $i);
         self::checkErrno( 0, self::$db -> getError()['errno'] );
         self::$cs -> selectCL( self::$clName );	
         self::checkErrno( 0, self::$db -> getError()['errno'] );
      }
   }
   
   function test()
   {
      echo "\n---Begin to check snapshot.\n";
      $csSnapCur = self::$db -> snapshot( SDB_SNAP_COLLECTIONSPACES, 
				array( "Name" => self::$csName . "_1" ),
				array( "Collection" => array( "$include" => 1) ),
				array( "Name" => 1),
				null, 0, -1 );
      //$csSnapCur = self::$db -> snapshot( SDB_SNAP_COLLECTIONSPACES, array( "Name" => self::$csName . "_1") );
      if( empty( $csSnapCur ) ) 
      {
	 throw new Exception( $csName . "_1 is not exist, check snapshot error");
      }
      $times = 0;
      while( $record = $csSnapCur -> next())
      {
         var_dump( $record );
         $clArr = $record["Collection"];
         $actCSName = explode( ".", $clArr[0]["Name"] )[0];
         if( strcmp( $actCSName, self::$csName . "_1" ) !=0 )
	 {
            throw new Exception( "check cl full name error, exp: " . self::$csName . "_1  act: " . $actCSName );
         }
         $times++;
      }
      if( $times != 1)
      {
         throw new Exception( "check snapshot record num error, exp: 1, act: " . $times );
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
