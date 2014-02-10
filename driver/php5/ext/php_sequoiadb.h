/** \file php_sequoiadb.h
    \brief XXX
 */
#ifndef PHP_SEQUOIADB_H
#define PHP_SEQUOIADB_H

#define PHP_SEQUOIADB_VERSION "1.0.0"
extern zend_module_entry sequoiadb_module_entry;
#define phpext_sequoiadb_ptr &sequoiadb_module_entry

#ifdef PHP_WIN32
#define PHP_SEQUOIADB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_SEQUOIADB_API __attribute__ ((visibility("default")))
#else
#define PHP_SEQUOIADB_API PHPAPI
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_JSON_OUTPUT_ARRAY	 0
#define PHP_JSON_OUTPUT_OBJECT 1

#define PHP_GET_VALUE_ERROR -2
#define PHP_GET_VALUE_NOTFIND -1

#define RETURN_ARRAY_TYPE TRUE
#define RETURN_STRING_TYPE FALSE


PHP_SEQUOIADB_API void json_encode_array ( CHAR **buf,
                                           INT32 &bufSize,
                                           INT32 &leftLen,
                                           zval **val TSRMLS_DC ) ;
PHP_SEQUOIADB_API void php_json_encode ( CHAR **buf,
                                         INT32 &bufSize,
                                         INT32 &leftLen,
                                         zval *val TSRMLS_DC ) ;
PHP_SEQUOIADB_API BOOLEAN php_json_decode ( const CHAR *buf,
                                            zval **val TSRMLS_DC ) ;
PHP_SEQUOIADB_API BOOLEAN php_toJson ( CHAR **buf, zval *val TSRMLS_DC );
PHP_SEQUOIADB_API BOOLEAN php_jsonArr2Vector ( std::vector<char *> *pVector, zval *val TSRMLS_DC ) ;
PHP_SEQUOIADB_API INT32 key_get_value ( zval *val, CHAR *key TSRMLS_DC ) ;
PHP_SEQUOIADB_API BOOLEAN php_toNum64 ( INT64 &num64, zval *val TSRMLS_DC ) ;
PHP_SEQUOIADB_API void map_encode_array(std::map<std::string,std::string> *pMap,
                                        zval **val TSRMLS_DC ) ;
PHP_SEQUOIADB_API void php_map_encode(std::map<std::string,std::string> *pMap,
                                      zval *val TSRMLS_DC ) ;
PHP_SEQUOIADB_API BOOLEAN php_to_map ( zval *val,
                                       std::map<std::string,std::string> *pMap TSRMLS_DC ) ;

PHP_MINIT_FUNCTION(sequoiadb);
PHP_MSHUTDOWN_FUNCTION(sequoiadb);
PHP_RINIT_FUNCTION(sequoiadb);
PHP_RSHUTDOWN_FUNCTION(sequoiadb);
PHP_MINFO_FUNCTION(sequoiadb);


PHP_METHOD ( SequoiaDB, __construct ) ;
PHP_METHOD ( SequoiaDB, __destruct ) ;
PHP_METHOD ( SequoiaDB, install ) ;
PHP_METHOD ( SequoiaDB, getError ) ;
PHP_METHOD ( SequoiaDB, connect ) ;
PHP_METHOD ( SequoiaDB, close ) ;
PHP_METHOD ( SequoiaDB, execSQL ) ;
PHP_METHOD ( SequoiaDB, execUpdateSQL ) ;
PHP_METHOD ( SequoiaDB, getSnapshot ) ;
PHP_METHOD ( SequoiaDB, getList ) ;
PHP_METHOD ( SequoiaDB, selectShard ) ;
PHP_METHOD ( SequoiaDB, resetSnapshot ) ;
PHP_METHOD ( SequoiaDB, selectCS ) ;
PHP_METHOD ( SequoiaDB, listCSs ) ;
PHP_METHOD ( SequoiaDB, listCollections ) ;
PHP_METHOD ( SequoiaDB, createCataShard ) ;
PHP_METHOD ( SequoiaDB, dropCollectionSpace ) ;

