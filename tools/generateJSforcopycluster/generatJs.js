function generatePublicVar(file)
{
   if (undefined === file)
   {
      throw "invalid file";
   }

   file.write("if(undefined === coordHostName)\n");
   file.write("{\n");
   file.write("   var coordHostName = 'localhost';\n");
   file.write("}\n\n");
   
   file.write("if(undefined === coordPort)\n");
   file.write("{\n");
   file.write("   var coordPort = 11810;\n");
   file.write("}\n\n");
   
   file.write("if(undefined === needDeploy)\n");
   file.write("{\n");
   file.write("   var needDeploy = false;\n");
   file.write("}\n\n");

   file.write("if(undefined === needCreateDomain)\n");
   file.write("{\n");
   file.write("   var needCreateDomain = true;\n");
   file.write("}\n\n");
}

//获取组定义
function getGroupDefine(db, jsFile)
{
   try
   {
      var cursor = db.list(7);
      jsFile.write("var groups = [");
      var firsttime = true;
      while (cursor.next())
      {
         var obj = cursor.current().toObj();
         var group = {}
         group.name = obj.GroupName;
         group.nodes = [];

         for (var i = 0; i < obj.Group.length; ++i)
         {
            var node = {};
            node.HostName = obj.Group[i].HostName;
            node.dbpath = obj.Group[i].dbpath;
            node.serviceName = obj.Group[i].Service[0].Name;
            group.nodes.push(node);
         }

         if (firsttime)
         {
            jsFile.write(JSON.stringify(group) + ",");
            firsttime = false;
         }
         else
         {
            jsFile.write("\n");
            jsFile.write("              " + JSON.stringify(group) + ",");
            
         }
      }

      jsFile.write("];");
      jsFile.write("\n\n");
      println("getGroupDefine success");

   }
   catch(e)
   {
      throw "getGroupDefine " + e;
   }
}

//获取domain定义
function getDomainDefine(db, jsFile)
{
   try
   {
      var cursor = db.listDomains();
      
      jsFile.write("var domains = [")
      var firsttime = true;
      while (cursor.next())
      {
         var obj = cursor.current().toObj();
         var domain = {};
         domain.name = obj.Name;
         domain.groups = [];
         for (var i = 0; i < obj.Groups.length; ++i)
         {
            domain.groups.push(obj.Groups[i].GroupName);
         }
         if (true === obj.AutoSplit)
         {
            domain.option = {};
            domain.option.AutoSplit = obj.AutoSplit; 
         }
         if (true === firsttime)
         {
            jsFile.write(JSON.stringify(domain) + ",");
            firsttime = false;
         }
         else
         {
            jsFile.write("\n");
            jsFile.write("               " + JSON.stringify(domain) + ",");
         }
      }
      
      jsFile.write("];\n\n");
      println("getDomainDefine success");
   }
   catch(e)
   {
      throw "getDomainDefine " + e;
   }
}

// 获取CS定义
function getCSDefine(db, jsFile)
{
   try
   {
      var cursor = db.SYSCAT.SYSCOLLECTIONSPACES.find();
      jsFile.write("var csSet = [");
      var firsttime = true;
      while(cursor.next())
      {
         var cs = {};
         var obj = cursor.current().toObj();
         
         cs.name = obj.Name;
         cs.option = {};
         if (undefined !== obj.Domain)
         {
            cs.option.Domain = obj.Domain;
         }
         
         cs.option.LobPageSize = obj.LobPageSize;
         cs.option.PageSize = obj.PageSize
         
         if (true === firsttime)
         {
            jsFile.write(JSON.stringify(cs) + ",");  
            firsttime = false;
         }
         else
         {
            jsFile.write("\n");
            jsFile.write("             "  + JSON.stringify(cs) + ",");
         }
      }
      
      jsFile.write("];\n\n");
      println("getCSDefine success");
   }
   catch(e)
   {
      throw "getCSDefine " + e;
   }
}

