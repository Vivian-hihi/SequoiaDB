/****************************************************
@description:      rename check
@modify list:
        2018-3-14 luweikang init
****************************************************/
<?php
class RenameUtils
{
   public function checkRenameCS( $db, $oldCSName, $newCSName, $clNum = 1)
   {
      $db -> getCS( $oldCSName );
      self::checkErrno( -34, $db -> getError()['errno'] );
      
      sleep(2);
      $csSnapCur = $db -> snapshot( SDB_SNAP_COLLECTIONSPACES, array( 'Name' => $newCSName) );
      if( empty( $csSnapCur ) )
      {
         throw new Exception( $newCSName . " is not exist, check snapshot error");
      }
      while( $record = $csSnapCur -> next())
      {
         $clArr = $record["Collection"];
         if( $clNum > 0 )
         {
            $actCSName = explode( ".", $clArr[0]["Name"] )[0];
            if( strcmp( $actCSName, $newCSName) !=0 )
	    {
               throw new Exception( "check cl full name error, exp: " . $newCSName . ", act: " . $actCSName );
            }
         }
      }
   }
   
   public function checkRenameCL( $db, $csName, $oldCLName, $newCLName )
   {
      $db -> getCS( $csName ) -> getCL( $oldCLName );
      self::checkErrno( -23, $db -> getError()['errno'] );
      
      $db -> getCS( $csName ) -> getCL( $newCLName );
      self::checkErrno( 0, $db -> getError()['errno'] );
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