PHP_METHOD ( SequoiaCS, __construct ) ;
PHP_METHOD ( SequoiaCS, __destruct ) ;
PHP_METHOD ( SequoiaCS, selectCollection ) ;
PHP_METHOD ( SequoiaCS, drop ) ;
PHP_METHOD ( SequoiaCS, dropCollection ) ;
PHP_METHOD ( SequoiaCS, getName ) ;

PHP_METHOD ( SequoiaCollection, __construct ) ;
PHP_METHOD ( SequoiaCollection, __destruct ) ;
PHP_METHOD ( SequoiaCollection, insert ) ;
PHP_METHOD ( SequoiaCollection, update ) ;
PHP_METHOD ( SequoiaCollection, remove ) ;
PHP_METHOD ( SequoiaCollection, find ) ;
PHP_METHOD ( SequoiaCollection, split ) ;
PHP_METHOD ( SequoiaCollection, drop ) ;
PHP_METHOD ( SequoiaCollection, aggregate ) ;
PHP_METHOD ( SequoiaCollection, createIndex ) ;
PHP_METHOD ( SequoiaCollection, deleteIndex ) ;
PHP_METHOD ( SequoiaCollection, getIndex ) ;
PHP_METHOD ( SequoiaCollection, getCSName ) ;
PHP_METHOD ( SequoiaCollection, getCollectionName ) ;
PHP_METHOD ( SequoiaCollection, getFullName ) ;
PHP_METHOD ( SequoiaCollection, count ) ;

PHP_METHOD ( SequoiaCursor, __construct ) ;
PHP_METHOD ( SequoiaCursor, __destruct ) ;
PHP_METHOD ( SequoiaCursor, getNext ) ;
PHP_METHOD ( SequoiaCursor, current ) ;
//PHP_METHOD ( SequoiaCursor, updateCurrent ) ;
//PHP_METHOD ( SequoiaCursor, deleteCurrent ) ;

PHP_METHOD ( SequoiaID, __construct ) ;
PHP_METHOD ( SequoiaID, __toString ) ;

PHP_METHOD ( SequoiaDate, __construct ) ;
PHP_METHOD ( SequoiaDate, __toString ) ;

PHP_METHOD ( SequoiaTimestamp, __construct ) ;
PHP_METHOD ( SequoiaTimestamp, __toString ) ;

PHP_METHOD ( SequoiaRegex, __construct ) ;
PHP_METHOD ( SequoiaRegex, __toString ) ;

PHP_METHOD ( SequoiaINT64, __construct ) ;
PHP_METHOD ( SequoiaINT64, __toString ) ;

/* ********* shard ************ */

PHP_METHOD ( sequoiaReplicaShard, getNodeNum ) ;
PHP_METHOD ( sequoiaReplicaShard, getDetail ) ;
PHP_METHOD ( sequoiaReplicaShard, getMaster ) ;
PHP_METHOD ( sequoiaReplicaShard, getSlave ) ;
PHP_METHOD ( sequoiaReplicaShard, getNode ) ;
PHP_METHOD ( sequoiaReplicaShard, createNode ) ;
PHP_METHOD ( sequoiaReplicaShard, start ) ;
PHP_METHOD ( sequoiaReplicaShard, stop ) ;
PHP_METHOD ( sequoiaReplicaShard, isCatalog ) ;


/* ************* node ***************** */

PHP_METHOD ( sequoiaReplicaNode, stop ) ;
PHP_METHOD ( sequoiaReplicaNode, start ) ;
PHP_METHOD ( sequoiaReplicaNode, getNodeName ) ;
PHP_METHOD ( sequoiaReplicaNode, getServiceName ) ;
PHP_METHOD ( sequoiaReplicaNode, getHostName ) ;
PHP_METHOD ( sequoiaReplicaNode, getStatus ) ;
PHP_METHOD ( sequoiaReplicaNode, connect ) ;




#ifdef ZTS
#define SEQUOIADB_G(v) TSRMG(sequoiadb_globals_id, zend_sequoiadb_globals *, v)
#else
#define SEQUOIADB_G(v) (sequoiadb_globals.v)
#endif

#endif
