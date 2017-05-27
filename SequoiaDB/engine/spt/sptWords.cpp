/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = sptWords.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          6/4/2017    TZB  Initial Draft

   Last Changed =

*******************************************************************************/
#if defined (_WINDOWS )
#include <windows.h>
#endif

#include "sptWords.hpp"
#include "string.h"
#include <boost/algorithm/string.hpp>

#define PERIOD_CH          "\u3002"
#define COMMA_CH           "\uFF0C"
#define PERIOD_EN          "."
#define COMMA_EN           ","
#define BACKSLASH          "\\"
#define TABLE_LINE_LEN     24

#if defined ( _WINDOWS )
string _gbk2utf8(const string &input)
{
    string output;
    WCHAR *str1 = NULL;
    INT32 n = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, NULL, 0);
    str1 = new WCHAR[n];
    MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char *str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    output = str2;
    delete[] str1;
    str1 = NULL;
    delete[] str2;
    str2 = NULL;
    return output;
}
#endif

void sdbUTF82CharSet(const string &input, vector<string> &output)
{
   string c ;
   INT32 i = 0, len = 0 ;
   for (; i != (INT32)input.length(); i += len )
   {
      UINT8 byte = (UINT8)input[i] ;
      if (byte >= 0xFC) // lenght 6
         len = 6 ;
      else if (byte >= 0xF8)
         len = 5 ;
      else if (byte >= 0xF0)
         len = 4 ;
      else if (byte >= 0xE0)
         len = 3 ;
      else if (byte >= 0xC0)
         len = 2 ;
      else
         len = 1 ;
      c = input.substr( i, len ) ;
      output.push_back( c ) ;
   }
}

void sdbChar2Word( const string &text, vector<string> &output )
{
    INT32 word_len = 0 ;
    vector<string> vec_chars ;
    vector<string>::iterator it ;
    string word ;
    string digit ;
    
    sdbUTF82CharSet( text, vec_chars ) ;
    for( it = vec_chars.begin(); it != vec_chars.end(); it++ )
    {
        word_len = it->length() ;
        if ( word_len == 1 )
        {
            if ( (*it >= "a" && *it <= "z") ||
                 (*it >= "A" && *it <= "Z") )
            {
                if ( !digit.empty() )
                {
                    output.push_back(digit) ;
                    digit.clear() ;
                }
                word += *it ;
            }
            else if ( *it == "+" || *it == "-" || ( *it >= "0" && *it <= "9" ) )
            {
                if ( !word.empty() )
                {
                    output.push_back( word ) ;
                    word.clear() ;
                }
                digit += *it ;
            }
            else
            {
                // save the word and digits which had been cached first
                if ( !word.empty() )
                {
                    output.push_back( word ) ;
                    word.clear() ;
                }
                if ( !digit.empty() ) 
                {
                    output.push_back( digit ) ;
                    digit.clear() ;
                }
                // and then save the current charactor
                output.push_back( *it ) ;
            }
        }
        else if ( word_len > 1 )
        {
            // keep the word and digits which had been cached first
            if ( !word.empty() )
            {
                output.push_back( word ) ;
                word.clear() ;
            }
            if ( !digit.empty() ) 
            {
                output.push_back( digit ) ;
                digit.clear() ;
            }
            // and then keep the current charactor
            output.push_back( *it ) ;
        }
    } // for
    // keep the word and digits we had cached
    if ( !word.empty() )
    {
        output.push_back( word ) ;
        word.clear() ;
    }
    if ( !digit.empty() ) 
    {
        output.push_back( digit ) ;
        digit.clear() ;
    }
}

void sdbSplitWords( const string &text, INT32 lineLen, vector<string> &vec_out )
{
    string one_line ;
    string pre_char ;
    vector<string> vec_words ;
    vector<string>::iterator it ;
    INT32 left = lineLen ;

    // get words, the output is: "abc", " ", ",", "1", "1024", "集合",...
    sdbChar2Word( text, vec_words ) ;

    // build line
    for( it = vec_words.begin(); it != vec_words.end(); it++ )
    {
        left -= it->length() ;
        if (left > 0)
        {
            one_line += *it ;
            pre_char = *it ;
        }
        else
        {
            BOOLEAN has_handle = FALSE ;
            // if it's "," and ".", we still let 
            // it put to the end of the line
            if ( *it == COMMA_EN ||
                 *it == PERIOD_EN ||
                 pre_char == BACKSLASH ||
#if defined ( _WINDOWS )
                 *it == _gbk2utf8(COMMA_CH) ||
                 *it == _gbk2utf8(PERIOD_CH) )
#else
                 *it == COMMA_CH ||
                 *it == PERIOD_CH )
#endif
            {
                one_line += *it ;
                has_handle = TRUE ;
            }
            // put the current line into vector
            if ( !one_line.empty() )
            {
                boost::trim_left( one_line ) ;
                vec_out.push_back( one_line ) ;
            }
            // clean up
            one_line.clear() ;
            pre_char = "" ;
            // try to start a new line
            if ( !has_handle )
            {
                // start a new line
                one_line += *it ;
                pre_char = *it ;
                left = lineLen - it->length() ;
            }
            else
            {
                left = lineLen ;
            }
        }
    }
    // put the last line to the output
    if ( !one_line.empty() )
    {
        boost::trim_left( one_line ) ;
        vec_out.push_back( one_line ) ;
    }
}
