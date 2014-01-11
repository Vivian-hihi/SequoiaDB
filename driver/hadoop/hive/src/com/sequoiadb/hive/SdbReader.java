package com.sequoiadb.hive;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hbase.io.ImmutableBytesWritable;
import org.apache.hadoop.hive.ql.plan.ExprNodeColumnDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeConstantDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeGenericFuncDesc;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.RecordReader;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;

class ByteArrayField {
	private byte[] array = null;
	private int startPos = 0;
	private int endPos = 0;

	public ByteArrayField(byte[] array, int startPos, int endPos) {
		this.array = array;
		this.startPos = startPos;
		this.endPos = endPos;
	}

	public int copyFiledtoArray(byte[] destArray, int pos) {
		int length = endPos - startPos;
		for (int i = 0; i < length; i++) {
			destArray[pos + i] = array[this.startPos + i];
		}
		return length;
	}

	public String toString() {
		String str = new String(array, startPos, endPos - startPos);
		return str;
	}
}

//public class SdbReader implements RecordReader<LongWritable, BytesWritable> {
public class SdbReader implements RecordReader<LongWritable, BytesWritable> {
	public static final Log LOG = LogFactory.getLog(SdbReader.class.getName());
	private Sequoiadb sdb = null;
	private DBCursor cursor = null;
	private long pos = 0;
	List<Integer> readColIDs;
	private String[] columnsMap;
	private int[] selectorColIDs;
	private SdbSplit sdbSplit = null;

	private int recordsLenth = 0;

	private static final Map<String, String> COMP_BSON_TABLE = new HashMap<String, String>();
	private static final Map<String, String> LOGIC_BSON_TABLE = new HashMap<String, String>();
	static {
		COMP_BSON_TABLE.put(
				"org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqual",
				"$et");
		COMP_BSON_TABLE.put(
				"org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPLessThan",
				"$lt");
		COMP_BSON_TABLE
				.put("org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqualOrLessThan",
						"$lte");
		COMP_BSON_TABLE
				.put("org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPGreaterThan",
						"$gt");
		COMP_BSON_TABLE
				.put("org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqualOrGreaterThan",
						"$gte");

		LOGIC_BSON_TABLE
				.put("org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPAnd",
						"$and");
		LOGIC_BSON_TABLE
				.put("org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNot",
						"$not");
		LOGIC_BSON_TABLE.put(
				"org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPOr", "$or");
	}

	public SdbReader(String spaceName, String colName, InputSplit split,
			String[] columns, List<Integer> readColIDs, ExprNodeDesc filterExpr) {
		
		
		if (split == null || !(split instanceof SdbSplit)) {
			throw new IllegalArgumentException(
					"The split is not SdbSplit type.");
		}
		this.readColIDs = readColIDs;
		this.columnsMap = columns;
		
		//LOG.info("columns is " + columns.toString());
		this.sdbSplit = (SdbSplit) split;

		LOG.debug("The split information:" + split.toString());
		if (sdbSplit.getSdbAddr() == null) {
			throw new IllegalArgumentException(
					"The split.sdbAddr is null. split=" + sdbSplit.toString());
		}

		sdb = new Sequoiadb(sdbSplit.getSdbAddr().getHost(), sdbSplit
				.getSdbAddr().getPort(), null, null);
		CollectionSpace space = sdb.getCollectionSpace(spaceName);
		DBCollection collection = space.getCollection(colName);

		BSONObject query = null;
		if (filterExpr != null) {
			try {
				
				query = parserFilterExprToBSON(filterExpr, 0);
				
				
			} catch (Exception e) {
				//If have any exception, query all record without condition.
				query = null;
			}
		}
		LOG.debug("query:" + query);
		
		
		// BSONObject selector = null;
		BasicBSONObject selector = new BasicBSONObject();
		for (String column : parserReadColumns(columnsMap, readColIDs)) {
			selector.put(column.toLowerCase(), 1);
		}
		LOG.debug("selector:" + selector);
		
		selectorColIDs = new int[selector.size()];
		
		int index = 0;
		for (Entry<String, Object> entry : selector.entrySet()) {
			for (int i = 0; i < this.columnsMap.length; i++) {
				if (columnsMap[i].equalsIgnoreCase(entry.getKey())) {
					LOG.debug("selectorColIDs[" + index + "] = " + i);
					this.selectorColIDs[index++] = i;
					break;
				}
			}
		}

		BSONObject orderBy = null;

		BSONObject hint = new BasicBSONObject();

		BasicBSONList blocksList = new BasicBSONList();
		blocksList.add(sdbSplit.getDataBlockId());

		BSONObject metaObj = new BasicBSONObject();
		metaObj.put("ScanType", sdbSplit.getScanType());
		metaObj.put("Datablocks", blocksList);
		
		hint.put("$Meta", metaObj);

		LOG.debug("hint:" + hint);
		cursor = collection.query(query, selector, orderBy, hint, 1);
	}

