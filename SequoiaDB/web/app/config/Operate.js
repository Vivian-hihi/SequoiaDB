(function(){
   //创建集合空间的配置
   window.SdbSacManagerConf.CreateCSConf = { id: 'createCS', title: '', input: [], action: 'CreateCSConf' } ;
   //删除集合空间的配置
   window.SdbSacManagerConf.RemoveCSConf = { id: 'removeCS', title: '', input: [], action: 'RemoveCSConf' } ;
   //创建集合的配置
   window.SdbSacManagerConf.CreateCLConf = { id: 'createCL', title: '', input: [], action: 'CreateCLConf', relyon: function( key, values ){
      var isShow = true ;
      switch( key )
      {
      case 'ShardingKey':
         if( values['ShardingType'] == 'normal' )
         {
            isShow = false ;
         }
         break ;
      case 'Partition':
         if( values['ShardingType'] == 'normal' )
         {
            isShow = false ;
         }
         else if( values['ShardingType'] == 'range' )
         {
            isShow = false ;
         }
         else if( values['ShardingType'] == 'maincl' )
         {
            isShow = false ;
         }
         break ;
      case 'AutoSplit':
         if( values['ShardingType'] == 'normal' )
         {
            isShow = false ;
         }
         else if( values['ShardingType'] == 'range' )
         {
            isShow = false ;
         }
         else if( values['ShardingType'] == 'maincl' )
         {
            isShow = false ;
         }
         break ;
      case 'Compressed':
      case 'Group':
         if( values['ShardingType'] == 'maincl' )
         {
            isShow = false ;
         }
         break ;
      }
      return isShow ;
   } } ;
   //删除集合的配置
   window.SdbSacManagerConf.RemoveCLConf = { id: 'removeCL', title: '', input: [], action: 'RemoveCLConf' } ;
   //创建索引的配置
   window.SdbSacManagerConf.CreateIndexConf = { id: 'createIndex', title: '', input: [], action: 'CreateIndexConf', relyon: function( key, values ){
      var isShow = true ;
      switch( key )
      {
      case 'enforced':
         if( values['isUnique'] == 'false' )
         {
            isShow = false ;
         }
         break ;
      }
      return isShow ;
   } } ;
   //选择cluster
   window.SdbSacManagerConf.SelectCluster = { id: 'selcetCluster', icon:'', ok: 'true', title: '', input: [] } ;
}());