// 获取CL定义
function getCLDefine(db, jsFile)
{
   try
   {
      var cursor = db.snapshot(8);
      jsFile.write("var clSet = [");
      var firsttime = true;
      while (cursor.next())
      {
         var cl = {};   
         var obj = cursor.current().toObj();
         
         cl.name = obj.Name;
         cl.option = {};
         if (undefined !== obj.ShardingType)
         {
            cl.option.ShardingType = obj.ShardingType;
            cl.option.ShardingKey = obj.ShardingKey;
         }
         
         if (undefined !== obj.EnsureShardingIndex)
         {
            cl.option.EnsureShardingIndex = obj.EnsureShardingIndex
         }
         
         if (undefined === obj.AutoSplit && 
             1 === obj.CataInfo.length)
         {
            cl.option.Group = obj.CataInfo[0].GroupName;  
         }
         
         if (undefined !== obj.IsMainCL)
         {
            cl.option.IsMainCL = true;
            
            cl.subcls = [];
            for (var i = 0; i< obj.CataInfo.length; ++i)
            {
               var subcl = {};
               subcl.name = obj.CataInfo[i].SubCLName;
               subcl.attchOpt = {}
               
               for (key in cl.option.ShardingKey)
               {
                  subcl.attchOpt.LowBound = {};
                  subcl.attchOpt.LowBound[key] = obj.CataInfo[i].LowBound[""];
                  subcl.attchOpt.UpBound = {};
                  subcl.attchOpt.UpBound[key] = obj.CataInfo[i].UpBound[""];
               }
               cl.subcls.push(subcl);
            }
         }
         
         if (undefined !== obj.Partition)
         {
            cl.option.Partition = obj.Partition;
         }
         
         if (undefined !== obj.AutoIndexId)
         {
            cl.option.Partition = obj.AutoIndexId;
         }
         
         if (undefined !== obj.Attribute)
         {
            cl.option.Compressed = true;
            
            if (undefined !== obj.CompressionType &&
                1 === obj.CompressionType)
            {
               cl.option.CompressionType = "lzw";
            }
         }
         
         if (undefined !== obj.ReplSize)
         {
            cl.option.ReplSize = obj.ReplSize;
         }
         
         if (true === firsttime)
         {
            jsFile.write(JSON.stringify(cl) + ",");
            firsttime = false;
         }
         else
         {
            jsFile.write("\n");
            jsFile.write("             " + JSON.stringify(cl) + ",");
         }
      }
      
      jsFile.write("];\n\n");
      println("getCLDefine success");
   }
   catch(e)
   {
      throw "getCLDefined" + e;
   }
}

// 获取索引定义
function getIndexDefine(db, jsFile)
{
   try
   {
      var clSet = db.list(4).toArray();
      jsFile.write("var indexDefs = [");
      var firsttime = true;
      for (var i = 0; i < clSet.length; ++i)
      {
         var obj = eval("(" + clSet[i] + ")");
         var fullname = obj.Name;
         var names = fullname.split(".");
         if (2 === names.length )
         {
            var cl = db.getCS(names[0]).getCL(names[1]);
            var cursor = cl.listIndexes();
            var clIndexs = {}
            clIndexs.name = fullname;
            clIndexs.indexs = [];
            while (cursor.next())
            {
               var index = {};
               var obj = cursor.current().toObj();
               if ("$id" !== obj.IndexDef.name && 
                   "$shard" !== obj.IndexDef.name)
               {
                  index.name = obj.IndexDef.name;
                  index.key = obj.IndexDef.key;
                  index.unique = obj.IndexDef.unique;
                  index.enforced = obj.IndexDef.enforced;
                  clIndexs.indexs.push(index);
               }
            }
            
            if (true === firsttime && 
                0 !== clIndexs.indexs.length )
            {
               jsFile.write(JSON.stringify(clIndexs) + ",");
               firsttime = false;     
            }
            else if(0 !== clIndexs.indexs.length)
            {
               jsFile.write("\n");
               jsFile.write("                " + JSON.stringify(clIndexs) + ",");
            }
         }
      }
      
      jsFile.write("];\n\n");
      println("getIndexDefine success");
   }
   catch(e)
   {
      throw "getIndexDefine " + e;
   }
}

