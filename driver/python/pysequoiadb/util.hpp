#ifndef _SDB_PYTHON_DRIVER_UTIL_HPP_
#define _SDB_PYTHON_DRIVER_UTIL_HPP_

/*
 * some useful macros
 **/
#define PYOBJECT PyObject

#define PARSE_PYTHON_ARGS PyArg_ParseTuple

#define MAKE_RETURN_INT_VALUE( ret_value ) \
   ( PyObject * )Py_BuildValue( "i", ret_value )

#define MAKE_RETURN_OBJECT(cpp_object, dtor_func) \
   PyObject_FromVoidPtr( cpp_object, dtor_func )

#define CAST_PYOBJECT_TO_COBJECT( py_object, tmp, instance, classname ) \
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

#endif