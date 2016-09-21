// BasicBSONObject.java

/**
 *      Copyright (C) 2008 10gen Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

package org.bson;

// BSON
import java.beans.BeanInfo;
import java.beans.IntrospectionException;
import java.beans.Introspector;
import java.beans.PropertyDescriptor;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.math.BigDecimal;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.UUID;
import java.util.regex.Pattern;

import org.bson.types.BSONDecimal;
import org.bson.types.BSONTimestamp;
import org.bson.types.BasicBSONList;
import org.bson.types.Binary;
import org.bson.types.Code;
import org.bson.types.CodeWScope;
import org.bson.types.MaxKey;
import org.bson.types.MinKey;
import org.bson.types.ObjectId;
import org.bson.types.Symbol;
import org.bson.util.JSON;

// Java

/**
 * A simple implementation of <code>DBObject</code>. A <code>DBObject</code> can
 * be created as follows, using this class: <blockquote>
 * 
 * <pre>
 * DBObject obj = new BasicBSONObject();
 * obj.put(&quot;foo&quot;, &quot;bar&quot;);
 * </pre>
 * 
 * </blockquote>
 */
public class BasicBSONObject implements Map<String, Object>, BSONObject {
	private static final long serialVersionUID = -4415279469780082174L;
	private Map<String, Object> _objectMap = null;

	/**
	 * Creates an empty object.
	 * 
	 * @param sort
	 *            true: key will be sorted false: key won't be sorted.
	 */
	public BasicBSONObject(boolean sort) {
		if (sort) {
			_objectMap = new TreeMap<String, Object>();
		} else {
			_objectMap = new LinkedHashMap<String, Object>();
		}
	}

	/**
	 * Creates an empty object. by default, key won't be sorted
	 */
	public BasicBSONObject() {
		this(false);
	}

	public BasicBSONObject(int size) {
		this(false);
	}

	public boolean isEmpty() {
		return _objectMap.size() == 0;
	}

	/**
	 * Convenience CTOR
	 * 
	 * @param key
	 *            key under which to store
	 * @param value
	 *            value to stor
	 */
	public BasicBSONObject(String key, Object value) {
		this(false);
		put(key, value);
	}

	/**
	 * Creates a DBObject from a map.
	 * 
	 * @param m
	 *            map to convert
	 */
	@SuppressWarnings({ "unchecked" })
	public BasicBSONObject(Map m) {
		_objectMap = new LinkedHashMap<String, Object>(m);
	}

	/**
	 * Converts a DBObject to a map.
	 * 
	 * @return the DBObject
	 */
	// @Override
	public Map toMap() {
		if (_objectMap instanceof LinkedHashMap) {
			return new LinkedHashMap<String, Object>(_objectMap);
		} else {
			return new TreeMap<String, Object>(_objectMap);
		}
	}

	/**
	 * Deletes a field from this object.
	 * 
	 * @param key
	 *            the field name to remove
	 * @return the object removed
	 */
	// @Override
	public Object removeField(String key) {
		return _objectMap.remove(key);
	}

	/**
	 * Checks if this object contains a given field
	 * 
	 * @param field
	 *            field name
	 * @return if the field exists
	 */
	// @Override
	public boolean containsField(String field) {
		return _objectMap.containsKey(field);
	}

	/**
	 * @deprecated
	 */
	// @Override
	@Deprecated
	public boolean containsKey(String key) {
		return containsField(key);
	}

	/**
	 * Gets a value from this object
	 * 
	 * @param key
	 *            field name
	 * @return the value
	 */
	// @Override
	public Object get(String key) {
		return _objectMap.get(key);
	}

	/**
	 * Returns the value of a field as an <code>int</code>.
	 * 
	 * @param key
	 *            the field to look for
	 * @return the field value (or default)
	 */
	public int getInt(String key) {
		Object o = get(key);
		if (o == null)
			throw new NullPointerException("no value for: " + key);

		return BSON.toInt(o);
	}

	/**
	 * Returns the value of a field as an <code>int</code>.
	 * 
	 * @param key
	 *            the field to look for
	 * @param def
	 *            the default to return
	 * @return the field value (or default)
	 */
	public int getInt(String key, int def) {
		Object foo = get(key);
		if (foo == null)
			return def;

		return BSON.toInt(foo);
	}

