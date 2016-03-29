<?php
class typeTest extends PHPUnit_Framework_TestCase
{
   public function test_type()
   {
      $oid   = new SequoiaID( '123456789012345678901234' ) ;
      $this -> assertEquals( '123456789012345678901234', $oid -> __toString(), 'Oid错误' ) ;
      
      $int64 = new SequoiaINT64( '123' ) ;
      $this -> assertEquals( '123', $int64 -> __toString(), 'Int64错误' ) ;
 
      $date  = new SequoiaDate( '1991-11-27' ) ;
      $this -> assertEquals( '1991-11-27', $date -> __toString(), 'Date错误' ) ;
      
      $date1 = new SequoiaDate( '1991-11-27T16:30:12.123456Z' ) ;
      $this -> assertEquals( '1991-11-28', $date1 -> __toString(), 'Date错误' ) ;
      
      $date2 = new SequoiaDate( '2051222400000' ) ;
      $this -> assertEquals( '2035-01-01', $date2 -> __toString(), 'Date错误' ) ;

      $time  = new SequoiaTimestamp( '1991-11-27-12.30.20.123456' ) ;
      $this -> assertEquals( '1991-11-27-12.30.20.123456', $time -> __toString(), 'Timestamp错误' ) ;
      
      $regex = new SequoiaRegex( 'a', 'i' ) ;
      $this -> assertEquals( '/a/i', $regex -> __toString(), 'Regex错误' ) ;
      
      $bin   = new SequoiaBinary( 'aGVsbG8=', '1' ) ;
      $this -> assertEquals( '(1)aGVsbG8=', $bin -> __toString(), 'Binary错误' ) ;
      
      $min   = new SequoiaMinKey() ;
      $this -> assertTrue( is_object( $min ) && is_a( $min, 'SequoiaMinKey' ), 'MinKey错误' ) ;
      
      $max   = new SequoiaMaxKey() ;
      $this -> assertTrue( is_object( $max ) && is_a( $max, 'SequoiaMaxKey' ), 'MaxKey错误' ) ;
   }
}
?>
