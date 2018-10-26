/******************************************************************************
@Description :   seqDB-15993: 创建集合时，创建16个自增字段 
@Modify list :   2018-10-17    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   } 
    
   var clName = COMMCLNAME + "_15993";
   
   commDropCL(db, COMMCSNAME, clName);
   
   var autoIncrements = getAutoIncrements();
   
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, { AutoIncrement : autoIncrements });
   
   //check autoIncrement and sequence
   var sequenceNames = getSequenceNames(COMMCSNAME, clName, autoIncrements); 
   var expIncrements = getExpIncrements(sequenceNames, autoIncrements);
   checkAutoIncrementonCL(COMMCSNAME, clName, expIncrements);
   
   for(var i in sequenceNames)
   {
      checkSequence(sequenceNames[i], {}); 
   }
   
   commDropCL(db, COMMCSNAME, clName);
}

function getAutoIncrements()
{
   var autoIncrements = new Array();
   for(var i = 0; i < 16; i++)
   {
      autoIncrements.push({ Field : "id" + i });
   }
   return autoIncrements;
}

function getSequenceNames(csName, clName, autoIncrements)
{
   var sequenceNames = new Array();
   var clID = getCLID(csName, clName);
   for(var i in autoIncrements)
   {
      sequenceNames.push( "SYS_" + clID + "_" + autoIncrements[i].Field + "_SEQ" );   
   }
   return sequenceNames;   
}

function getExpIncrements(sequenceNames, autoIncrements)
{  
   var expIncrements = new Array();
   for(var i in autoIncrements)
   {
      expIncrements.push({ Field : autoIncrements[i].Field, SequenceName : sequenceNames[i] });   
   }
   return expIncrements;
}

main();