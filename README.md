SequoiaDB README<br />
<br />
<br />
Welcome to SequoiaDB!<br />
<br />
<br />
SequoiaDB is a distributed document-oriented NoSQL Database.<br />
<br />
<br />
Engine:<br />
<span style="white-space:pre">	</span>sequoiadb &nbsp; &nbsp; &nbsp; -- SequoiaDB Engine<br />
<span style="white-space:pre">	</span>sdbstart &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB Engine start<br />
<span style="white-space:pre">	</span>sdbstop &nbsp; &nbsp; &nbsp; &nbsp; -- SequoiaDB Engine stop<br />
<span style="white-space:pre">	</span>sdblist &nbsp; &nbsp; &nbsp; &nbsp; -- SequoiaDB Engine list<br />
<span style="white-space:pre">	</span>sdbfmp &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB fenced mode process<br />
<br />
<br />
Shell:<br />
<span style="white-space:pre">	</span>sdb &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; -- SequoiaDB client<br />
<span style="white-space:pre">	</span>sdbbp &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; -- SequoiaDB client backend process<br />
<br />
<br />
Cluster Manager:<br />
<span style="white-space:pre">	</span>sdbcm &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; -- SequoiaDB cluster manager<br />
<span style="white-space:pre">	</span>sdbcmart &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB cluster manager start<br />
<span style="white-space:pre">	</span>sdbcmtop &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB cluster manager stop<br />
<span style="white-space:pre">	</span>sdbcmd &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB cluster manager daemon<br />
<br />
<br />
Tools:<br />
<span style="white-space:pre">	</span>sdbdpsdump &nbsp; &nbsp; &nbsp;-- SequoiaDB log dump<br />
<span style="white-space:pre">	</span>sdbexprt &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB export<br />
<span style="white-space:pre">	</span>sdbimprt &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB import<br />
<span style="white-space:pre">	</span>sdbinspt &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB data inspection<br />
<span style="white-space:pre">	</span>sdbrestore &nbsp; &nbsp; &nbsp;-- SequoiaDB restore<br />
<span style="white-space:pre">	</span>sdbtop &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB TOP<br />
<span style="white-space:pre">	</span>sdbperfcol &nbsp; &nbsp; &nbsp;-- SequoiaDB performance collection<br />
<span style="white-space:pre">	</span>sdbwsart &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB web service start<br />
<span style="white-space:pre">	</span>sdbwstop &nbsp; &nbsp; &nbsp; &nbsp;-- SequoiaDB web service stop<br />
<br />
<br />
Drivers:<br />
<span style="white-space:pre">	</span>C Driver:<br />
<span style="white-space:pre">		</span>libsdbc.a<br />
<span style="white-space:pre">		</span>libsdbc.so<br />
<span style="white-space:pre">	</span>C++ Driver:<br />
<span style="white-space:pre">		</span>libsdbcpp.a<br />
<span style="white-space:pre">		</span>libsdbcpp.so<br />
<span style="white-space:pre">	</span>PHP Driver:<br />
<span style="white-space:pre">		</span>libsdbphp-x.x.x.so<br />
<span style="white-space:pre">	</span>JAVA Driver:<br />
<span style="white-space:pre">		</span>sequoiadb.jar<br />
<span style="white-space:pre">	</span>PYTHON Driver:<br />
<span style="white-space:pre">		</span>lib.linux-x86_64-2.6<br />
<span style="white-space:pre">	</span>.NET Driver:<br />
<span style="white-space:pre">		</span>sequoiadb.dll<br />
<span style="white-space:pre">	</span>Python Driver:<br />
<span style="white-space:pre">		</span>pysequoiadb.tar.gz<br />
<br />
<br />
Connectors:<br />
<span style="white-space:pre">	</span>Hadoop Connector:<br />
<span style="white-space:pre">		</span>hadoop-connector.jar<br />
<span style="white-space:pre">	</span>Hive Connector:<br />
<span style="white-space:pre">		</span>hive-sequoiadb-apache.jar<br />
<span style="white-space:pre">	</span>Storm Connector:<br />
<span style="white-space:pre">		</span>storm-sequoiadb.jar<br />
<br />
<br />
Building Prerequisites:<br />
<span style="white-space:pre">	</span>scons ( 2.3.0 )<br />
<span style="white-space:pre">	</span>ant ( 1.8.2 )<br />
&nbsp; &nbsp; &nbsp; &nbsp; Python ( 2.7.3 )<br />
<span style="white-space:pre">	</span>Linux x86-64:<br />
<span style="white-space:pre">		</span>g++ ( 4.3.4 )<br />
<span style="white-space:pre">		</span>gcc ( 4.3.4 )<br />
<span style="white-space:pre">		</span>make ( 3.81 )<br />
<span style="white-space:pre">		</span>kernel ( 3.0.13-0.27-default )<br />
&nbsp; &nbsp; &nbsp; &nbsp; Linux PPC64:<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; g++ ( 4.3.4 )<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; gcc ( 4.3.4 )<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; make ( 3.81 )<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; kernel ( 3.0.13-0.27-ppc64 )<br />
<span style="white-space:pre">	</span>Windows:<br />
<span style="white-space:pre">		</span>Windows SDK 7.1 ( Installation path must be C:\Program Files\Microsoft SDKs\Windows\v7.1 or C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1 )<br />
<span style="white-space:pre">	</span>Note:<br />
<span style="white-space:pre">		</span>The utility version is for recommendation only. Users may use different version of utilities but may need to deal with any incompatibility issues.<br />
<span style="white-space:pre">		</span>The kernel version is for recommendation only. Users may use different version of linux kernel but may need to deal with any incompatibility issues.<br />
<br />
<br />
Building Engine:<br />
<span style="white-space:pre">	</span>Engine Only:<br />
<span style="white-space:pre">		</span>scons --engine<br />
<span style="white-space:pre">	</span>C/C++ Client:<br />
<span style="white-space:pre">		</span>scons --client<br />
<span style="white-space:pre">	</span>Shell:<br />
<span style="white-space:pre">		</span>scons --shell<br />
<span style="white-space:pre">	</span>Tools:<br />
<span style="white-space:pre">		</span>scons --tool<br />
<span style="white-space:pre">	</span>Testcase:<br />
<span style="white-space:pre">		</span>scons --testcase<br />
<span style="white-space:pre">	</span>FMP:<br />
<span style="white-space:pre">		</span>scons --fmp<br />
<span style="white-space:pre">	</span>All ( except drivers ):<br />
<span style="white-space:pre">		</span>scons --all<br />
<span style="white-space:pre">	</span>Note:<br />
<span style="white-space:pre">		</span>adding option &quot;--dd&quot; for debug build<br />
<br />
<br />
Building Drivers:<br />
<span style="white-space:pre">	</span>C/C++ Client:<br />
<span style="white-space:pre">		</span>scons --client<br />
<span style="white-space:pre">	</span>PHP Client:<br />
<span style="white-space:pre">		</span>cd driver/php5<br />
<span style="white-space:pre">		</span>scons --phpversion=5.3.10<br />
<span style="white-space:pre">		</span>Note:<br />
<span style="white-space:pre">			</span>PHP source code is located in thirdparty/php directory<br />
<span style="white-space:pre">			</span>The dir name must be &quot;php-&lt;version&gt;&quot;<br />
<span style="white-space:pre">	</span>Python Client:<br />
<span style="white-space:pre">		</span>cd driver/python<br />
<span style="white-space:pre">		</span>scons<br />
<span style="white-space:pre">	</span>Java Client:<br />
<span style="white-space:pre">		</span>cd driver/java<br />
<span style="white-space:pre">		</span>scons<br />
<span style="white-space:pre">	</span>.Net Client:<br />
<span style="white-space:pre">		</span>cd driver/C#.Net<br />
<span style="white-space:pre">		</span>scons<br />
<br />
<br />
Building Connectors:<br />
<span style="white-space:pre">	</span>Hadoop Connector:<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; cd driver/java<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; scons<br />
<span style="white-space:pre">		</span>cd driver/hadoop/hadoop-connector<br />
<span style="white-space:pre">		</span>ant -Dhadoop.version=2.2<br />
<span style="white-space:pre">	</span>Hive Connector:<br />
<span style="white-space:pre">		</span>cd driver/java<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; scons<br />
<span style="white-space:pre">		</span>cd driver/hadoop/hive<br />
<span style="white-space:pre">		</span>ant<br />
<span style="white-space:pre">	</span>Storm Connector:<br />
<span style="white-space:pre">		</span>cd driver/storm<br />
<span style="white-space:pre">		</span>ant<br />
<span style="white-space:pre">	</span>PostgreSQL FDW:<br />
<span style="white-space:pre">		</span>cd driver/postgresql<br />
<span style="white-space:pre">		</span>make local<br />
<span style="white-space:pre">		</span># Make sure pg_config is in PATH<br />
<span style="white-space:pre">		</span>make install<br />
<br />
<br />
Running:<br />
<span style="white-space:pre">	</span>For command line options to start SequoiaDB, invoke:<br />
<span style="white-space:pre">		</span>$ ./sdbstart --help<br />
<span style="white-space:pre">	</span>For command line options to stop SequoiaDB, invoke:<br />
<span style="white-space:pre">		</span>$ ./sdbstop --help<br />
<span style="white-space:pre">	</span>For command line options to start cluster manager, invoke:<br />
<span style="white-space:pre">		</span>$ ./sdbcmart --help<br />
<span style="white-space:pre">	</span>For command line options to stop cluster manager, invoke:<br />
<span style="white-space:pre">		</span>$ ./sdbcmtop --help<br />
<br />
<br />
<span style="white-space:pre">	</span>To run in standalone mode:<br />
<span style="white-space:pre">		</span>$ mkdir /sequoiadb/data<br />
<span style="white-space:pre">		</span>$ /opt/sequoiadb/bin/sdbstart<br />
<span style="white-space:pre">		</span>$ # sequoiadb start successful<br />
<span style="white-space:pre">		</span>$ # start sequoiadb shell<br />
<span style="white-space:pre">		</span>$ /opt/sequoiadb/bin/sdb<br />
<span style="white-space:pre">		</span>&gt; var db = new Sdb() ;<br />
<span style="white-space:pre">		</span>&gt; db.help() ;<br />
<br />
<br />
<span style="white-space:pre">	</span>To run in cluster mode, please refer SequoiaDB Infomation Center.<br />
<br />
<br />
Documentation:<br />
<span style="white-space:pre">	</span>http://www.sequoiadb.com/<br />
<br />
<br />
Restrictions:<br />
<span style="white-space:pre">	</span>- SequoiaDB officially only support 64 bit Linux build on CentOS, Redhat, SUSE and Ubuntu.<br />
<span style="white-space:pre">	</span>- Windows build and 32 bit build are for testing purpose only.<br />
<br />
<br />
License:<br />
<span style="white-space:pre">	</span>Most SequoiaDB source files are made available under the terms of the<br />
<span style="white-space:pre">	</span>GNU Affero General Public License (AGPL). See individual files for details.