<?php
/*******************************************************************************
   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*******************************************************************************/

/** \file class_date.php
    \brief date class
 */

/**
 * Class for create an object of the date type
 */
class SequoiaDate
{
   /**
    * Constructor.
    *
    * @param $date the date
    *
    * @code string format
    *
    * @code date format.
    * $dateObj = new SequoiaDate( '2000-01-01' ) ;
    * $arr = array( 'date' => $dateObj ) ; // json ==> { "date": { "$date": "2000-01-01" } }
    * @endcode
    *
    * @code default parameter, current date. ( Available/Support on 2.10 or newer version. )
    * $dateObj = new SequoiaDate() ;
    * $arr = array( 'date' => $dateObj ) ; // json ==> { "date": { "$date": "2017-12-01" } }
    * @endcode
    *
    * @code string format(millisecond). ( Available/Support on 2.10 or newer version. )
    * $dateObj = new SequoiaDate( '946656000000' ) ;
    * $arr = array( 'date' => $dateObj ) ; // json ==> { "date": { "$date": "2000-01-01" } }
    * @endcode
    *
    * @code integer(millisecond). ( Available/Support on 2.10 or newer version. )
    * $dateObj = new SequoiaDate( 946656000000 ) ;
    * $arr = array( 'date' => $dateObj ) ; // json ==> { "date": { "$date": "2000-01-01" } }
    * @endcode
   */
   public function __construct( string|integer $date ){}

   /**
    * PHP Magic Methods, the class as string output.
    *
    * @return date string
    *
    * @code
    * $dateObj = new SequoiaDate( '2000-01-01' ) ;
    * echo $dateObj ; // output ==> 2000-01-01
    * @endcode
    */
   public function __toString(){}
}