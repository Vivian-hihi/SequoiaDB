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
#define MAKE_RETURN_INT( ret_value ) \
   ( PyObject * )Py_BuildValue( "i", ret_value )

#define MAKE_RETURN_INT_INT( ret_value, int_value ) \
   ( PyObject * )Py_BuildValue( ("i,i"), ret_value, int_value )

#define MAKE_RETURN_INT_LONG( ret_value, long_value ) \
   ( PyObject * )Py_BuildValue( ("i,l"), ret_value, long_value )

#define MAKE_RETURN_INT_PYSTRING( ret_value, c_string ) \
   ( PyObject * )Py_BuildValue( ("i,s"), ret_value, c_string )
/*
 *@brief      macro to cast C++ object to object of Python, 
              it will be used in creating a instance of C++ class
 *@cpp_object [in] the exactly object to cast
 *@dtor_func  [in] the destructor function for delete C++ object
 *@return     pointer to PyObject
 **/
#define MAKE_RETURN_OBJECT( cpp_object ) \
   ( PyObject * )PyCObject_FromVoidPtr( cpp_object, NULL )

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
#define CAST_PYBSON_TO_CPPBSON( py_object, bson_object )       \
   if ( NULL == py_object )                                    \
   {                                                           \
      bson_object = &sdbclient::_sdbStaticObject ;             \
   }                                                           \
   else                                                        \
   {                                                           \
      void *tmp = PyBytes_AsString( py_object ) ;              \
      if ( NULL == tmp )                                       \
      {                                                        \
         rc = SDB_INVALIDARGS ;                                \
         goto done ;                                           \
      }                                                        \
                                                               \
      bson_object = SDB_OSS_NEW bson::BSONObj( tmp ) ;         \
      if ( NULL == bson_object )                               \
      {                                                        \
         rc = SDB_OOM ;                                        \
         goto done ;                                           \
      }                                                        \
   }
   

#define MAKE_PYLIST_TO_VECTOR( py_list, list_size, tmp, vec_bson )         \
   if( !PyList_Check( py_list) )                                           \
   {                                                                       \
      rc = SDB_INVALIDARGS ;                                               \
      goto done ;                                                          \
   }                                                                       \
                                                                           \
   list_size = PyList_Size( py_list ) ;                                    \
   for ( idx = 0 ; idx < list_size ; ++idx )                               \
   {                                                                       \
      bson::BSONObj *obj = NULL ;                                          \
      CAST_PYBSON_TO_CPPBSON( PyList_GetItem( py_list, idx), tmp, obj ) ;  \
      vec_bson.push_back( *obj ) ;                                         \
      DELETE_CPPOBJECT( obj ) ;                                            \
   }


#define INC_PYOBJECT_REF( py_object )  \
   if ( NULL != py_object )            \
   {                                   \
      Py_IncRef( obj ) ;               \
   }

#define DEC_PYOBJECT_REF( py_object )  \
   if ( NULL != py_object )            \
   {                                   \
      Py_DecRef( obj ) ;               \
   }

#define DELETE_CPPOBJECT( pObject ) \
   if ( NULL != pObject )           \
   {                                \
      SDB_OSS_DEL pObject ;         \
      pObject = NULL ;              \
   }

#define DEFINE_MODULE(modulename, methods)             \
static struct PyModuleDef moduledef = {                \
   PyModuleDef_HEAD_INIT,                              \
   #modulename,                                        \
   NULL,                                               \
   sizeof(struct module_state),                        \
   methods,                                            \
   NULL,                                               \
   NULL,                                               \
   NULL,                                               \
   NULL                                                \
};                                                     \
                                                
#define CREATE_MODULE(modulename, methods)             \
#if PY_MAJOR_VERSION >= 3                              \                       
#define INITERROR return NULL                          \ 
DEFINE_MODULE(modulename, methods)                     \                
PyMODINIT_FUNC                                         \
PyInit__##modulename(void)                             \
#else                                                  \                                      
#define INITERROR return                               \
PyMODINIT_FUNC                                         \
init##modulename(void)                                 \
#endif                                                 \
{                                                      \
   PyObject *m;                                        \   
#if PY_MAJOR_VERSION >= 3                              \
   m = PyModule_Create(&moduledef);                    \
#else                                                  \
   m = Py_InitModule(#modulename, methods);            \  
#endif                                                 \
   if (m == NULL)                                      \
   {                                                   \
      INITERROR;                                       \
   }                                                   \
                                                       \
#if PY_MAJOR_VERSION >= 3                              \
   return m;                                           \
#endif                                                 \
}

#endif