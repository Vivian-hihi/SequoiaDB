/*    Copyright 2012 SequoiaDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/* This list file is automatically generated,you shoud NOT modify this file anyway! test comment*/
#ifndef utilTRACE_H__
#define utilTRACE_H__
// Component: util
#define SDB_WIN32READ                                      0x1000000000003fcL
#define SDB_LNSETMULTILINE                                 0x1000000000003fdL
#define SDB_ISUNSUPPORTTERM                                0x1000000000003feL
#define SDB_ENABLERAWMODE                                  0x1000000000003ffL
#define SDB_DISABLERAWMODE                                 0x100000000000400L
#define SDB_GETCURSORPOSITION                              0x100000000000401L
#define SDB_GETCOLUMNS                                     0x100000000000402L
#define SDB_LNCLEARSCREEN                                  0x100000000000403L
#define SDB_LNBEEP                                         0x100000000000404L
#define SDB_FREECOMPLETIONS                                0x100000000000405L
#define SDB_COMPLETELINE                                   0x100000000000406L
#define SDB_LNHISTORYCLEAR                                 0x100000000000407L
#define SDB_LNSETCPLCALLBACK                               0x100000000000408L
#define SDB_LNADDCPL                                       0x100000000000409L
#define SDB_ABINIT                                         0x10000000000040aL
#define SDB_ABAPPEND                                       0x10000000000040bL
#define SDB_ABFREE                                         0x10000000000040cL
#define SDB_CALCHIGHLIGHTPOS                               0x10000000000040dL
#define SDB_SETDISPLAYATTR                                 0x10000000000040eL
#define SDB_REFRESHSINGLELINE                              0x10000000000040fL
#define SDB_REFRESHMULTILINE                               0x100000000000410L
#define SDB_REFRESHLINE                                    0x100000000000411L
#define SDB_LNEDITINSERT                                   0x100000000000412L
#define SDB_LNEDITMOVELEFT                                 0x100000000000413L
#define SDB_LNEDITMOVERIGHT                                0x100000000000414L
#define SDB_LNEDITMOVEHOME                                 0x100000000000415L
#define SDB_LNEDITMOVEEND                                  0x100000000000416L
#define SDB_LNEDITHISTORYNEXT                              0x100000000000417L
#define SDB_LNEDITDELETE                                   0x100000000000418L
#define SDB_LNEDITBACKSPACE                                0x100000000000419L
#define SDB_LNEDITDELPREVWORD                              0x10000000000041aL
#define SDB_LNEDIT                                         0x10000000000041bL
#define SDB_LNPRINTKEYCODES                                0x10000000000041cL
#define SDB_LNRAW                                          0x10000000000041dL
#define SDB_LN                                             0x10000000000041eL
#define SDB_FREEHISTORYINLINENOISE                         0x10000000000041fL
#define SDB_LNNOISEATEXT                                   0x100000000000420L
#define SDB_LNHISTORYGET                                   0x100000000000421L
#define SDB_LNHISTORYADD                                   0x100000000000422L
#define SDB_LNHISTORYSETMAXLEN                             0x100000000000423L
#define SDB_LNHISTORYSAVE                                  0x100000000000424L
#define SDB_LNHISTORYLOAD                                  0x100000000000425L
#define SDB_SETDISPLAYATTRIBUTE                            0x100000000000426L
#define SDB_BSONGEN_GENRAND                                0x100000000000427L
#define SDB_BSONGEN_GENOBJ                                 0x100000000000428L
#define SDB_BSONGEN_GENARR                                 0x100000000000429L
#define SDB__UTILBSONHASHER_HASHOBJ                        0x10000000000042aL
#define SDB__UTILBSONHASHER_HASHELE                        0x10000000000042bL
#define SDB__UTILBSONHASHER_HASH                           0x10000000000042cL
#define SDB__UTILCSV__INIT                                 0x10000000000042dL
#define SDB__UTILCSV__GETNEXTRECORD                        0x10000000000042eL
#define SDB__UTILCOMPRESSORLZ4_COMPRESSBOUND               0x10000000000042fL
#define SDB__UTILCOMPRESSORLZ4_COMPRESS                    0x100000000000430L
#define SDB__UTILCOMPRESSORLZ4_GETUNCOMPRESSEDLEN          0x100000000000431L
#define SDB__UTILCOMPRESSORLZ4_DECOMPRESS                  0x100000000000432L
#define SDB__UTILCOMPRESSORSNAPPY_COMPRESSBOUND            0x100000000000433L
#define SDB__UTILCOMPRESSORSNAPPY_COMPRESS                 0x100000000000434L
#define SDB__UTILCOMPRESSORSNAPPY_GETUNCOMPRESSEDLEN       0x100000000000435L
#define SDB__UTILCOMPRESSORSNAPPY_DECOMPRESS               0x100000000000436L
#define SDB__UTILCOMPRESSORZLIB_COMPRESSBOUND              0x100000000000437L
#define SDB__UTILCOMPRESSORZLIB_COMPRESS                   0x100000000000438L
#define SDB__UTILCOMPRESSORZLIB_GETUNCOMPRESSEDLEN         0x100000000000439L
#define SDB__UTILCOMPRESSORZLIB_DECOMPRESS                 0x10000000000043aL
#define SDB__UTILLZWDICTIONARY_INIT                        0x10000000000043bL
#define SDB__UTILLZWDICTIONARY_RESET                       0x10000000000043cL
#define SDB__UTILLZWDICTIONARY__FORMATONEGRP               0x10000000000043dL
#define SDB__UTILLZWDICTIONARY__INITFINALENV               0x10000000000043eL
#define SDB__UTILLZWDICTIONARY__FORMATREMOTESTR            0x10000000000043fL
#define SDB__UTILLZWDICTIONARY__FORMATONECODE              0x100000000000440L
#define SDB__UTILLZWDICTIONARY__FORMATDST                  0x100000000000441L
#define SDB__UTILLZWDICTIONARY__CALCCODESIZE               0x100000000000442L
#define SDB__UTILLZWDICTIONARY__ADDADDITIONALINFO          0x100000000000443L
#define SDB__UTILLZWDICTIONARY__SETVARLENSPLITINFO         0x100000000000444L
#define SDB__UTILLZWDICTIONARY_FINALIZE                    0x100000000000445L
#define SDB__UTILLZWDICTCREATOR_PREPARE                    0x100000000000446L
#define SDB__UTILLZWDICTCREATOR_RESET                      0x100000000000447L
#define SDB__UTILLZWDICTCREATOR_BUILD                      0x100000000000448L
#define SDB__UTILLZWDICTCREATOR_FINALIZE                   0x100000000000449L
#define SDB__UTILJSONPS__INIT                              0x10000000000044aL
#define SDB__UTILJSONPS__GETNEXTRECORD                     0x10000000000044bL
#define SDB__UTILCOMPRESSORLZW_CONSTRUCTOR                 0x10000000000044cL
#define SDB__UTILCOMPRESSORLZW__COMPRESSLEVELONE           0x10000000000044dL
#define SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTWO           0x10000000000044eL
#define SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTHREE         0x10000000000044fL
#define SDB__UTILCOMPRESSORLZW__DECODEFIXLENCODE           0x100000000000450L
#define SDB__UTILCOMPRESSORLZW__DECODEVARLENCODE           0x100000000000451L
#define SDB__UTILCOMPRESSORLZW_COMPRESSBOUND               0x100000000000452L
#define SDB__UTILCOMPRESSORLZW_COMPRESS                    0x100000000000453L
#define SDB__UTILCOMPRESSORLZW_GETUNCOMPRESSLEN            0x100000000000454L
#define SDB__UTILCOMPRESSORLZW_DECOMPRESS                  0x100000000000455L
#define SDB__LINENOISECMDBLD__RELSNODE                     0x100000000000456L
#define SDB__LINENOISECMDBLD__LOADCMD                      0x100000000000457L
#define SDB__LINENOISECMDBLD__ADDCMD                       0x100000000000458L
#define SDB__LINENOISECMDBLD__DELCMD                       0x100000000000459L
#define SDB__LINENOISECMDBLD__INSERT                       0x10000000000045aL
#define SDB__LINENOISECMDBLD__GETCOMPLETIONS2              0x10000000000045bL
#define SDB__LINENOISECMDBLD__PREFIND                      0x10000000000045cL
#define SDB__LINENOISECMDBLD__GETCOMPLETIONS               0x10000000000045dL
#define SDB_LINECOMPLETE                                   0x10000000000045eL
#define SDB_CANCONTINUENXTLINE                             0x10000000000045fL
#define SDB_HISTORYCLEAR                                   0x100000000000460L
#define SDB_GETNXTCMD                                      0x100000000000461L
#define SDB_HISTORYINIT                                    0x100000000000462L
#define SDB_FROMJSON                                       0x100000000000463L
#define SDB_BASE64_ENCODE                                  0x100000000000464L
#define SDB_BASE64_DECODE                                  0x100000000000465L
#endif
