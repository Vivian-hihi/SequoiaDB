package com.sequoiadb.hive;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.JobConf;
import org.bson.BSONObject;
import org.bson.types.BasicBSONList;


import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.exception.BaseException;

public class SdbSplit extends FileSplit implements InputSplit {

	public static final Log LOG = LogFactory.getLog(SdbSerDe.class.getName());

	private static final String[] EMPTY_ARRAY = new String[] {};
	private SdbConnAddr sdbAddr;
	private String  scanType;
	private Integer dataBlockId;

	public SdbSplit() {
		super((Path) null, 0, 0, EMPTY_ARRAY);

	}

	public SdbSplit(String host, int port, String scanType, int dataBlockID,
			Path dummyPath) {
		super(dummyPath, 0, 0, EMPTY_ARRAY);
		this.sdbAddr = new SdbConnAddr(host, port);
		this.scanType = scanType;
		this.dataBlockId = dataBlockID;
	}


	@Override
	public void readFields(final DataInput input) throws IOException {
		super.readFields(input);
		this.sdbAddr = new SdbConnAddr();
		sdbAddr.setHost(input.readUTF());
		sdbAddr.setPort(input.readInt());
		
		scanType = input.readUTF();
		dataBlockId = input.readInt();
		
	}

	@Override
	public void write(final DataOutput output) throws IOException {
		super.write(output);
		output.writeUTF(this.sdbAddr.getHost());
		output.writeInt(this.sdbAddr.getPort());

		output.writeUTF(this.scanType);
		output.writeInt(this.dataBlockId);
	}

	public SdbConnAddr getSdbAddr() {
		return this.sdbAddr;
	}

	public String getScanType() {
		return this.scanType;
	}
	
	public Integer getDataBlockId() {
		return this.dataBlockId;
	}

	@Override
	public long getLength() {
		return 128 * 1024 * 1024;
	}

	/* Data is remote for all nodes. */
	@Override
	public String[] getLocations() throws IOException {
		return new String[] {sdbAddr.getHost()};
	}

	@Override
	public String toString() {

		return String.format("SdbSplit(sdbaddr=%s, block id=%d)",
				sdbAddr == null ? "null" : sdbAddr.toString(),
				dataBlockId);
	}

	public static InputSplit[] getSplits(JobConf conf, int numSplits) {

		LOG.debug("Entry getSplits function, with numSplites=" + numSplits);

		SdbConnAddr[] sdbAddrList = ConfigurationUtil
				.getAddrList(ConfigurationUtil.getDBAddr(conf));

		if (sdbAddrList == null || sdbAddrList.length == 0) {
			throw new IllegalArgumentException("The argument "
					+ ConfigurationUtil.DB_ADDR + " must be set.");
		}

		final Path[] tablePaths = FileInputFormat.getInputPaths(conf);

		Sequoiadb sdb = null;
		BaseException lastException = null;
		for (int i = 0; i < sdbAddrList.length; i++) {
			try {
				sdb = new Sequoiadb(sdbAddrList[i].getHost(),
						sdbAddrList[i].getPort(), null, null);
				break;
			} catch (BaseException e) {
				lastException = e;
				continue;
			}
		}
		if (sdb == null) {
			throw lastException;
		}

		LOG.info("Start Get data blocks");
		String spaceName = null;
		String colName = null;
		if( ConfigurationUtil.getCsName(conf) == null && ConfigurationUtil.getClName(conf) == null ){
			spaceName = ConfigurationUtil.getSpaceName(conf);
			colName = ConfigurationUtil.getCollectionName(conf);
		}else{
			spaceName = ConfigurationUtil.getCsName(conf);
			colName = ConfigurationUtil.getClName(conf);
		}
		
		DBCollection collection = sdb.getCollectionSpace(spaceName)
				.getCollection(colName);
		DBCursor cursor = collection.getQueryMeta(null, null, null, 0,
				-1, 0);
		
		List<InputSplit> splits = new LinkedList<InputSplit>();
		
		while (cursor.hasNext()) {
			BSONObject obj = cursor.getNext();

			LOG.info("mete record:" + obj.toString());
			String hostname = (String) obj.get("HostName");
			int port = Integer.parseInt((String) obj
					.get("ServiceName"));

			String scanType = (String) obj.get("ScanType");

			if (scanType.equals("ixscan")) {
				String indexName = (String) obj.get("IndexName");
				int indexLID = (Integer) obj.get("IndexLID");
				int direction = (Integer) obj.get("Direction");

				BasicBSONList indexBlockList = (BasicBSONList) obj
						.get("Indexblocks");
				for (Object objBlock : indexBlockList) {
					if (objBlock instanceof BSONObject) {
						BSONObject indexBlock = (BSONObject) objBlock;

						BSONObject startKey = (BSONObject) indexBlock
								.get("StartKey");
						BSONObject endKey = (BSONObject) indexBlock
								.get("EndKey");
					}
				}
			} else if (scanType.equals("tbscan")) {

				BasicBSONList blockList = (BasicBSONList) obj.get("Datablocks");
				int i = 0;
				for (Object objBlock : blockList) {
					if (objBlock instanceof Integer) {
						Integer blockId = (Integer) objBlock;
						splits.add(new SdbSplit(hostname, port, scanType, blockId,
								tablePaths[0]));
					}
				}
			}
		}

		LOG.info("Exit SdbScanNode::getScanRangeLocations");
		sdb.disconnect();

		return splits.toArray(new InputSplit[splits.size()]);
	}
}
