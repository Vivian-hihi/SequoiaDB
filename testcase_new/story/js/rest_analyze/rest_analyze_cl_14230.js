/****************************************************
@description:     test analyze cl
@testlink cases:  seqDB-14230
@modify list:
2018-07-30        linsuqiang init
****************************************************/

var analyzeClName = "analyzeCl14230";
var nonAnalyzeClName = "nonAnalyzeCl14230";
var cs = commCreateCS( db, COMMCSNAME, /*ignoreExisted*/true );
var analyzeCl = cs.createCL( analyzeClName );
insertDataWithIndex( analyzeCl );
var nonAnalyzeCl = cs.createCL( nonAnalyzeClName );
insertDataWithIndex( nonAnalyzeCl );

checkScanTypeByExplain( analyzeCl, "ixscan" ) ;
checkScanTypeByExplain( nonAnalyzeCl, "ixscan" ) ;
var clFullName = COMMCSNAME + "." + analyzeClName;
tryCatch( ["cmd=analyze", "options={Collection: \"" + clFullName + "\"}"], 
          [0], 
          "fail to analyze." );
checkScanTypeByExplain( analyzeCl, "tbscan" ) ;
checkScanTypeByExplain( nonAnalyzeCl, "ixscan" ) ;

cs.dropCL( analyzeClName );
cs.dropCL( nonAnalyzeClName );