	private String[] parserReadColumns(String[] columnsMap,
			List<Integer> readColIDs) {

		String[] readColumns = null;
		// Get read columns list.
		boolean addAll = (readColIDs.size() == 0);
		if (addAll) {
			readColumns = columnsMap;
		} else {
			readColumns = new String[readColIDs.size()];
			for (int i = 0; i < readColumns.length; i++) {
				readColumns[i] = columnsMap[readColIDs.get(i)];
			}
		}
		for(String f : readColumns){
			LOG.info("readColumns is " + f);
		}
		return readColumns;
	}

	protected BSONObject parserFilterExprToBSON(ExprNodeDesc filterExpr,
			int level) throws IOException {
		StringBuffer space = new StringBuffer();
		for (int i = 0; i < level * 3; i++) {
			space.append(" ");
		}
		String prexString = space.toString();

		BSONObject bson = new BasicBSONObject();

		if (filterExpr instanceof ExprNodeGenericFuncDesc) {
			ExprNodeGenericFuncDesc funcDesc = (ExprNodeGenericFuncDesc) filterExpr;

			LOG.debug(prexString + "ExprNodeGenericFuncDesc:"
					+ funcDesc.toString());

			String funcName = funcDesc.getGenericUDF().getClass().getName();

			LOG.debug(prexString + "funcName:" + funcName);
			LOG.info(prexString + "funcName:" + funcName);
			for (Entry<String, String> entry : COMP_BSON_TABLE.entrySet()) {
				LOG.debug(entry.getKey());
				LOG.info(entry.getKey());
			}
			if (COMP_BSON_TABLE.containsKey(funcName)) {

				List<String> columnList = new ArrayList<String>();
				List<Object> constantList = new ArrayList<Object>();

				for (ExprNodeDesc nodeDesc : funcDesc.getChildren()) {
					if (nodeDesc instanceof ExprNodeColumnDesc) {
						ExprNodeColumnDesc columnDesc = (ExprNodeColumnDesc) nodeDesc;
						columnList.add(columnDesc.getColumn());
					} else if (nodeDesc instanceof ExprNodeConstantDesc) {
						ExprNodeConstantDesc constantDesc = (ExprNodeConstantDesc) nodeDesc;
						constantList.add(constantDesc.getValue());
					} else if (nodeDesc instanceof ExprNodeGenericFuncDesc) {
						return null;
					}
				}

				BSONObject compObj = new BasicBSONObject();
				if (constantList.size() == 0 && columnList.size() > 1) {
					BSONObject fieldObj = new BasicBSONObject();
					fieldObj.put("$field", columnList.get(1).toUpperCase());

					compObj.put(COMP_BSON_TABLE.get(funcName), fieldObj);
				} else {
					compObj.put(COMP_BSON_TABLE.get(funcName),
							constantList.get(0));
				}

				bson.put(columnList.get(0).toUpperCase(), compObj);

			} else if (LOGIC_BSON_TABLE.containsKey(funcName)) {

				BasicBSONList bsonList = new BasicBSONList();

				for (ExprNodeDesc chileDesc : funcDesc.getChildren()) {
					BSONObject Child = parserFilterExprToBSON(chileDesc,
							level + 1);
					bsonList.add(Child);
				}
				bson.put(LOGIC_BSON_TABLE.get(funcName), bsonList);
			} else if (funcName
					.equals("org.apache.hadoop.hive.ql.udf.generic.GenericUDFIn")) {

				String column = findColumnNameInChildrenNode(funcDesc
						.getChildren());

				BSONObject compObj = new BasicBSONObject();
				BasicBSONList bsonList = new BasicBSONList();
				for (Object value : findValueInChildrenNode(funcDesc
						.getChildren())) {
					bsonList.add(value);
				}
				compObj.put("$in", bsonList);
				bson.put(column, compObj);
			} else if (funcName.equals("org.apache.hadoop.hive.ql.udf.UDFLike")) {

				String column = findColumnNameInChildrenNode(funcDesc
						.getChildren());

				Object value = findValueInChildrenNode(funcDesc.getChildren())
						.get(0);
				if (value instanceof String) {
					String likeRegx = likePatternToRegExp((String) value);
					Pattern pattern = Pattern.compile(likeRegx,
							Pattern.CASE_INSENSITIVE);
					bson.put(column, pattern);
				} else {
					throw new IOException(
							"The like UDF have not string parame:"
									+ funcDesc.toString());
				}

			} else {
				throw new IOException("The current is not support this UDF:"
						+ funcDesc.toString());
			}
		}
		return bson;
	}