// 生成部署函数
function generatDeployFunction(file)
{
var functionContext = "\n\
function deployCluster(db, groups) \n\
{\n\
   for (var i = 0; i < groups.length; ++i) \n\
   {\n\
      if ('SYSCatalogGroup' === groups[i].name)\n\
      {\n\
         for (var k = 0; k < groups[i].nodes.length; ++k)\n\
         {\n\
            var node = groups[i].nodes[k];\n\
            if (0 === k)\n\
            {\n\
               var catarg = db.createCataRG(node.HostName, node.serviceName, node.dbpath);\n\
            }\n\
            else\n\
            {\n\
               catarg.createNode(node.HostName, node.serviceName, node.dbpath);\n\
            }\n\
         }\n\
         catarg.start();\n\
      }\n\
   }\n\
   \n\
   for (var i = 0; i < groups.length; ++i)\n\
   {\n\
      if ('SYSCatalogGroup' === groups[i].name)\n\
      {\n\
         continue;\n\
      }\n\
      \n\
      if ('SYSCoord' === groups[i].name)\n\
      {\n\
         var rg = db.createCoordRG();\n\
      }\n\
      else\n\
      {\n\
         var rg = db.createRG(groups[i].name);\n\
      }\n\
      \n\
      for (var k = 0; k < groups[i].nodes.length; ++k)\n\
      {\n\
         var node = groups[i].nodes[k];\n\
         rg.createNode(node.HostName, node.serviceName, node.dbpath);\n\
      }\n\
      rg.start();  \n\
   }\n\
}"

   file.write(functionContext + "\n");
   println("generatDeployFunction success");
}

// 生成CS创建函数
function generatCreateCSFunction(file)
{
var functionContext = "\n\
function createAllCS(db, csSet)\n\
{\n\
   for (var i = 0; i < csSet.length; ++i)\n\
   {\n\
      db.createCS(csSet[i].name, csSet[i].option);\n\
      println('createCS(' + csSet[i].name + ',' + JSON.stringify(csSet[i].option) + ') success');\n\
   }\n\
}";
   file.write(functionContext + "\n");
   println("generatCreateCSFunction success");
}

// 生成CL创建函数
function generatCreateCLFunction(file)
{
   var functionContext = "\n\
function createAllCL(db, clSet)\n\
{\n\
   var mainCLSet  = [];\n\
   for (var i = 0; i < clSet.length; ++i)\n\
   {\n\
      var names = clSet[i].name.split('.');\n\
      db.getCS(names[0]).createCL(names[1], clSet[i].option); \n\
      println('createCL(' + names[1] + ',' + JSON.stringify(clSet[i].option) + ') success');\n\
      \n\
      if (undefined !== clSet[i].option.IsMainCL && \n\
          true === clSet[i].option.IsMainCL)\n\
      {\n\
         mainCLSet.push(clSet[i]);\n\
      }\n\
   }\n\
   \n\
   for (var i = 0; i<mainCLSet.length; ++i)\n\
   {\n\
      name = mainCLSet[i].name.split('.');\n\
      for (var j = 0; j < mainCLSet[i].subcls.length; ++j)\n\
      {\n\
         var item = mainCLSet[i].subcls[j];\n\
         db.getCS(name[0]).getCL(name[1]).attachCL(item.name, item.attchOpt);\n\
         println('db.' + mainCLSet[i].name + '.attachCL(' + item.name + ',' + JSON.stringify(item.attchOpt) + ') success');\n\
      }\n\
   }\n\
}"
   file.write(functionContext + "\n");
   println("generatCreateCLFunction success");
}

