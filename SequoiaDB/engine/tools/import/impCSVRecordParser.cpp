/*******************************************************************************

   Copyright (C) 2011-2015 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = impCSVRecordParser.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impCSVRecordParser.hpp"
#include "../client/base64c.h"
#include "ossUtil.h"
#include "pd.hpp"
#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>

namespace import
{
   /* csv type */
   #define CSV_STR_AUTO       "auto"
   #define CSV_STR_INT        "int"
   #define CSV_STR_INTEGER    "integer"
   #define CSV_STR_LONG       "long"
   #define CSV_STR_BOOL       "bool"
   #define CSV_STR_BOOLEAN    "boolean"
   #define CSV_STR_DOUBLE     "double"
   #define CSV_STR_STRING     "string"
   #define CSV_STR_TIMESTAMP  "timestamp"
   #define CSV_STR_DATE       "date"
   #define CSV_STR_NULL       "null"
   #define CSV_STR_OID        "oid"
   #define CSV_STR_REGEX      "regex"
   #define CSV_STR_BINARY     "binary"
   #define CSV_STR_NUMBER     "number"

   #define CSV_STR_TRUE       "true"
   #define CSV_STR_FALSE      "false"
   #define CSV_STR_TRUE_SIZE  ((INT32)(sizeof(CSV_STR_TRUE) - 1))
   #define CSV_STR_FALSE_SIZE ((INT32)(sizeof(CSV_STR_FALSE) - 1))

   #define CSV_STR_INT_SIZE         (sizeof(CSV_STR_INT) - 1)
   #define CSV_STR_INTEGER_SIZE     (sizeof(CSV_STR_INTEGER) - 1)
   #define CSV_STR_LONG_SIZE        (sizeof(CSV_STR_LONG) - 1)
   #define CSV_STR_BOOL_SIZE        (sizeof(CSV_STR_BOOL) - 1)
   #define CSV_STR_BOOLEAN_SIZE     (sizeof(CSV_STR_BOOLEAN) - 1)
   #define CSV_STR_DOUBLE_SIZE      (sizeof(CSV_STR_DOUBLE) - 1)
   #define CSV_STR_STRING_SIZE      (sizeof(CSV_STR_STRING) - 1)
   #define CSV_STR_TIMESTAMP_SIZE   (sizeof(CSV_STR_TIMESTAMP) - 1)
   #define CSV_STR_DATE_SIZE        (sizeof(CSV_STR_DATE) - 1)
   #define CSV_STR_NULL_SIZE        (sizeof(CSV_STR_NULL) - 1)
   #define CSV_STR_OID_SIZE         (sizeof(CSV_STR_OID) - 1)
   #define CSV_STR_REGEX_SIZE       (sizeof(CSV_STR_REGEX) - 1)
   #define CSV_STR_BINARY_SIZE      (sizeof(CSV_STR_BINARY) - 1)
   #define CSV_STR_NUMBER_SIZE      (sizeof(CSV_STR_NUMBER) - 1)
   #define CSV_STR_TYPE_MIN_SIZE    3

   #define CSV_STR_TYPE_EQ(type, str, len) \
      ((sizeof(type) - 1) == len && ossStrncasecmp(str, type, len) == 0)

   /* key word */
   #define CSV_STR_DEFAULT       "default"
   #define CSV_STR_DEFAULT_SIZE  (sizeof(CSV_STR_DEFAULT) - 1)
   #define CSV_STR_FIELD         "field"

   #define CSV_STR_BACKSLASH     '/'
   #define CSV_STR_EMPTYOPTIONS  ""

   #define CSV_STR_LEFTBRACKET   '('
   #define CSV_STR_RIGHTBRACKET  ')'

   #define CSV_INT_MAX  (2147483647)
   #define CSV_INT_MIN  (-2147483648)
   #define CSV_LONG_MAX OSS_SINT64_MAX
   #define CSV_LONG_MIN OSS_SINT64_MIN

   #define TIME_FORMAT "%d-%d-%d-%d.%d.%d.%d"
   #define DATE_FORMAT "%d-%d-%d"
   #define INT32_LAST_YEAR 2038
   #define RELATIVE_YEAR 1900
   #define RELATIVE_MOD 12
   #define RELATIVE_DAY 31
   #define RELATIVE_HOUR 24
   #define RELATIVE_MIN_SEC 60

   #define TIME_MAX_NUM  2147356800
   #define TIME_MIX_NUM -2209017600

   #define RECORD_ID_NAME "_id"

   static inline void _skipSpace(CHAR** data, INT32& length)
   {
      CHAR* str = *data;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(NULL != str, "str can't be NULL");

      while (length > 0)
      {
         if (!isspace(*str))
         {
            *data = str;
            break;
         }

         str++;
         length--;
      }
   }

   static inline BOOLEAN _startWith(const CHAR* data, INT32 dataLen,
                                    const CHAR* str, INT32 strLen)
   {
      SDB_ASSERT(dataLen > 0, "dataLen must be greater than 0");
      SDB_ASSERT(strLen > 0, "strLen must be greater than 0");

      if (data[0] == str[0])
      {
         // accelerate for single character
         if (1 == strLen)
         {
            return TRUE;
         }
         else if (dataLen >= strLen && 0 == ossStrncmp(data, str, strLen))
         {
            return TRUE;
         }
      }

      return FALSE;
   }

   static inline BOOLEAN _isValidFieldName(CHAR* field, INT32 length)
   {
      SDB_ASSERT(NULL != field, "field can't be NULL");

      if (length <= 0)
      {
         return FALSE;
      }

      // the first character can be underscore and alphabet
      if ('_' != field[0] && !isalpha(field[0]))
      {
         return FALSE;
      }

      // the rest characters can be underscore, digit and alphabet
      field++;
      length--;

      while (length > 0)
      {
         if ('_' != *field && !isalpha(*field) && !isdigit(*field))
         {
            return FALSE;
         }

         field++;
         length--;
      }

      return TRUE;
   }

   static inline CHAR* _CSVTypeToString(CSV_TYPE type)
   {
      switch(type)
      {
      case CSV_TYPE_AUTO:
         return CSV_STR_AUTO;
      case CSV_TYPE_INT:
         return CSV_STR_INT;
      case CSV_TYPE_LONG:
         return CSV_STR_LONG;
      case CSV_TYPE_NUMBER:
         return CSV_STR_NUMBER;
      case CSV_TYPE_DOUBLE:
         return CSV_STR_DOUBLE;
      case CSV_TYPE_BOOL:
         return CSV_STR_BOOL;
      case CSV_TYPE_STRING:
         return CSV_STR_STRING;
      case CSV_TYPE_TIMESTAMP:
         return CSV_STR_TIMESTAMP;
      case CSV_TYPE_DATE:
         return CSV_STR_DATE;
      case CSV_TYPE_NULL:
         return CSV_STR_NULL;
      case CSV_TYPE_OID:
         return CSV_STR_OID;
      case CSV_TYPE_REGEX:
         return CSV_STR_REGEX;
      case CSV_TYPE_BINARY:
         return CSV_STR_BINARY;
      default:
         return "unknown type";
      }
   }

   static inline INT32 _convertToCSVType(const CHAR* data, INT32 length, CSV_TYPE& type)
   {
      CHAR* str = (CHAR*)data;
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      if (length < CSV_STR_TYPE_MIN_SIZE)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid csv type");
         goto error;
      }

      type = CSV_TYPE_AUTO;

      switch(str[0])
      {
      case 'b':
         // bool
         // boolean
         // binary
         if (CSV_STR_TYPE_EQ(CSV_STR_BOOL, str, length) ||
             CSV_STR_TYPE_EQ(CSV_STR_BOOLEAN, str, length))
         {
            type = CSV_TYPE_BOOL;
         }
         else if (CSV_STR_TYPE_EQ(CSV_STR_BINARY, str, length))
         {
            type = CSV_TYPE_BINARY;
         }
         break;
      case 'd':
         // date
         // double
         if (CSV_STR_TYPE_EQ(CSV_STR_DATE, str, length))
         {
            type = CSV_TYPE_DATE;
         }
         else if (CSV_STR_TYPE_EQ(CSV_STR_DOUBLE, str, length))
         {
            type = CSV_TYPE_DOUBLE;
         }
         break;
      case 'i':
         // int
         // integer
         if (CSV_STR_TYPE_EQ(CSV_STR_INT, str, length) ||
             CSV_STR_TYPE_EQ(CSV_STR_INTEGER, str, length))
         {
            type = CSV_TYPE_INT;
         }
         break;
      case 'l':
         // long
         if (CSV_STR_TYPE_EQ(CSV_STR_LONG, str, length))
         {
            type = CSV_TYPE_LONG;
         }
         break;
      case 'n':
         // null
         // number
         if (CSV_STR_TYPE_EQ(CSV_STR_NULL, str, length))
         {
            type = CSV_TYPE_NULL;
         }
         else if (CSV_STR_TYPE_EQ(CSV_STR_NUMBER, str, length))
         {
            type = CSV_TYPE_NUMBER;
         }
         break;
      case 'o':
         // oid
         if (CSV_STR_TYPE_EQ(CSV_STR_OID, str, length))
         {
            type = CSV_TYPE_OID;
         }
         break;
      case 'r':
         // regex
         if (CSV_STR_TYPE_EQ(CSV_STR_REGEX, str, length))
         {
            type = CSV_TYPE_REGEX;
         }
         break;
      case 's':
         // string
         if (CSV_STR_TYPE_EQ(CSV_STR_STRING, str, length))
         {
            type = CSV_TYPE_STRING;
         }
         break;
      case 't':
         // timestamp
         if (CSV_STR_TYPE_EQ(CSV_STR_TIMESTAMP, str, length))
         {
            type = CSV_TYPE_TIMESTAMP;
         }
         break;
      default:
         rc = SDB_INVALIDARG;
         goto error;
      }

      if (CSV_TYPE_AUTO == type)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid csv type");
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   // [+|-]<0~9...>
   static inline INT32 _stringToInt(const CHAR* data, INT32 length,
                                    INT32& value, INT32& valueLength)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      BOOLEAN neg = FALSE;
      UINT32 quo; // quoteint
      INT32 rem; // remainder
      INT32 num;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      if ('-' == *str)
      {
         neg = TRUE;
         str++;
         len--;
      }
      else if ('+' == *str)
      {
         str++;
         len--;
      }

      if (len == 0 || !isdigit(*str))
      {
         rc = SDB_INVALIDARG;
         //PD_LOG(PDERROR, "no digit for integer");
         goto error;
      }

      quo = neg ? -CSV_INT_MIN : CSV_INT_MAX;
      rem = quo % 10;
      quo /= 10;
      num = 0;

      while (len > 0)
      {
         INT32 ch = *str;

         if (!isdigit(ch))
         {
            break;
         }

         ch -= '0';

         // overflow
         if (num > (INT32)quo || (num == (INT32)quo && ch > rem))
         {
            rc = SDB_INVALIDARG;
            //PD_LOG(PDERROR, "integer overflow");
            goto error;
         }
         
         num = num * 10 + ch;
         str++;
         len--;
      }

      value = neg ? -num : num;
      valueLength = length - len;

   done:
      return rc;
   error:
      goto done;
   }

   // [+|-]<0~9...>
   static inline INT32 _stringToLong(const CHAR* data, INT32 length,
                                     INT64& value, INT32& valueLength)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      BOOLEAN neg = FALSE;
      UINT64 quo; // quoteint
      INT64 rem; // remainder
      INT64 num;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      if ('-' == *str)
      {
         neg = TRUE;
         str++;
         len--;
      }
      else if ('+' == *str)
      {
         str++;
         len--;
      }

      if (len == 0 || !isdigit(*str))
      {
         rc = SDB_INVALIDARG;
         //PD_LOG(PDERROR, "no digit for long");
         goto error;
      }

      quo = neg ? ((UINT64)CSV_LONG_MAX + 1) : CSV_LONG_MAX;
      rem = quo % 10;
      quo /= 10;
      num = 0;

      while (len > 0)
      {
         INT32 ch = *str;

         if (!isdigit(ch))
         {
            break;
         }

         ch -= '0';

         // overflow
         if (num > (INT64)quo || (num == (INT64)quo && ch > rem))
         {
            rc = SDB_INVALIDARG;
            //PD_LOG(PDERROR, "long overflow");
            goto error;
         }
         
         num = num * 10 + ch;
         str++;
         len--;
      }

      value = neg ? -num : num;
      valueLength = length - len;

   done:
      return rc;
   error:
      goto done;
   }

   static inline INT32 _stringToNumber(const CHAR* data, INT32 length,
                                       CSV_TYPE& type, CSVFieldValue& value,
                                       INT32& valueLength)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      INT64 integer;
      INT64 decimal;
      INT64 exponent;
      INT32 intLen;
      INT32 decLen;
      INT32 expLen;
      FLOAT64 num;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      // integer part
      rc = _stringToLong(str, len, integer, intLen);
      if (SDB_OK != rc)
      {
         //PD_LOG(PDERROR, "failed to convert to long, rc=%d", rc);
         goto error;
      }

      str += intLen;
      len -= intLen;

      if ('.' != *str)
      {
         if (integer >= CSV_INT_MIN && integer <= CSV_INT_MAX)
         {
            type = CSV_TYPE_INT;
            value.intVal = (INT32)integer;
         }
         else
         {
            type = CSV_TYPE_LONG;
            value.longVal = integer;
         }
         valueLength = length - len;
         goto done;
      }

      str++;
      len--;
      type = CSV_TYPE_DOUBLE;

      if (!isdigit(*str))
      {
         value.doubleVal = integer;
         valueLength = length - len;
         goto done;
      }

      // fractional part
      rc = _stringToLong(str, len, decimal, decLen);
      if (SDB_OK != rc)
      {
         //PD_LOG(PDERROR, "failed to convert to long, rc=%d", rc);
         goto error;
      }

      SDB_ASSERT(decimal >= 0, "decimal must be greater or equals 0");

      str += decLen;
      len -= decLen;
      valueLength += decLen;
      num = (FLOAT64)integer + (FLOAT64)decimal / pow(10.0, decLen);

      if ('E' != *str && 'e' != *str)
      {
         value.doubleVal = num;
         valueLength = length - len;
         goto done;
      }

      str++;
      len--;

      if (!isdigit(*str) && '+' != *str && '-' != *str)
      {
         value.doubleVal = num;
         valueLength = length - len;
         goto done;
      }

      // exponent part
      rc = _stringToLong(str, len, exponent, expLen);
      if (SDB_OK != rc)
      {
         //PD_LOG(PDERROR, "failed to convert to long, rc=%d", rc);
         goto error;
      }

      str += expLen;
      len -= expLen;
      valueLength += expLen;
      num *= pow(10.0, (FLOAT64)exponent);

      value.doubleVal = num;
      valueLength = length - len;

   done:
      return rc;
   error:
      goto done;
   }

   // <true|false>
   static inline INT32 _stringToBool(const CHAR* data, INT32 length,
                                     BOOLEAN& value, INT32& valueLength)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      if (length >= CSV_STR_TRUE_SIZE &&
          ossStrncasecmp(data, CSV_STR_TRUE, CSV_STR_TRUE_SIZE) == 0)
      {
         value = TRUE;
         valueLength = CSV_STR_TRUE_SIZE;
         goto done;
      }
      else if (length >= CSV_STR_FALSE_SIZE &&
               ossStrncasecmp(data, CSV_STR_FALSE, CSV_STR_FALSE_SIZE) == 0)
      {
         value = FALSE;
         valueLength = CSV_STR_FALSE_SIZE;
         goto done;
      }
      else
      {
         rc = SDB_INVALIDARG;
         //PD_LOG(PDERROR, "failed to convert to bool, rc=%d", rc);
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   // [+|-]<0~9...>[.<0~9...>[<E|e>[+|-]<0~9...>]]
   // -123.45e-678
   static inline INT32 _stringToDouble(const CHAR* data, INT32 length,
                                       FLOAT64& value, INT32& valueLength)
   {
      INT32 rc = SDB_OK;
      CSV_TYPE subType = CSV_TYPE_AUTO;
      CSVFieldValue subValue;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      rc = _stringToNumber(data, length, subType, subValue, valueLength);
      if (SDB_OK != rc)
      {
         //PD_LOG(PDERROR, "failed to convert to number, rc=%d", rc);
         goto error;
      }

      switch(subType)
      {
      case CSV_TYPE_DOUBLE:
         value = subValue.doubleVal;
         break;
      case CSV_TYPE_INT:
         value = subValue.intVal;
         break;
      case CSV_TYPE_LONG:
         value = subValue.longVal;
         break;
      default:
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid subtype: %d", subType);
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   static INT32 _stringToNull(const CHAR* data, INT32 length,
                              const CHAR* fieldDel, INT32 fieldDelLen,
                              INT32& valueLength, BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      _skipSpace(&str, len);
      if (len == 0)
      {
         valueLength = length;
         goto done;
      }

      if (0 == ossStrncasecmp(str, CSV_STR_NULL, CSV_STR_NULL_SIZE))
      {
         str += CSV_STR_NULL_SIZE;
         len -= CSV_STR_NULL_SIZE;
      }

      _skipSpace(&str, len);
      if (len == 0)
      {
         valueLength = length;
         goto done;
      }

      if (_startWith(str, len, fieldDel, fieldDelLen))
      {
         fieldEnd = TRUE;
         valueLength = length - len;
         goto done;
      }
      else
      {
         rc = SDB_INVALIDARG;
         //PD_LOG(PDERROR, "invalid null");
         goto error;
      }
      

   done:
      return rc;
   error:
      goto done;
   }

   static inline INT32 _stringToString(const CHAR* data, INT32 length,
                                       const CHAR* strDel, INT32 strDelLen,
                                       const CHAR* fieldDel, INT32 fieldDelLen,
                                       CSVString& value, INT32& valueLength,
                                       BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      BOOLEAN inString = FALSE;
      fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");
      SDB_ASSERT(!isspace(*data), "data can't begin with space");

      if (_startWith(str, len, strDel, strDelLen))
      {
         // skip the string delimiter
         str += strDelLen;
         len -= strDelLen;
         inString = TRUE;
      }

      // point to string head
      value.str = str;

      while (len > 0)
      {
         if (_startWith(str, len, strDel, strDelLen))
         {
            // previous character is escape
            // TODO: process "\\\\"
            if ('\\' == *(str - 1))
            {
               len -= strDelLen;
               str += strDelLen;
               continue;
            }

            len -= strDelLen;
            str += strDelLen;

            if (len == 0)
            {
               //*(str - strDelLen) = '\0';
               valueLength = length;
               value.length = str - strDelLen - value.str;
               goto done;
            }

            // two consecutive string delimiter
            if (_startWith(str, len, strDel, strDelLen))
            {
               len -= strDelLen;
               str += strDelLen;
               continue;
            }

            inString = FALSE;
            // terminate the string
            //*(str - strDelLen) = '\0';
            value.length = str - strDelLen - value.str;
            break;
         }

         if (!inString)
         {
            if (_startWith(str, len, fieldDel, fieldDelLen))
            {
               //*str = '\0';
               fieldEnd = TRUE;
               valueLength = length - len;
               value.length = str - value.str;
               goto done;
            }

            if (isspace(*str))
            {
               //*str = '\0';
               value.length = str - value.str;
               str++;
               len--;
               if (len == 0)
               {
                  valueLength = length;
                  goto done;
               }
               break;
            }
         }

         str++;
         len--;
      }

      if (inString || len == 0)
      {
         SDB_ASSERT(len == 0, "len must be equal 0");
         // must be sure it's safe to terminate the string
         //*str = '\0';
         valueLength = length;
         value.length = str - value.str;
         goto done;
      }
      else
      {
         _skipSpace(&str, len);
         if (len == 0)
         {
            valueLength = length;
            goto done;
         }
         else if (_startWith(str, len, fieldDel, fieldDelLen))
         {
            fieldEnd = TRUE;
            valueLength = length - len;
            goto done;
         }

         rc = SDB_INVALIDARG;
         //PD_LOG(PDERROR, "invalid string");
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   // support INT/LONG/DOUBLE/BOOL/NULL/STRING
   static inline INT32 _detectFieldType(const CHAR* data, INT32 length,
                                        const CHAR* strDel, INT32 strDelLen,
                                        const CHAR* fieldDel, INT32 fieldDelLen,
                                        CSV_TYPE& fieldType, CSVFieldValue& fieldValue,
                                        INT32& fieldLength, BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      INT32 valueLength = 0;
      fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");
      SDB_ASSERT(!isspace(*data), "data can't begin with space");

      rc = _stringToNumber(str, len, fieldType, fieldValue, valueLength);
      if (SDB_OK != rc)
      {
         fieldType = CSV_TYPE_BOOL;
         rc = _stringToBool(str, len, fieldValue.boolVal, valueLength);
      }

      if (SDB_OK == rc)
      {
         str += valueLength;
         len -= valueLength;

         _skipSpace(&str, len);
         if (len == 0)
         {
            fieldLength = length;
            goto done;
         }
         else if (_startWith(str, len, fieldDel, fieldDelLen))
         {
            fieldEnd = TRUE;
            fieldLength = length - len;
            goto done;
         }

         // reset
         str = (CHAR*)data;
         len = length;
      }

      // null
      rc = _stringToNull(str, len, fieldDel, fieldDelLen, fieldLength, fieldEnd);
      if (SDB_OK == rc)
      {
         fieldType = CSV_TYPE_NULL;
         goto done;
      }

      // string
      fieldType = CSV_TYPE_STRING;
      rc = _stringToString(str, len,
                           strDel, strDelLen,
                           fieldDel, fieldDelLen,
                           fieldValue.strVal, fieldLength,
                           fieldEnd);
      if (SDB_OK != rc)
      {
         goto error;
      }

   done:
      return rc;
   error:
      goto error;
   }

   static inline INT32 _stringToTimestamp(CSVString& data, CSVTimestamp& value)
   {
      CHAR* str = data.str;
      INT32 rc = SDB_OK;
      BOOLEAN hasNonDigit = FALSE;
      CHAR ch;

      SDB_ASSERT(NULL != data.str, "data.str can't be NULL");

      // terminate string
      CHAR* term = str + data.length;
      CHAR tmpch = *term;
      *term = '\0';

      while ((ch = *(str++)) != '\0')
      {
         if (!isdigit(ch))
         {
            hasNonDigit = TRUE;
            break;
         }
      }

      if (hasNonDigit)
      {
         struct tm t ;
         /* date and timestamp */
         INT32 year = 0;
         INT32 month = 0;
         INT32 day = 0;
         INT32 hour = 0;
         INT32 minute = 0;
         INT32 second = 0;
         INT32 micros = 0;
         time_t timep;

         /* for timestamp type, we provide yyyy-mm-dd-hh.mm.ss.uuuuuu */
         if (!sscanf(data.str,
                     TIME_FORMAT,
                     &year,
                     &month,
                     &day,
                     &hour,
                     &minute,
                     &second,
                     &micros))
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "failed to scan timepstamp, rc=%d", rc);
            goto error;
         }
         month--;

         /* sanity check */
         if (year > INT32_LAST_YEAR || year < RELATIVE_YEAR ||
             month >= RELATIVE_MOD || month < 0 ||
             day > RELATIVE_DAY || day <= 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid date of timestamp");
            goto error;
         }

         if (INT32_LAST_YEAR == year)
         {
            if (month > 0 || (month == 0 && day >= 18))
            {
               rc = SDB_INVALIDARG;
               PD_LOG(PDERROR, "invalid month or day of timestamp");
               goto error;
            }
         }

         if (hour >= RELATIVE_HOUR || hour < 0 ||
             minute >= RELATIVE_MIN_SEC || minute < 0 ||
             second >= RELATIVE_MIN_SEC || second < 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid time of timestamp");
            goto error;
         }

         year -= RELATIVE_YEAR;

         /* construct tm */
         t.tm_year = year;
         t.tm_mon  = month;
         t.tm_mday = day;
         t.tm_hour = hour;
         t.tm_min  = minute;
         t.tm_sec  = second;

         /* create integer time representation */
         timep = mktime(&t);
         if(!timep)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "failed to make time of timestamp");
            goto error;
         }
         value.sec = (INT32)timep;
         value.us = micros;
      }
      else
      {
         INT64 varLong = 0;
         INT64 sec = 0;
         INT64 us = 0;
         INT32 valueLength = 0;

         rc = _stringToLong(data.str, data.length, varLong, valueLength);
         if (SDB_OK != rc || data.length != valueLength)
         {
            PD_LOG(PDERROR, "failed to get the number of timestamp, rc=%d", rc);
            goto error;
         }

         sec = varLong / 1000;
         us = varLong - ( sec * 1000 );

         if (varLong < TIME_MIX_NUM )
         {
            PD_LOG(PDERROR, "The timestamp %lld is greater than %d000",
                   varLong, TIME_MIX_NUM);
            rc = SDB_INVALIDARG;
            goto error;
         }

         if ((sec > TIME_MAX_NUM) || ((sec == TIME_MAX_NUM) && us > 0))
         {
            PD_LOG(PDERROR, "The timestamp %lld is greater than %d000",
                   varLong, TIME_MAX_NUM);
            rc = SDB_INVALIDARG;
            goto error;
         }

         value.sec = (INT32)sec;
         value.us = (INT32)us;
      }

   done:
      // recovery string
      *term = tmpch;
      return rc;
   error:
      goto done;
   }

   static inline INT32 _stringToDate(CSVString& data, INT64& value)
   {
      CHAR* str = data.str;
      INT32 rc = SDB_OK;
      BOOLEAN hasNonDigit = FALSE;
      CHAR ch;

      SDB_ASSERT(NULL != data.str, "data.str can't be NULL");

      // terminate string
      CHAR* term = str + data.length;
      CHAR tmpch = *term;
      *term = '\0';

      while ((ch = *(str++)) != '\0')
      {
         if (!isdigit(ch))
         {
            hasNonDigit = TRUE;
            break;
         }
      }

      if (hasNonDigit)
      {
         struct tm t;
         /* date and timestamp */
         INT32 year = 0;
         INT32 month = 0;
         INT32 day = 0;
         time_t timep;

         /* for timestamp type, we provide yyyy-mm-dd-hh.mm.ss.uuuuuu */
         if (!sscanf(data.str,
                     DATE_FORMAT,
                     &year,
                     &month,
                     &day))
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "failed to scan date");
            goto error;
         }
         month--;

         /* sanity check */
         if (year > INT32_LAST_YEAR || year < RELATIVE_YEAR ||
             month >= RELATIVE_MOD || month < 0 ||
             day > RELATIVE_DAY || day <= 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid date");
            goto error;
         }

         if (INT32_LAST_YEAR == year)
         {
            if (month > 0 || (month == 0 && day >= 18))
            {
               rc = SDB_INVALIDARG;
               PD_LOG(PDERROR, "invalid month or day of date");
               goto error;
            }
         }

         year -= RELATIVE_YEAR;

         /* construct tm */
         t.tm_year = year;
         t.tm_mon  = month;
         t.tm_mday = day;
         t.tm_hour = 0;
         t.tm_min  = 0;
         t.tm_sec  = 0;

         /* create integer time representation */
         timep = mktime(&t);
         if (!timep)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "failed to make time of date");
            goto error;
         }

         value = (INT64)timep * 1000;
      }
      else
      {
         INT32 valueLength = 0;

         rc = _stringToLong(data.str, data.length, value, valueLength);
         if (SDB_OK != rc || data.length != valueLength)
         {
            goto error;
         }

         if (value < TIME_MIX_NUM)
         {
            PD_LOG(PDERROR, "The time stamp %lld is greater than %d",
                   value, TIME_MIX_NUM);
            rc = SDB_INVALIDARG;
            goto error;
         }

         if (value > TIME_MAX_NUM)
         {
            PD_LOG(PDERROR, "The time stamp %lld is greater than %d",
                   value, TIME_MAX_NUM);
            rc = SDB_INVALIDARG;
            goto error;
         }

         value *= 1000;
      }

   done:
      // recovery string
      *term = tmpch;
      return rc;
   error:
      goto done;
   }

   static inline INT32 _stringToOID(CSVString& data, CSVString& value)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != data.str, "data.str can't be NULL");

      if (data.length != 24)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid oid length");
      }
      else
      {
         value.str = data.str;
         value.length = data.length;
      }

      return rc;
   }

   // "/pattern/<options>"
   static inline INT32 _stringToRegex(CSVString& data, CSVRegex& value)
   {
      CHAR* str = data.str;
      INT32 len = data.length;
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != data.str, "data.str can't be NULL");
      SDB_ASSERT(data.length > 0, "data.length must be greater than 0");

      if (len <= 0)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid regex length");
         goto error;
      }

      if (CSV_STR_BACKSLASH != *str)
      {
         value.pattern = str;
         value.patternLen = len;
         value.option = CSV_STR_EMPTYOPTIONS;
         goto done;
      }

      // skip '/'
      str++;
      len--;
      value.pattern = str;

      while (len > 0)
      {
         if (CSV_STR_BACKSLASH == *str)
         {
            break;
         }

         str++;
         len--;
      }

      if (len == 0 || str - data.str <= 1) // '/...' or '//...'
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid regex format");
         goto error;
      }

      //*str = '\0'; // terminate the pattern
      value.patternLen = str - value.pattern;

      // skip '/'
      str++;
      len--;

      if (len == 0)
      {
         value.option = CSV_STR_EMPTYOPTIONS;
         goto done;
      }

      if (isspace(*str))
      {
         value.option = CSV_STR_EMPTYOPTIONS;
      }
      else
      {
         value.option = str;
      }

      while (len > 0)
      {
         if (isspace(*str))
         {
            break;
         }

         str++;
         len--;
      }

      if (len != 0)
      {
         //*str = '\0';
         value.optionLen = str - value.option;
         str++;
         len--;
         _skipSpace(&str, len);

         if (len != 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid regex format");
            goto error;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // "(type)value"
   static inline INT32 _stringToBinary(CSVString& data, CSVBinary& value)
   {
      CHAR* str = data.str;
      INT32 len = data.length;
      INT32 rc = SDB_OK;
      INT32 base64Len = 0;

      SDB_ASSERT(NULL != data.str, "data.str can't be NULL");
      SDB_ASSERT(data.length > 0, "data.length must be greater than 0");

      value.bin = NULL;

      if (len <= 0)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid binary length");
         goto error;
      }

      if (CSV_STR_LEFTBRACKET == *str)
      {
         INT32 type = 0;
         INT32 typeLength = 0;

         // skip '('
         str++;
         len--;

         _skipSpace(&str, len);
         if (len == 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid binary format");
            goto error;
         }

         rc = _stringToInt(str, len, type, typeLength);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "invalid binary type");
            goto error;
         }

         if (type < 0 || type > 255)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "binary type is out of range[0~255], type:%d", type);
            goto error;
         }

         value.type = type;

         // skip the type number
         str += typeLength;
         len -= typeLength;

         _skipSpace(&str, len);
         if (len == 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid binary format");
            goto error;
         }

         if (CSV_STR_RIGHTBRACKET != *str)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid binary format");
            goto error;
         }

         // skip ')'
         str++;
         len--;

         if (len == 0)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "invalid binary format");
            goto error;
         }
      }
      else
      {
         value.type = 0;
      }

      value.str = str;

      base64Len = getDeBase64Size(str);
      if (base64Len <= 1)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid binary base64 size");
         goto error;
      }

      value.bin = (CHAR*)SDB_OSS_MALLOC(base64Len);
      if (NULL == value.bin)
      {
         rc = SDB_OOM;
         PD_LOG(PDERROR, "failed to malloc");
         goto error;
      }

      if (0 == base64Decode(str, value.bin, base64Len))
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "failed to decode binary");
         goto error;
      }

      value.binLen = base64Len;

   done:
      return rc ;
   error:
      SAFE_OSS_FREE(value.bin);
      goto done ;
   }

   static void _printField(CSVField& field)
   {
      CSV_TYPE type = field.type;

      std::cout << "id:\t" << field.id << std::endl;
      std::cout << "name:\t" << field.name << std::endl;
      std::cout << "type:\t" << _CSVTypeToString(type) << std::endl;
      if (CSV_TYPE_NUMBER == type)
      {
         std::cout << "type:\t" << _CSVTypeToString(field.subType) << std::endl;
      }
      std::cout << "hasDefault:\t" << field.hasDefault << std::endl;
      if (field.hasDefault)
      {
         std::cout << "default:\t";

         if (CSV_TYPE_NUMBER == type)
         {
            type = field.subType;
         }

         switch(type)
         {
         case CSV_TYPE_AUTO:
            std::cout << "auto" << std::endl;
            break;
         case CSV_TYPE_INT:
            std::cout << field.defaultValue.intVal << std::endl;
            break;
         case CSV_TYPE_LONG:
            std::cout << field.defaultValue.longVal << std::endl;
            break;
         case CSV_TYPE_DOUBLE:
            std::cout << field.defaultValue.doubleVal << std::endl;
            break;
         case CSV_TYPE_NUMBER:
            break;
         case CSV_TYPE_BOOL:
            std::cout << field.defaultValue.boolVal << std::endl;
            break;
         case CSV_TYPE_STRING:
            std::cout << "[" << field.defaultValue.strVal.str
                      << "](length: " << field.defaultValue.strVal.length << ")"
                      << std::endl;
            break;
         case CSV_TYPE_TIMESTAMP:
            std::cout << field.defaultValue.timestampVal.sec
                      << "."
                      << field.defaultValue.timestampVal.us
                      << std::endl;
            break;
         case CSV_TYPE_DATE:
            std::cout << field.defaultValue.dateVal << std::endl;
            break;
         case CSV_TYPE_OID:
            std::cout << "[" << field.defaultValue.oidVal.str
                      << "](length: " << field.defaultValue.oidVal.length << ")"
                      << std::endl;
            break;
         case CSV_TYPE_REGEX:
            std::cout << "pattern: [" << field.defaultValue.regexVal.pattern
                      << "], option: [";
            if (NULL != field.defaultValue.regexVal.option)
            {
               std::cout << field.defaultValue.regexVal.option;
            }
            else
            {
               std::cout << "NULL";
            }
            std::cout << "]"
                      << std::endl;
            break;
         case CSV_TYPE_BINARY:
            std::cout << "type: [" << field.defaultValue.binaryVal.type
                      << "], str: ["
                      << field.defaultValue.binaryVal.str
                      << "], binLen: ["
                      << field.defaultValue.binaryVal.binLen
                      << "]"
                      << std::endl;
            break;
         default:
            std::cout << "unsupported type" << std::endl;
            break;
         }
      }
   }

   static inline INT32 _parseFieldValue(const CHAR* data, INT32 length,
                                        const CHAR* fieldDel, INT32 fieldDelLen,
                                        const CHAR* strDel, INT32 strDelLen,
                                        CSV_TYPE& type, CSV_TYPE& subType,
                                        CSVFieldValue& fieldValue,
                                        INT32& valueLength, BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      if (_startWith(str, len, fieldDel, fieldDelLen))
      {
         type = CSV_TYPE_NULL;
         valueLength = 0;
         fieldEnd = TRUE;
         goto done;
      }

      switch(type)
      {
      case CSV_TYPE_INT:
         rc = _stringToInt(data, length, fieldValue.intVal, valueLength);
         break;
      case CSV_TYPE_LONG:
         rc = _stringToLong(data, length, fieldValue.longVal, valueLength);
         break;
      case CSV_TYPE_NUMBER:
         rc = _stringToNumber(data, length, subType, fieldValue, valueLength);
         break;
      case CSV_TYPE_DOUBLE:
         rc = _stringToDouble(data, length, fieldValue.doubleVal, valueLength);
         break;
      case CSV_TYPE_BOOL:
         rc = _stringToBool(data, length, fieldValue.boolVal, valueLength);
         break;
      case CSV_TYPE_NULL:
         rc = _stringToNull(data, length, fieldDel, fieldDelLen, valueLength, fieldEnd);
         goto done;
      case CSV_TYPE_STRING:
         rc = _stringToString(data, length, strDel, strDelLen, fieldDel,
                              fieldDelLen, fieldValue.strVal, valueLength, fieldEnd);
         goto done;
      case CSV_TYPE_TIMESTAMP:
         rc = _stringToString(data, length, strDel, strDelLen, fieldDel,
                              fieldDelLen, fieldValue.strVal, valueLength, fieldEnd);
         if (SDB_OK != rc)
         {
            goto error;
         }
         rc = _stringToTimestamp(fieldValue.strVal, fieldValue.timestampVal);
         goto done;
      case CSV_TYPE_DATE:
         rc = _stringToString(data, length, strDel, strDelLen, fieldDel,
                              fieldDelLen, fieldValue.strVal, valueLength, fieldEnd);
         if (SDB_OK != rc)
         {
            goto error;
         }
         rc = _stringToDate(fieldValue.strVal, fieldValue.dateVal);
         goto done;
      case CSV_TYPE_OID:
         rc = _stringToString(data, length, strDel, strDelLen, fieldDel,
                              fieldDelLen, fieldValue.strVal, valueLength, fieldEnd);
         if (SDB_OK != rc)
         {
            goto error;
         }
         rc = _stringToOID(fieldValue.strVal, fieldValue.oidVal);
         goto done;
      case CSV_TYPE_REGEX:
         rc = _stringToString(data, length, strDel, strDelLen, fieldDel,
                              fieldDelLen, fieldValue.strVal, valueLength, fieldEnd);
         if (SDB_OK != rc)
         {
            goto error;
         }
         rc = _stringToRegex(fieldValue.strVal, fieldValue.regexVal);
         goto done;
      case CSV_TYPE_BINARY:
         rc = _stringToString(data, length, strDel, strDelLen, fieldDel,
                              fieldDelLen, fieldValue.strVal, valueLength, fieldEnd);
         if (SDB_OK != rc)
         {
            goto error;
         }
         rc = _stringToBinary(fieldValue.strVal, fieldValue.binaryVal);
         goto done;
      case CSV_TYPE_AUTO:
         rc = _detectFieldType(data, length, strDel, strDelLen, fieldDel, fieldDelLen,
                               type, fieldValue, valueLength, fieldEnd);
         SDB_ASSERT(CSV_TYPE_AUTO != type, "type must not be CSV_TYPE_AUTO after detecting field type");
         goto done;
      default:
         rc = SDB_INVALIDARG;
      }

      if (SDB_OK != rc)
      {
         if (CSV_TYPE_NULL != type && CSV_TYPE_AUTO != type)
         {
            rc = _stringToNull(data, length, fieldDel, fieldDelLen, valueLength, fieldEnd);
            if (SDB_OK == rc)
            {
               type = CSV_TYPE_NULL;
               goto done;
            }
         }
         goto error;
      }

      // process non-string type

      str += valueLength;
      len -= valueLength;

      _skipSpace(&str, len);

      if (len == 0)
      {
         valueLength = length;
         goto done;
      }
      else if (_startWith(str, len, fieldDel, fieldDelLen))
      {
         fieldEnd = TRUE;
         valueLength = length - len;
         goto done;
      }
      else
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid field");
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   static inline INT32 _parseFieldName(const CHAR* data, INT32 length,
                                       const CHAR* fieldDel, INT32 fieldDelLen,
                                       string& fieldName, INT32& fieldNameLength,
                                       BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      fieldEnd = FALSE;
 
      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(NULL != fieldDel, "fieldDel can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");
      SDB_ASSERT(fieldDelLen > 0, "fieldDelLen must be greater than 0");

      while (len > 0)
      {
         if (isspace(*str))
         {
            break;
         }
         else if (_startWith(str, len, fieldDel, fieldDelLen))
         {
            fieldEnd = TRUE;
            break;
         }

         str++;
         len--;
      }

      if (len == length)
      {
         rc = SDB_INVALIDARG;
         goto error;
      }

      fieldNameLength = length - len;
      if (!_isValidFieldName((CHAR*)data, fieldNameLength))
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid field name");
         goto error;
      }

      fieldName = string(data, fieldNameLength);

      _skipSpace(&str, len);
      fieldNameLength = length - len;
      if (len != 0 && _startWith(str, len, fieldDel, fieldDelLen))
      {
         fieldEnd = TRUE;
      }

   done:
      return rc;
   error:
      goto done;
   }

   static inline INT32 _parseFieldTypeString(const CHAR* data, INT32 length,
                                             const CHAR* fieldDel, INT32 fieldDelLen,
                                             CSV_TYPE& fieldType, INT32& fieldTypeLength,
                                             BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;
      fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(NULL != fieldDel, "fieldDel can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");
      SDB_ASSERT(fieldDelLen > 0, "fieldDelLen must be greater than 0");

      while (len > 0)
      {
         if (isspace(*str))
         {
            break;
         }
         else if (_startWith(str, len, fieldDel, fieldDelLen))
         {
            fieldEnd = TRUE;
            break;
         }

         str++;
         len--;
      }

      if (len == length)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid field type");
         goto error;
      }

      fieldTypeLength = length - len;

      rc = _convertToCSVType(data, fieldTypeLength, fieldType);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "invalid csv type");
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   static inline INT32 _parseFieldDefaultValue(const CHAR* data, INT32 length,
                                               const CHAR* fieldDel, INT32 fieldDelLen,
                                               const CHAR* strDel, INT32 strDelLen,
                                               CSVField& field, INT32& fieldDefaultLength,
                                               BOOLEAN& fieldEnd)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 valueLen = 0;
      INT32 rc = SDB_OK;
      fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(NULL != fieldDel, "fieldDel can't be NULL");
      SDB_ASSERT(NULL != strDel, "strDel can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");
      SDB_ASSERT(fieldDelLen > 0, "fieldDelLen must be greater than 0");
      SDB_ASSERT(strDelLen > 0, "strDelLen must be greater than 0");

      if (!_startWith(str, len, CSV_STR_DEFAULT, CSV_STR_DEFAULT_SIZE))
      {
         fieldDefaultLength = 0;
         if (_startWith(str, len, fieldDel, fieldDelLen))
         {
            fieldEnd = TRUE;
            goto done;
         }
         else
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "missed \"default\" keyword");
            goto error;
         }
      }

      field.hasDefault = TRUE;

      str += CSV_STR_DEFAULT_SIZE;
      len -= CSV_STR_DEFAULT_SIZE;

      if (len == 0)
      {
         rc = SDB_INVALIDARG;
         goto error;
      }

      if (!isspace(*str))
      {
         rc = SDB_INVALIDARG;
         goto error;
      }

      _skipSpace(&str, len);
      if (len == 0)
      {
         rc = SDB_INVALIDARG;
         goto error;
      }

      rc = _parseFieldValue(str, len,
                            fieldDel, fieldDelLen,
                            strDel, strDelLen,
                            field.type, field.subType,
                            field.defaultValue,
                            valueLen, fieldEnd);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "invalid field value");
         goto error;
      }

      if (CSV_TYPE_NULL == field.type)
      {
         goto error;
      }

      str += valueLen;
      len -= valueLen;
      fieldDefaultLength = length - len;

   done:
      return rc;
   error:
      goto done;
   }

   static inline INT32 _bsonAppendField(bson& obj, CSVField& field, CSVFieldData& data)
   {
      CSV_TYPE type;
      CSVFieldValue* value = NULL;
      INT32 rc = SDB_OK;

      SDB_ASSERT(CSV_TYPE_AUTO != data.type, "data.type can't be CSV_TYPE_AUTO");

      if (CSV_TYPE_NULL == data.type)
      {
         if (CSV_TYPE_AUTO != field.type && field.hasDefault)
         {
            type = (CSV_TYPE_NUMBER == field.type) ? field.subType: field.type;
            value = &(field.defaultValue);
         }
         else
         {
            type = CSV_TYPE_NULL;
         }
      }
      else
      {
         type = (CSV_TYPE_NUMBER == data.type) ? data.subType: data.type;
         value = &(data.value);
      }

      SDB_ASSERT(type > CSV_TYPE_AUTO && type < CSV_TYPE_NUM, "invalid type");

      switch(type)
      {
      case CSV_TYPE_INT:
         rc = bson_append_int(&obj, field.name.c_str(), value->intVal);
         break;
      case CSV_TYPE_LONG:
         rc = bson_append_long(&obj, field.name.c_str(), value->longVal);
         break;
      case CSV_TYPE_DOUBLE:
         rc = bson_append_double(&obj, field.name.c_str(), value->doubleVal);
         break;
      case CSV_TYPE_BOOL:
         rc = bson_append_bool(&obj, field.name.c_str(), value->boolVal);
         break;
      case CSV_TYPE_STRING:
         rc = bson_append_string_n(&obj, field.name.c_str(),
                                   value->strVal.str, value->strVal.length);
         break;
      case CSV_TYPE_NULL:
         rc = bson_append_null(&obj, field.name.c_str());
         break;
      case CSV_TYPE_OID:
         {
            bson_oid_t oid;
            bson_oid_from_string(&oid, value->oidVal.str);
            rc = bson_append_oid(&obj, field.name.c_str(), &oid);
         }
         break;
      case CSV_TYPE_TIMESTAMP:
         rc = bson_append_timestamp2(&obj, field.name.c_str(),
                                     value->timestampVal.sec,
                                     value->timestampVal.us);
         break;
      case CSV_TYPE_DATE:
         rc = bson_append_date(&obj, field.name.c_str(), value->dateVal);
         break;
      case CSV_TYPE_REGEX:
         {
            CHAR* patTerm = NULL;
            CHAR* optTerm = NULL;
            CHAR patCh = '\0';
            CHAR optCh = '\0';

            // terminate string
            patTerm = value->regexVal.pattern + value->regexVal.patternLen;
            patCh= *patTerm;
            *patTerm = '\0';
            optTerm = value->regexVal.option;
            if (*optTerm != '\0')
            {
               optTerm += value->regexVal.optionLen;
               optCh = *optTerm;
               *optTerm = '\0';
            }

            rc = bson_append_regex(&obj, field.name.c_str(),
                                   value->regexVal.pattern,
                                   value->regexVal.option);

            // recovery string
            *patTerm = patCh;
            if (optCh != '\0')
            {
               *optTerm = optCh;
            }
         }
         break;
      case CSV_TYPE_BINARY:
         rc = bson_append_binary(&obj, field.name.c_str(),
                                 value->binaryVal.type,
                                 value->binaryVal.bin,
                                 value->binaryVal.binLen);
         break;
      case CSV_TYPE_AUTO:
      default:
         rc = SDB_INVALIDARG;
         SDB_ASSERT(FALSE, "invalid csv type");
      }

      if (BSON_OK != rc)
      {
         rc = SDB_DRIVER_BSON_ERROR;
      }
      else
      {
         rc = SDB_OK;
      }

      return rc;
   }

   CSVRecordParser::CSVRecordParser(const string& fieldDelimiter,
                          const string& stringDelimiter,
                          BOOLEAN autoAddField,
                          BOOLEAN autoAddValue,
                          BOOLEAN hasHeaderLine)
   : RecordParser(fieldDelimiter,
                  stringDelimiter,
                  autoAddField,
                  autoAddValue),
     _hasHeaderLine(hasHeaderLine)
   {
      _hasId = FALSE;
   }

   CSVRecordParser::~CSVRecordParser()
   {
   }

   // field_name [field_type] [default <default_value>],
   INT32 CSVRecordParser::parseFields(const CHAR* data, INT32 length)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;

      const CHAR* fieldDel = NULL;
      INT32 fieldDelLen = 0;
      const CHAR* strDel = NULL;
      INT32 strDelLen = 0;

      CSVField* field = NULL;
      INT32 fieldCount = 0;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");
      SDB_ASSERT(0 == _fieldVec.size(), "fields already parsed");

      if (_hasHeaderLine)
      {
         fieldDel = _fieldDelimiter.c_str();
         fieldDelLen = _fieldDelimiter.length();
         strDel = _stringDelimiter.c_str();
         strDelLen = _stringDelimiter.length();
      }
      else
      {
         fieldDel = ",";
         fieldDelLen = 1;
         strDel = "\"";
         strDelLen = 1;
      }

      _fields = string(data, length);

      while (len > 0)
      {
         _skipSpace(&str, len);
         if (len == 0)
         {
            if (fieldCount > 0)
            {
               goto done;
            }

            rc = SDB_INVALIDARG;
            goto error;
         }

         field = SDB_OSS_NEW CSVField();
         if (NULL == field)
         {
            rc = SDB_OOM;
            PD_LOG(PDERROR, "failed to create CSVField, rc=%d", rc);
            goto error;
         }

         // field name
         {
            INT32 fieldNameLen = 0;
            BOOLEAN fieldEnd = FALSE;
            rc = _parseFieldName(str, len,
                                 fieldDel, fieldDelLen,
                                 field->name, fieldNameLen, fieldEnd);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to parse field name, rc=%d", rc);
               goto error;
            }

            fieldCount++;
            field->id = fieldCount;

            str += fieldNameLen;
            len -= fieldNameLen;

            if (fieldEnd)
            {
               rc = _pushField(field);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to push field, rc=%d", rc);
                  goto error;
               }
               field = NULL;
               str += fieldDelLen;
               len -= fieldDelLen;
               continue;
            }
         }

         _skipSpace(&str, len);
         if (len == 0)
         {
            rc = _pushField(field);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to push field, rc=%d", rc);
               goto error;
            }
            field = NULL;
            goto done;
         }

         // type
         {
            INT32 fieldTypeLen = 0;
            BOOLEAN fieldEnd = FALSE;
            CSV_TYPE fieldType = CSV_TYPE_AUTO;

            rc = _parseFieldTypeString(str, len, fieldDel, fieldDelLen,
                                 fieldType, fieldTypeLen, fieldEnd);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to parse field type, rc=%d", rc);
               goto error;
            }

            field->type = fieldType;

            str += fieldTypeLen;
            len -= fieldTypeLen;

            if (fieldEnd)
            {
               rc = _pushField(field);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to push field, rc=%d", rc);
                  goto error;
               }
               field = NULL;
               str += fieldDelLen;
               len -= fieldDelLen;
               continue;
            }
         }

         _skipSpace(&str, len);
         if (len == 0)
         {
            rc = _pushField(field);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to push field, rc=%d", rc);
               goto error;
            }
            field = NULL;
            goto done;
         }

         // default
         {
            INT32 fieldDefaultLen = 0;
            BOOLEAN fieldEnd = FALSE;

            rc = _parseFieldDefaultValue(str, len, fieldDel, fieldDelLen,
                                         strDel, strDelLen,
                                         *field, fieldDefaultLen, fieldEnd);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to parse field default value, rc=%d", rc);
               goto error;
            }

            str += fieldDefaultLen;
            len -= fieldDefaultLen;

            if (fieldEnd)
            {
               rc = _pushField(field);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to push field, rc=%d", rc);
                  goto error;
               }
               field = NULL;
               str += fieldDelLen;
               len -= fieldDelLen;
               continue;
            }

            if (len == 0)
            {
               rc = _pushField(field);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to push field, rc=%d", rc);
                  goto error;
               }
               field = NULL;
               goto done;
            }

            if (len > 0)
            {
               rc = SDB_INVALIDARG;
               PD_LOG(PDERROR, "invalid field");
               goto error;
            }
         }
      }

      done:
         return rc;
      error:
         SAFE_OSS_DELETE(field);
         goto done;
   }

   INT32 CSVRecordParser::_pushField(CSVField* field)
   {
      static string recordIdName = RECORD_ID_NAME;
      INT32 size = _fieldVec.size();
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != field, "field can't be NULL");

      for (INT32 i = 0; i < size; i++)
      {
         if (field->name == _fieldVec[i]->name)
         {
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "duplicate field name: %s", field->name.c_str());
            goto error;
         }
      }

      _fieldVec.push_back(field);

      if (CSV_TYPE_OID == field->type && field->name == recordIdName)
      {
         _hasId = TRUE;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 CSVRecordParser::parseRecord(const CHAR* data, INT32 length, bson& obj)
   {
      CHAR* str = (CHAR*)data;
      INT32 len = length;
      INT32 rc = SDB_OK;

      const CHAR* fieldDel = NULL;
      INT32 fieldDelLen = 0;
      const CHAR* strDel = NULL;
      INT32 strDelLen = 0;

      INT32 fieldDefNum = _fieldVec.size();
      INT32 fieldCount = 0;

      CSVField* field = NULL;
      CSVFieldData fieldData;
      INT32 valueLength = 0;
      BOOLEAN fieldEnd = FALSE;

      SDB_ASSERT(NULL != data, "data can't be NULL");
      SDB_ASSERT(length > 0, "length must be greater than 0");

      fieldDel = _fieldDelimiter.c_str();
      fieldDelLen = _fieldDelimiter.length();
      strDel = _stringDelimiter.c_str();
      strDelLen = _stringDelimiter.length();

      bson_init(&obj);

      while (len > 0 && fieldCount < fieldDefNum)
      {
         _skipSpace(&str, len);
         if (len == 0)
         {
            break;
         }

         field = _fieldVec[fieldCount];
         fieldData.reset();
         fieldData.type = field->type;
         fieldData.subType = field->subType;

         rc = _parseFieldValue(str, len,
                               fieldDel, fieldDelLen,
                               strDel, strDelLen,
                               fieldData.type, fieldData.subType,
                               fieldData.value, valueLength, fieldEnd);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to parse field, rc=%d", rc);
            goto error;
         }

         rc = _bsonAppendField(obj, *field, fieldData);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to append field, rc=%d", rc);
            goto error;
         }

         str += valueLength;
         len -= valueLength;
         fieldCount++;

         if (len == 0)
         {
            break;
         }

         if (!fieldEnd)
         {
            goto error;
         }

         str += fieldDelLen;
         len -= fieldDelLen;
      }

      if (len == 0 && fieldCount < fieldDefNum)
      {
         if (_autoAddValue)
         {
            while (fieldCount < fieldDefNum)
            {
               field = _fieldVec[fieldCount];
               fieldData.reset();
               fieldData.type = CSV_TYPE_NULL;

               rc = _bsonAppendField(obj, *field, fieldData);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to append field, rc=%d", rc);
                  goto error;
               }

               fieldCount++;
            }
         }
      }
      else if (len != 0 && fieldCount == fieldDefNum)
      {
         if (_autoAddField)
         {
            CSVField tmpField;

            while (len > 0)
            {
               _skipSpace(&str, len);
               if (len == 0)
               {
                  break;
               }

               fieldData.reset();

               rc = _parseFieldValue(str, len,
                                     fieldDel, fieldDelLen,
                                     strDel, strDelLen,
                                     fieldData.type, fieldData.subType,
                                     fieldData.value, valueLength, fieldEnd);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to parse field, rc=%d", rc);
                  goto error;
               }

               if (CSV_TYPE_NULL == fieldData.type)
               {
                  str += valueLength + fieldDelLen;
                  len -= valueLength + fieldDelLen;
                  SDB_ASSERT(fieldEnd, "CSV_TYPE_NULL must be fieldEnd");
                  continue;
               }

               {
                  stringstream ss;
                  ss << "field" << fieldCount;
                  tmpField.name = ss.str();
               }

               rc = _bsonAppendField(obj, tmpField, fieldData);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to append field, rc=%d", rc);
                  goto error;
               }

               str += valueLength;
               len -= valueLength;
               fieldCount++;

               if (len == 0)
               {
                  break;
               }

               if (!fieldEnd)
               {
                  goto error;
               }

               str += fieldDelLen;
               len -= fieldDelLen;
            }
         }
      }
      else
      {
         SDB_ASSERT(len == 0, "len must be 0");
         SDB_ASSERT(fieldCount == fieldDefNum, "fieldCount must be equals to fieldDefNum");
      }

      if (!_hasId)
      {
         bson_oid_t oid;
         bson_oid_gen(&oid);
         rc = bson_append_oid(&obj, RECORD_ID_NAME, &oid);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to append record id, rc=%d", rc);
            goto error;
         }
      }

      if (BSON_OK != bson_finish(&obj))
      {
         rc = SDB_DRIVER_BSON_ERROR;
         PD_LOG(PDERROR, "failed to finish bson, rc=%d", rc);
         goto error;
      }

   done:
      return rc;
   error:
      bson_destroy(&obj);
      goto done;
   }

   void CSVRecordParser::printFieldsDef()
   {
      INT32 size = _fieldVec.size();

      if (0 == size)
      {
         std::cout << "No fields definition!" << std::endl;
         return;
      }

      for (INT32 i = 0; i < size; i++)
      {
         CSVField* field = _fieldVec[i];
         _printField(*field);
         std::cout << std::endl;
      }
   }
}