	public static String likePatternToRegExp(String likePattern) {
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < likePattern.length(); i++) {
			// Make a special case for "\\_" and "\\%"
			char n = likePattern.charAt(i);
			if (n == '\\'
					&& i + 1 < likePattern.length()
					&& (likePattern.charAt(i + 1) == '_' || likePattern
							.charAt(i + 1) == '%')) {
				sb.append(likePattern.charAt(i + 1));
				i++;
				continue;
			}

			if (n == '_') {
				sb.append(".");
			} else if (n == '%') {
				sb.append(".*");
			} else {
				sb.append(Pattern.quote(Character.toString(n)));
			}
		}
		return sb.toString();
	}

	protected String findColumnNameInChildrenNode(
			List<ExprNodeDesc> childrenNodeDesc) {
		for (ExprNodeDesc nodeDesc : childrenNodeDesc) {
			if (nodeDesc instanceof ExprNodeColumnDesc) {
				ExprNodeColumnDesc columnDesc = (ExprNodeColumnDesc) nodeDesc;
				return columnDesc.getColumn();
			}
		}
		return null;
	}

	protected List<Object> findValueInChildrenNode(
			List<ExprNodeDesc> childrenNodeDesc) {
		List<Object> constantList = new ArrayList<Object>();
		for (ExprNodeDesc nodeDesc : childrenNodeDesc) {
			if (nodeDesc instanceof ExprNodeConstantDesc) {
				ExprNodeConstantDesc constantDesc = (ExprNodeConstantDesc) nodeDesc;
				constantList.add(constantDesc.getValue());
			}
		}
		return constantList;
	}

	@Override
	public void close() throws IOException {
		if (cursor != null) {
			cursor.close();
		}

		if (sdb != null) {
			sdb.disconnect();
		}
	}

	@Override
	public LongWritable createKey() {
		return new LongWritable();
	}

	@Override
	public BytesWritable createValue() {
		return new BytesWritable();
	}

	@Override
	public long getPos() throws IOException {
		return this.pos;
	}

	@Override
	public float getProgress() throws IOException {
		return sdbSplit.getLength() > 0 ? this.recordsLenth
				/ (float) sdbSplit.getLength() * 64 * 1024 : 1.0f;
	}
	
	
	
	@Override
	public boolean next(LongWritable keyHolder, BytesWritable valueHolder)
			throws IOException {
		
		// fetch record from cursor
		if (!cursor.hasNextRaw()) {
			
			return false;
		}
		// text always start from byte 10 here
		final int TEXT_START_POS = 10;
		// get the byte array from result buffer, each fields are seaprated by bar
		// what happen if there's bar in the text? we don't handle and let it break~~~
		byte[] record = cursor.getNextRaw();
		// record the length so that we can keep monitoring the progress
		recordsLenth += record.length;

		// let's build array of byte array to hold the start pointer and length
		// for each field from record buffer
		ByteArrayField[] byteArrayRef = new ByteArrayField[this.selectorColIDs.length];
		
		// initialize start position and current position
		int startPos = TEXT_START_POS;
		int i = TEXT_START_POS;
		// keep track of number of fields we want
		// sanity check could be done by nFileNum
		int nFileNum = 0;
		// we are going to iterate the receive buffer and push the start pointer + length
		// into ByteArrayField
		for (; i < record.length - 1; i++) {
			if (record[i] == '|') {
				ByteArrayField ref = new ByteArrayField(record, startPos, i);
				byteArrayRef[nFileNum++] = ref;
				startPos = i + 1;
			}
		}
		// must be <=, otherwise for "abc|" will got error for NullPointer exception
		// since the second NULL field was not allocated for new ByteArrayField
		if (startPos <= i) {
			ByteArrayField ref = new ByteArrayField(record, startPos, i);
			byteArrayRef[nFileNum++] = ref;
		}
		// need to add columnsMap.length since we need to create extra "|"
		// it seems like there's always 2 columns extra in columnsMap for
		// BLOCK__OFFSET__INSIDE__FILE
		// INPUT__FILE__NAME
		// just in case there's any other extra fields that we didn't realize, we always
		// add another this.columnsMap.length bytes
		byte[] recordWithAllColumns = new byte[record.length - TEXT_START_POS + 
		                                       this.columnsMap.length];
		// pos records the current position of the result buffer
		int pos = 0;
		// this function receives the definition of the table, and expect to return
		// all columns by the right order
		// so we need to iterate each column from columnsMap, and compare with the
		// actual result that we received from sequoiadb, and copy each fields into
		// bar separated format result buffer
		for (i = 0; i < this.columnsMap.length; i++) {
			// for each columns in the DDL
			for (int j = 0; j < this.selectorColIDs.length; j++) {
				// compare with the result fields from SDB
				if (this.selectorColIDs[j] == i) {
					// if match, let's add into result
					pos += byteArrayRef[j].copyFiledtoArray(
							recordWithAllColumns, pos);
					// break the loop once we find the field from result buffer
					break;
				}
			}
			// we need to append bar between each field
			if(pos != recordWithAllColumns.length ){
				recordWithAllColumns[pos++] = '|';
			}
		}
		//String rcWAC = new String(recordWithAllColumns);
		//LOG.info("byte returned to hive is " + rcWAC );
		// set the valueHolder from the result buffer, starting from 0 until pos
		valueHolder.set(recordWithAllColumns, 0, pos);
		return true;
	}
}