	/**
	 * Returns the value of a field as a <code>long</code>.
	 * 
	 * @param key
	 *            the field to return
	 * @return the field value
	 */
	public long getLong(String key) {
		Object foo = get(key);
		return ((Number) foo).longValue();
	}

	/**
	 * Returns the value of a field as an <code>long</code>.
	 * 
	 * @param key
	 *            the field to look for
	 * @param def
	 *            the default to return
	 * @return the field value (or default)
	 */
	public long getLong(String key, long def) {
		Object foo = get(key);
		if (foo == null)
			return def;

		return ((Number) foo).longValue();
	}

	/**
	 * Returns the value of a field as a <code>double</code>.
	 * 
	 * @param key
	 *            the field to return
	 * @return the field value
	 */
	public double getDouble(String key) {
		Object foo = get(key);
		return ((Number) foo).doubleValue();
	}

	/**
	 * Returns the value of a field as an <code>double</code>.
	 * 
	 * @param key
	 *            the field to look for
	 * @param def
	 *            the default to return
	 * @return the field value (or default)
	 */
	public double getDouble(String key, double def) {
		Object foo = get(key);
		if (foo == null)
			return def;

		return ((Number) foo).doubleValue();
	}

	/**
	 * Returns the value of a field as a string
	 * 
	 * @param key
	 *            the field to look up
	 * @return the value of the field, converted to a string
	 */
	public String getString(String key) {
		Object foo = get(key);
		if (foo == null)
			return null;
		return foo.toString();
	}

	/**
	 * Returns the value of a field as a string
	 * 
	 * @param key
	 *            the field to look up
	 * @param def
	 *            the default to return
	 * @return the value of the field, converted to a string
	 */
	public String getString(String key, final String def) {
		Object foo = get(key);
		if (foo == null)
			return def;

		return foo.toString();
	}

	/**
	 * Returns the value of a field as a boolean.
	 * 
	 * @param key
	 *            the field to look up
	 * @return the value of the field, or false if field does not exist
	 */
	public boolean getBoolean(String key) {
		return getBoolean(key, false);
	}

	/**
	 * Returns the value of a field as a boolean
	 * 
	 * @param key
	 *            the field to look up
	 * @param def
	 *            the default value in case the field is not found
	 * @return the value of the field, converted to a string
	 */
	public boolean getBoolean(String key, boolean def) {
		Object foo = get(key);
		if (foo == null)
			return def;
		if (foo instanceof Number)
			return ((Number) foo).intValue() > 0;
		if (foo instanceof Boolean)
			return ((Boolean) foo).booleanValue();
		throw new IllegalArgumentException("can't coerce to bool:"
				+ foo.getClass());
	}

	/**
	 * Returns the object id or null if not set.
	 * 
	 * @param field
	 *            The field to return
	 * @return The field object value or null if not found (or if null :-^).
	 */
	public ObjectId getObjectId(final String field) {
		return (ObjectId) get(field);
	}

	/**
	 * Returns the object id or def if not set.
	 * 
	 * @param field
	 *            The field to return
	 * @param def
	 *            the default value in case the field is not found
	 * @return The field object value or def if not set.
	 */
	public ObjectId getObjectId(final String field, final ObjectId def) {
		final Object foo = get(field);
		return (foo != null) ? (ObjectId) foo : def;
	}

	/**
	 * Returns the date or null if not set.
	 * 
	 * @param field
	 *            The field to return
	 * @return The field object value or null if not found.
	 */
	public Date getDate(final String field) {
		return (Date) get(field);
	}

	/**
	 * Returns the date or def if not set.
	 * 
	 * @param field
	 *            The field to return
	 * @param def
	 *            the default value in case the field is not found
	 * @return The field object value or def if not set.
	 */
	public Date getDate(final String field, final Date def) {
		final Object foo = get(field);
		return (foo != null) ? (Date) foo : def;
	}

	/**
	 * Returns the BigDecimal object or null if not set.
	 * 
	 * @param field
	 *            The field to return
	 * @return The field object value or null if not found.
	 */
	public BigDecimal getBigDecimal(final String field) {
		Object obj = get(field);
		if (obj == null) {
			return null;
		}
		if (obj instanceof BigDecimal) {
			return (BigDecimal)obj;
		} else {
			return ((BSONDecimal)get(field)).toBigDecimal();
		}
	}

