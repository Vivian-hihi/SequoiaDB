/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = utilDictionary.hpp

   Descriptive Name = Uniform dictionary definitions.

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/23/2016  YSD Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef UTIL_DICTIONARY
#define UTIL_DICTIONARY

namespace engine
{
   /* Dictionary type. Currently only lzw is supported. */
   enum UTIL_DICT_TYPE
   {
      UTIL_DICT_LZW = 1
   } ;

   /*
    * The uniform header structure of dictionaries. Any dictionary should put
    * this structure at the head.
    */
   struct _utilDictHead
   {
      UTIL_DICT_TYPE _type ;
      UINT8 _version ;
      CHAR _reserve[20] ;
   } ;
   typedef _utilDictHead utilDictHead ;
}

#endif /* UTIL_DICTIONARY */

