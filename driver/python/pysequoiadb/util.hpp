#ifndef _SDB_PYTHON_DRIVER_UTIL_HPP_
#define _SDB_PYTHON_DRIVER_UTIL_HPP_

/*
 * some useful macros
 **/
#define PYOBJECT PyObject

#define PARSE_PYTHON_ARGS PyArg_ParseTuple

/*
 *@brief     macro to cast INT32 to object of Python
 *@ret_value [in] the exactly value to cast
 *@return    pointer to PyObject, reference to error code
 **/
#define MAKE_RETURN_INT_VALUE( ret_value ) \
   ( PyObject * )Py_BuildValue( "i", ret_value )

#define MAKE_RETURN_INT_OBJECT( ret_value, cpp_object ) \
   ( PyObject * )Py_BuildValue( "iO", ret_value, cpp_object )

/*
 *@brief      macro to cast C++ object to object of Python, 
              it will be used in creating a instance of C++ class
 *@cpp_object [in] the exactly object to cast
 *@dtor_func  [in] the destructor function for delete C++ object
 *@return     pointer to PyObject
 **/
#define MAKE_RETURN_OBJECT( cpp_object ) \
   ( PyObject * )PyCObject_FromVoidPtr( cpp_object, NULL )

#define MAKE_PYTHON_VOID_OBJECT( cpp_object, py_object ) \
   py_object = MAKE_RETURN_OBJECT( cpp_object )

/*
 *@brief     macro to re-cast python object to specified class
 *@py_object [in] object need to cast
 *@tmp       [in] a temp pointer
 *@calssname [in] the class of the instance
 *@instance  [out] the pointer pointing to real object
 **/
#define CAST_PYOBJECT_TO_COBJECT( py_object, tmp, classname, instance ) \
   tmp = PyCObject_AsVoidPtr( obj ) ;                                   \
   if ( NULL == tmp )                                                   \
   {                                                                    \
      rc = SDB_INVALIDARGS ;                                            \
      goto done ;                                                       \
   }                                                                    \
                                                                        \
   instance = static_cast< classname * >( tmp ) ;                       \
   if ( NULL == instance )                                              \
   {                                                                    \
      rc = SDB_INVALIDARGS ;                                            \
      goto done ;                                                       \
   }

/*
 *@brief     macro to re-cast python object to specified class
 *@py_object [in] object need to cast
 *@str_bson  [in] a temp pointer
 *@calssname [in] the class of the instance
 *@instance  [out] the pointer pointing to real object
 **/
#define CAST_PYBSON_TO_CSTRING( py_object, str )      \
   str = PyBytes_AsString( py_object ) ;              \
   if ( NULL == str )                                 \
   {                                                  \
      rc = SDB_INVALIDARGS ;                          \
      goto done ;                                     \
   }                                                  \
                                                      \
   bson_object = SDB_OSS_NEW bson::BSONObj( tmp ) ;   \
   if ( NULL == bson_object )                         \
   {                                                  \
      rc = SDB_INVALIDARGS ;                          \
      goto done ;                                     \
   }

#define DEC_PYOBJECT_REF( py_object )  \
   if ( NULL != client )               \
   {                                   \
      Py_DECREF( obj ) ;               \
   }

#define DELETE_CPPOBJECT( pObject ) \
   if ( NULL != pObject )           \
   {                                \
      SDB_OSS_DEL pObject ;         \
      pObject = NULL ;              \
   }
   
#endif