	/**
	 * Returns the BigDecimal object or def if not set.
	 * 
	 * @param field
	 *            The field to return
	 * @param def
	 *            the default value in case the field is not found
	 * @return The field object value or def if not set.
	 */
	public BigDecimal getBigDecimal(final String field, final BigDecimal def) {
		final Object foo = get(field);
		return (foo != null) ? (BigDecimal) foo : def;
	}
	
	/**
	 * Add a key/value pair to this object
	 * 
	 * @param key
	 *            the field name
	 * @param val
	 *            the field value
	 * @return the <code>val</code> parameter
	 */
	// @Override
	public Object put(String key, Object val) {
		return _objectMap.put(key, val);
	}

	// @Override
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public void putAll(Map m) {
		for (Map.Entry entry : (Set<Map.Entry>) m.entrySet()) {
			put(entry.getKey().toString(), entry.getValue());
		}
	}

	// @Override
	public void putAll(BSONObject o) {
		for (String k : o.keySet()) {
			put(k, o.get(k));
		}
	}

	/**
	 * Add a key/value pair to this object
	 * 
	 * @param key
	 *            the field name
	 * @param val
	 *            the field value
	 * @return <code>this</code>
	 */
	public BasicBSONObject append(String key, Object val) {
		put(key, val);

		return this;
	}

	/**
	 * Returns a JSON serialization of this object
	 * 
	 * @return JSON serialization
	 */
	// @Override
	public String toString() {
		return JSON.serialize(this);
	}

	// @Override
	public boolean equals(Object o) {
		if (!(o instanceof BSONObject))
			return false;

		BSONObject other = (BSONObject) o;
		if (!keySet().equals(other.keySet()))
			return false;

		for (String key : keySet()) {
			Object a = get(key);
			Object b = other.get(key);

			if (a == null) {
				if (b != null)
					return false;
			}
			if (b == null) {
				if (a != null)
					return false;
			} else if (a instanceof Number && b instanceof Number) {
				if (((Number) a).doubleValue() != ((Number) b).doubleValue())
					return false;
			} else if (a instanceof Pattern && b instanceof Pattern) {
				Pattern p1 = (Pattern) a;
				Pattern p2 = (Pattern) b;
				if (!p1.pattern().equals(p2.pattern())
						|| p1.flags() != p2.flags())
					return false;
			} else {
				if (!a.equals(b))
					return false;
			}
		}
		return true;
	}

