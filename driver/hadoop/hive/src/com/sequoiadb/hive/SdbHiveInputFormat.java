package com.sequoiadb.hive;

import static com.sequoiadb.hive.ConfigurationUtil.getCollectionName;
import static com.sequoiadb.hive.ConfigurationUtil.getSpaceName;

import java.io.IOException;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.hadoop.hive.ql.exec.Utilities;
import org.apache.hadoop.hive.ql.io.HiveInputFormat;
import org.apache.hadoop.hive.ql.plan.ExprNodeDesc;
import org.apache.hadoop.hive.ql.plan.TableScanDesc;
import org.apache.hadoop.hive.serde2.ColumnProjectionUtils;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.RecordReader;
import org.apache.hadoop.mapred.Reporter;


public class SdbHiveInputFormat extends
		HiveInputFormat<LongWritable, BytesWritable> {

	@Override
	public RecordReader<LongWritable, BytesWritable> getRecordReader(InputSplit inputSplit, JobConf jobConf,
			Reporter Reporter) throws IOException {
		
		List<Integer> readColIDs = ColumnProjectionUtils.getReadColumnIDs(jobConf);

		String columnString = jobConf.get(ConfigurationUtil.COLUMN_MAPPING);
		if (StringUtils.isBlank(columnString)) {
			throw new IOException("no column mapping found!");
		}

		String[] columns = ConfigurationUtil.getAllColumns(columnString);
		if (readColIDs.size() > columns.length) {
			throw new IOException(
					"read column count larger than that in column mapping string!");
		}

		
		String filterExprSerialized = jobConf
				.get(TableScanDesc.FILTER_EXPR_CONF_STR);
		String filterTextSerialized = jobConf
				.get(TableScanDesc.FILTER_TEXT_CONF_STR);

		ExprNodeDesc filterExpr = null;
		if (filterTextSerialized != null) {
			
			filterTextSerialized = filterTextSerialized.replaceAll("\'", "\"");
			
			LOG.debug(TableScanDesc.FILTER_TEXT_CONF_STR + "=" + filterTextSerialized);
			System.out.println(TableScanDesc.FILTER_TEXT_CONF_STR + "=" + filterTextSerialized);
			
			filterExpr = Utilities.deserializeExpression(
					filterExprSerialized, jobConf);
			
		}
		
		return new SdbReader(getSpaceName(jobConf), getCollectionName(jobConf), inputSplit,	columns, readColIDs, filterExpr);
	}

	@Override
	public InputSplit[] getSplits(JobConf jobConf, int numSplits) throws IOException {
		return SdbSplit.getSplits(jobConf,numSplits);
	}

}