// 生成main函数
function generatmainFunction(file)
{
   var functionContext = "\n\
function main()\n\
{\n\
   try\n\
   {\n\
      var db = new Sdb(coordHostName, coordPort);\n\
      \n\
      if (true === needDeploy)\n\
      {\n\
         deployCluster(db, groups);\n\
      }\n\
      \n\
      if (true === needCreateDomain)\n\
      {\n\
         createAllDomains(db, domains);\n\
      }\n\
      createAllCS(db,csSet);\n\
      createAllCL(db, clSet);\n\
      createAllIndexes(db,indexDefs);\n\
   }\n\
   catch(e)\n\
   {\n\
      throw e;\n\
   }\n\
   finally\n\
   {\n\
      if (undefined !== db)\n\
      {\n\
         db.close();\n\
      }\n\
   }\n\
}\n\
println('Usage:')\n\
println('      -e \\\'var coordHostName = \\\"localhost\\\"; var coordPort=11810;var needDeploy=false; var needCreateDomain=true\\\'');\n\
println('you can modify parameters!!!');\n\
println('use default parameters continue...');\n\
main();"
   file.write(functionContext + "\n");
}


// 生成创建domain的函数
function generatCreateDomainFunction(file)
{
   var functionContext = "\n\
function createAllDomains(db, domains)\n\
{\n\
   for (var i = 0; i < domains.length; ++i)\n\
   {\n\
      db.createDomain(domains[i].name, domains[i].groups, domains[i].option);\n\
      println('createDomain ' + domains[i].name + ' success');\n\
   }\n\
}";
   file.write(functionContext + "\n");
   println("generatCreateDomainFunction success");
}

// 生成创建索引的函数
function generatCreateIdxFunction(file)
{
  var functionContext = "\n\
function createAllIndexes(db, indexes)\n\
{\n\
  for (var i = 0; i < indexes.length; ++i)\n\
  {\n\
     var names=indexes[i].name.split('.');\n\
     var cl = db.getCS(names[0]).getCL(names[1]);\n\
     for (var j = 0; j < indexes[i].indexs.length; ++j)\n\
     {\n\
        var item = indexes[i].indexs[j];\n\
        cl.createIndex(item.name, item.key, item.unique, item.enforced);\n\
        println(cl.toString() + 'createIndex(' + item.name + ',' + JSON.stringify(item.key) + ',' + item.unique + ',' + item.enforced + ') success');\n\
     }\n\
  }\n\
}"
  file.write(functionContext + "\n");
  println("generatCreateIdxFunction success");
}


function newDB(hostname, port)
{
   try
   {
      var db = new Sdb(hostname, port);
   }
   catch(e)
   {
      throw "new Sdb(" + hostname + "," + port +") ErrCode: " + e;
   }
   return db;
}

function createFile(filePath)
{
   try
   {
      var file = new File(filePath);
   }
   catch(e)
   {
      throw "new File(" + filePath + ") ErrCode: " + e;
   }
   return file;
}

if ( undefined === coordHostName)
{
   var coordHostName = "localhost";
}

if (undefined === coordPort)
{
   var coordPort = 11810;
}

if ( undefined === catalogHostName)
{
   var catalogHostName = "localhost";
}

if (undefined === catalogPort)
{
   var catalogPort = 11820;
}

if (undefined === generateFilePath)
{
   var generateFilePath = "./copyCluster.js";
}

function main()
{
   try
   {
      var db = newDB(coordHostName, coordPort);
      var catadb = newDB(catalogHostName, catalogPort);
      var file = createFile(generateFilePath);

      getGroupDefine(db,file);
      getDomainDefine(db,file);
      getCSDefine(catadb, file);
      getCLDefine(db, file);
      getIndexDefine(db, file);
      generatePublicVar(file);

      generatDeployFunction(file);
      generatCreateDomainFunction(file);
      generatCreateCSFunction(file);
      generatCreateCLFunction(file);
      generatCreateIdxFunction(file);
      generatmainFunction(file);
   }
   catch(e)
   {
      throw e;
   }
   finally
   {
      if (undefined !== file)
      {
         file.close();
      }
      
      if (undefined !== db)
      {
         db.close();
      }
   }
}

println("Usage:" );
println("     -e 'var coordHostName= \"localhost\"; var coordPort=11810; var catalogHostName=\"localhost\" ; var catalogPort=11820 ; var generateFilePath=./copyCluster.js '");
println("you can modify parameters!!!");
println("use default parameters continue...");
main();