	@SuppressWarnings({ "rawtypes" })
	public boolean BasicTypeWrite(Object object, Object value, Method method)
			throws IllegalArgumentException, IllegalAccessException,
			InvocationTargetException {
		// Get type of write method's first parameter.
		Class<?> paramType = method.getParameterTypes()[0];
		boolean result = true;
		boolean numberCompare = false;
		if (paramType.isPrimitive()) {
			// if (!(field instanceof Number) && !(field instanceof Character))
			// {
			// throw new IllegalArgumentException(
			// "The method: "
			// + method.getName()
			// +
			// " Expected parameter type:Number does not match with the actual type:"
			// + field.getClass().getName());
			// }

			if (paramType.getName().equals("int")) {
				method.invoke(object, ((Number) value).intValue());
			} else if (paramType.getName().equals("long")) {
				method.invoke(object, ((Number) value).longValue());
			} else if (paramType.getName().equals("byte")) {
				method.invoke(object, ((Number) value).byteValue());
			} else if (paramType.getName().equals("double")) {
				method.invoke(object, ((Number) value).doubleValue());
			} else if (paramType.getName().equals("float")) {
				method.invoke(object, ((Number) value).floatValue());
			} else if (paramType.getName().equals("short")) {
				method.invoke(object, ((Number) value).shortValue());
			} else if (paramType.getName().equals("char")) {
				method.invoke(object, ((Character) value).charValue());
			} else if (paramType.getName().equals("boolean")) {
				method.invoke(object, ((Boolean) value).booleanValue());// TODO
			} else {
				result = false;
			}

			return result;
		}

		// make sure paramType and field are both number
		if ((paramType.getName().equals("java.lang.Integer")
				|| paramType.getName().equals("java.lang.Long")
				|| paramType.getName().equals("java.lang.Float") || paramType
				.getName().equals("java.lang.Double"))
				&& (value.getClass().getName().equals("java.lang.Integer")
						|| value.getClass().getName().equals("java.lang.Long")
						|| value.getClass().getName().equals("java.lang.Float") || value
						.getClass().getName().equals("java.lang.Double"))) {
			numberCompare = true;
		}
		// for number compare, we always cast to Number then cast back
		if (!numberCompare && !paramType.isInstance(value)) {
			throw new IllegalArgumentException("The method: "
					+ method.getName() + " Expected parameter type:"
					+ paramType.getName()
					+ " does not match with the actual type:"
					+ value.getClass().getName());
		}

		result = true;
		if (String.class.isAssignableFrom(paramType)) {
			method.invoke(object, (String) value);
		} else if (Date.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Date) value);
		} else if (Integer.class.isAssignableFrom(paramType)) {
			method.invoke(object, new Integer(((Number) value).intValue()));
		} else if (Long.class.isAssignableFrom(paramType)) {
			method.invoke(object, new Long(((Number) value).longValue()));
		} else if (Double.class.isAssignableFrom(paramType)) {
			method.invoke(object, new Double(((Number) value).doubleValue()));
		} else if (Float.class.isAssignableFrom(paramType)) {
			method.invoke(object, new Float(((Number) value).floatValue()));
		} else if (Character.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Character) value);
		} else if (ObjectId.class.isAssignableFrom(paramType)) {
			method.invoke(object, (ObjectId) value);
		} else if (Boolean.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Boolean) value);
		} else if (Pattern.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Pattern) value);
			// } else if (Map.class.isAssignableFrom(paramType)) {
			// method.invoke(object, (Map) field);
			// } else if (paramType.isAssignableFrom(Iterable.class)) {
			// method.invoke(object, (Iterable) field);
		} else if (byte[].class.isAssignableFrom(paramType)) {
			method.invoke(object, (byte[]) value);
		} else if (Binary.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Binary) value);
		} else if (UUID.class.isAssignableFrom(paramType)) {
			method.invoke(object, (UUID) value);
			// } else if (paramType.getClass().isArray()) { // TODO
		} else if (Symbol.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Symbol) value);
		} else if (BSONTimestamp.class.isAssignableFrom(paramType)) {
			method.invoke(object, (BSONTimestamp) value);
		} else if (BSONDecimal.class.isAssignableFrom(paramType)) {
			method.invoke(object, (BSONDecimal) value);
		} else if (BigDecimal.class.isAssignableFrom(paramType)) {
			method.invoke(object, (BigDecimal)value);
		} else if (CodeWScope.class.isAssignableFrom(paramType)) {
			method.invoke(object, (CodeWScope) value);
		} else if (Code.class.isAssignableFrom(paramType)) {
			method.invoke(object, (Code) value);
		} else if (MinKey.class.isAssignableFrom(paramType))
			method.invoke(object, (MinKey) value);
		else if (MaxKey.class.isAssignableFrom(paramType)) {
			method.invoke(object, (MaxKey) value);
		} else if (List.class.isAssignableFrom(paramType)) {
			method.invoke(object, (List)value);
		} else {
			result = false;
		}
		return result;
	}

	/**
	 * Returns an instance of the class "cls" only for BasicBsonObject
	 * 
	 * @param cls target class object
	 * @return the instance of the class
	 * @throws Exception
	 */
	// @Override
	public <T> T as(Class<T> cls) throws Exception {
		return as(cls, null);
	}

	@SuppressWarnings({ "unchecked" })
	// @Override
	public <T> T as(Class<T> cls, Type eleType) throws Exception {
		boolean hasConsturctor = false;
		T result = null;
		for (Constructor<?> con : cls.getConstructors()) {
			if (con.getParameterTypes().length == 0) {
				result = (T) con.newInstance();
				hasConsturctor = true;
				break;
			}
		}
		if (hasConsturctor == false) {
			throw new Exception("Class " + cls.getName()
					+ " does not exist an default constructor method");
		}

		if (BSON.IsBasicType(result)) {
			throw new IllegalArgumentException(
					"Not support as to basic type. type=" + cls.getName());
		} else if (Collection.class.isAssignableFrom(cls)
				|| Map.class.isAssignableFrom(cls) || cls.isArray()) {
			throw new IllegalArgumentException(
					"Not support as to Collection/Map/Array type. type="
							+ cls.getName());
		} else {
			BeanInfo bi = Introspector.getBeanInfo(cls);
			PropertyDescriptor[] props = bi.getPropertyDescriptors();

			Object value = null;
			for (PropertyDescriptor p : props) {
				if (this.containsField(p.getName())) {
					Method writeMethod = p.getWriteMethod();

					if (writeMethod == null) {
						throw new IllegalArgumentException("The property:"
								+ cls.getName() + "." + p.getName()
								+ " have no set method.");
					}

					value = this.get(p.getName());

					if (value == null) {
						continue;
					} else if (p.getPropertyType().equals(java.util.Map.class)) { // TODO
						// p is Map
						Field mapField = cls.getDeclaredField(p.getName());
						Type generictype = mapField.getGenericType();
						Type valueType = null;
						if (generictype instanceof ParameterizedType) {
							Type[] types = ((ParameterizedType) generictype)
									.getActualTypeArguments();
							valueType = types[1];
						}
						// change bson object to map
						Map map = ((BSONObject) value).toMap();
						Map realMap = new HashMap();
						Set<Map.Entry<?, ?>> set = map.entrySet();
						Iterator<Map.Entry<?, ?>> iterator = set.iterator();
						while (iterator.hasNext()) {
							Map.Entry<?, ?> entry = iterator.next();
							String key = entry.getKey().toString();
							if (((Class) valueType)
									.equals(java.lang.Object.class)) {
								Object v = entry.getValue();
								if (BSON.IsBasicType(v)) {
									realMap.put(key, v);
								} else if (v instanceof BasicBSONList) {
									realMap.put(key,
											((BasicBSONList) v).asList());
								} else if (v instanceof BasicBSONObject) {
									realMap.put(key,
											((BasicBSONObject) v).asMap());
								} else {
									throw new IllegalArgumentException(
											"can't support in map. value_type="
													+ v.getClass());
								}
							} else {
								if (((Class) valueType).isPrimitive()
										|| ((Class) valueType)
												.equals(java.lang.String.class)) {
									realMap.put(key,
											((BSONObject) value).get(key));
								} else {
									Object tmpObj = ((BSONObject) value)
											.get(key);
									if (BSON.IsBasicType(tmpObj)) {
										realMap.put(key, tmpObj);
									} else {
										realMap.put(key, ((BSONObject) tmpObj)
												.as((Class) valueType));
									}
								}
							}
						}
						writeMethod.invoke(result, realMap);
					} else if (value instanceof BasicBSONObject) { // bson <=>
																	// Object
						writeMethod.invoke(result,
								((BSONObject) value).as(p.getPropertyType()));
					} else if (value instanceof BasicBSONList) { // bsonlist <=>
																	// Collection

						Field f = cls.getDeclaredField(p.getName());
						if (f == null)
							continue;
						Type _type = f.getGenericType();

						Type fileType = null;
						if (_type != null && _type instanceof ParameterizedType) {
							fileType = ((ParameterizedType) _type)
									.getActualTypeArguments()[0];
						} else {
							throw new IllegalArgumentException(
									"Current version only support parameterized type Collection(List/Set/Queue) field. unknow type="
											+ _type.toString());
						}

						writeMethod.invoke(result, ((BSONObject) value).as(
								p.getPropertyType(), fileType));
					} else if (BasicTypeWrite(result, value, writeMethod)) {
						continue;
					} else {
						continue;
					}
				}
			}
		}
		return result;
	}

	public Object asMap() {
		Map<String, Object> realMap = new HashMap<String, Object>();
		for (String key : this.keySet()) {
			Object v = this.get(key);
			if (v == null) {
				continue;
			} else if (BSON.IsBasicType(v)) {
				realMap.put(key, v);
			} else if (v instanceof BasicBSONList) {
				realMap.put(key, ((BasicBSONList) v).asList());
			} else if (v instanceof BasicBSONObject) {
				realMap.put(key, ((BasicBSONObject) v).asMap());
			} else {
				throw new IllegalArgumentException(
						"can't support in map. value_type=" + v.getClass());
			}
		}

		return realMap;
	}

	public static BSONObject typeToBson(Object object, Boolean ignoreNullValue)
			throws IntrospectionException, IllegalArgumentException,
			IllegalAccessException, InvocationTargetException {
		BSONObject result = null;
		if (object == null) {
			result = null;
		} else if (BSON.IsBasicType(object)) {
			throw new IllegalArgumentException(
					"Current version is not support basice type to bson in the top level.");
		} else if (object instanceof List) {
			BSONObject listObj = new BasicBSONList();
			List list = (List) object;
			int index = 0;
			for (Object obj : list) {
				if (BSON.IsBasicType(obj)) {
					if (!ignoreNullValue || null != obj) {
						listObj.put(Integer.toString(index), obj);
					}
				} else {
					BSONObject tmpObj = typeToBson(obj, ignoreNullValue);
					if (!ignoreNullValue || null != tmpObj) {
						listObj.put(Integer.toString(index), tmpObj);
					}
				}
				++index;
			}
			result = listObj;
		} else if (object instanceof Map) {
			BSONObject mapObj = new BasicBSONObject();
			Map map = (Map) object;
			Set<Map.Entry<?, ?>> set = map.entrySet();
			Iterator<Map.Entry<?, ?>> iterator = set.iterator();
			while (iterator.hasNext()) {
				Map.Entry<?, ?> entry = iterator.next();
				String key = entry.getKey().toString();
				Object value = entry.getValue();
				if (BSON.IsBasicType(value)) {
					if (!ignoreNullValue || null != value) {
						mapObj.put(key, value);
					}
				} else {
					BSONObject tmpObj = typeToBson(value, ignoreNullValue);
					if (!ignoreNullValue || null != value) {
						mapObj.put(key, tmpObj);
					}
				}
			}
			result = mapObj;
		} else if (object.getClass().isArray()) {
			throw new IllegalArgumentException(
					"Current version is not support Map/Array type field.");
		} else if (object instanceof BSONObject) {
			result = (BSONObject) object;
		} else if (object.getClass().getName() == "java.lang.Class") {
			throw new IllegalArgumentException(
					"Current version is not support java.lang.Class type field.");
		} else { // User define type.
			result = new BasicBSONObject();
			Class<?> cl = object.getClass();

			BeanInfo bi = Introspector.getBeanInfo(cl);
			PropertyDescriptor[] props = bi.getPropertyDescriptors();
			for (PropertyDescriptor p : props) {
				Class<?> type = p.getPropertyType();
				Object propObj = p.getReadMethod().invoke(object);
				if (BSON.IsBasicType(propObj)) {
					if (!ignoreNullValue || null != propObj) {
						result.put(p.getName(), propObj);
					}
				} else if (type.getName() == "java.lang.Class") {
					continue;
				} else {
					BSONObject tmpObj = typeToBson(propObj, ignoreNullValue);
					if (!ignoreNullValue || null != tmpObj) {
						result.put(p.getName(), tmpObj);
					}
				}
			}
		}

		return result;
	}

	@SuppressWarnings({ "rawtypes" })
	public static BSONObject typeToBson(Object object)
			throws IntrospectionException, IllegalArgumentException,
			IllegalAccessException, InvocationTargetException {

		return typeToBson(object, false);
	}

	@Override
	public Set<String> keySet() {
		return _objectMap.keySet();
	}

	public Set<Entry<String, Object>> entrySet() {
		return _objectMap.entrySet();
	}

	@Override
	public int size() {
		return _objectMap.size();
	}

	@Override
	public boolean containsKey(Object key) {
		return _objectMap.containsKey(key);
	}

	@Override
	public boolean containsValue(Object value) {
		return _objectMap.containsValue(value);
	}

	@Override
	public Object remove(Object key) {
		return _objectMap.remove(key);
	}

	@Override
	public void clear() {
		_objectMap.clear();
	}

	@Override
	public Collection<Object> values() {
		return _objectMap.values();
	}

	@Override
	public Object get(Object key) {
		return _objectMap.get(key);
	}

}
