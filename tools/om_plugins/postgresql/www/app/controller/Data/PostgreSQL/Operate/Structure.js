//@ sourceURL=Structure.js
(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.PostgreSQL.Structure.Ctrl', function( $scope, $location, $compile, SdbFunction, SdbRest, SdbSwap, SdbSignal ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      SdbSwap.dbName = SdbFunction.LocalData( 'PgsqlDbName' ) ;
      SdbSwap.tbName = SdbFunction.LocalData( 'PgsqlTbName' ) ;
      if( clusterName == null || moduleType != 'sequoiapostgresql' || $scope.ModuleName == null || SdbSwap.tbName == null || SdbSwap.dbName == null )
      {
         $location.path( '/Transfer' ).search( { 'r': new Date().getTime() } ) ;
         return;
      }
      $scope.FullName   = SdbSwap.dbName + '.' + SdbSwap.tbName ;
      SdbSwap.tbType    = SdbFunction.LocalData( 'PgsqlTbType' ) ;
      $scope.PrimaryKey = '' ;

      //获取字段信息
      $scope.QueryTableStruct = function(){
         var sql = sprintf("SELECT \
                   ordinal_position, \
                   column_name, \
                   data_type, \
                   character_maximum_length, \
                   numeric_precision, \
                   numeric_scale, \
                   is_nullable, \
                   column_default \
                   FROM \
                   information_schema.columns \
                   WHERE \
                   table_name = '?'", SdbSwap.tbName ) ;
         var data = { 'Sql': sql, 'DbName': SdbSwap.dbName } ;
         SdbRest.DataOperationV2( '/sql', data, {
            'success': function( fieldList ){
               SdbSignal.commit( 'setTableData', fieldList ) ;
               if( SdbSwap.tbType == 'table' )
               {
                  SdbSignal.commit( 'setPrimarySelect', fieldList ) ;
                  var sql = sprintf( 'select pg_constraint.conname as pk_name from pg_constraint  inner join pg_class \
 on pg_constraint.conrelid = pg_class.oid where pg_class.relname = \'?\' and pg_constraint.contype=\'p\'', SdbSwap.tbName ) ;
                  execSql( sql, true ) ;
               }
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  $scope.QueryTableStruct() ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      $scope.QueryTableStruct() ;
      
      //执行sql
      var execSql = function( sql, isQueryPrimary ){
         var data = { 'Sql': sql, 'DbName': SdbSwap.dbName } ;
         SdbRest.DataOperationV2( '/sql', data, {
            'success': function( result ){
               //重新查询字段列表
               if( isQueryPrimary )
               {
                  if( result.length > 0 )
                  {
                     $scope.PrimaryKey = result[0]['pk_name'] ;
                  }
               }
               else
               {
                  $scope.QueryTableStruct() ;
               }
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  execSql( sql ) ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      //添加字段 弹窗
      $scope.AddFieldWindow = {
         'config': {},
         'callback': {}
      }
      var appendOnly = false ;

      //打开 添加字段 弹窗
      $scope.ShowAddField = function(){
         $scope.AddFieldWindow['config'] = {
            'inputList': [
               {
                  "name": "fields",
                  "webName": $scope.pAutoLanguage( '字段' ),
                  "required": true,
                  "type": "list",
                  "child":[
                     [
                        {
                           "name": "name",
                           "webName": $scope.pAutoLanguage( "字段名" ),
                           "placeholder": $scope.pAutoLanguage( "字段名" ),
                           "type": "string",
                           "value": "",
                           "valid": {
                              "min": 1,
                              "max": 63,
                              "regex": "^[a-zA-Z_]+[0-9a-zA-Z_]*$",
                              "regexError": sprintf( $scope.pAutoLanguage( '?由字母和数字或\"_\"组成，并且以字母或\"_\"起头。' ), $scope.pAutoLanguage( '字段名' ) )
                           }
                        },
                        {
                           "name": "type",
                           "type": "select",
                           "value": "integer",
                           "valid": [
                              { "key": 'smallint', "value": "smallint" },
                              { "key": 'integer', "value": "integer" },
                              { "key": 'bigint', "value": "bigint" },
                              { "key": 'decimal', "value": "decimal" },
                              { "key": 'numeric', "value": "numeric" },
                              { "key": 'real', "value": "real" },
                              { "key": 'double precision', "value": "double precision" },
                              { "key": 'serial', "value": "serial" },
                              { "key": 'bigserial', "value": "bigserial" },
                              { "key": 'money', "value": "money" },
                              { "key": 'varchar', "value": "varchar" },
                              { "key": 'character', "value": "character" },
                              { "key": 'char', "value": "char" },
                              { "key": 'text', "value": "text" },
                              { "key": 'timestamp', "value": "timestamp" },
                              { "key": 'interval', "value": "interval" },
                              { "key": 'date', "value": "date" },
                              { "key": 'time', "value": "time" },
                              { "key": 'boolean', "value": "boolean" }
                           ]
                        },
                        {
                           "name": "length",
                           "webName": $scope.pAutoLanguage( "长度" ),
                           "placeholder": $scope.pAutoLanguage( "长度" ),
                           "type": "int",
                           "value": "",
                           "valid": {
                              "min": 0,
                              "empty": true
                           }
                        },
                        {
                           "name": "default",
                           "webName": $scope.pAutoLanguage( "默认值" ),
                           "placeholder": $scope.pAutoLanguage( "默认值" ),
                           "type": "string",
                           "valid": ""
                        },
                        {
                           "name": "null",
                           "webName": $scope.pAutoLanguage( "空" ),
                           "type": "checkbox",
                           "value": true
                        }
                     ]
                  ]
               }
            ]
         } ;
         $scope.AddFieldWindow['callback']['SetOkButton']( $scope.pAutoLanguage('确定'), function(){
            var isClear = $scope.AddFieldWindow['config'].check() ;
            if( isClear )
            {
               var value = $scope.AddFieldWindow['config'].getValue() ;
               var sql = sprintf( 'ALTER TABLE ? ', addQuotes( SdbSwap.tbName ) ) ;
               $.each( value['fields'], function( index, fieldInfo ){
                  var subSql = '' ;
                  if( index > 0 )
                  {
                     subSql += ', ' ;
                  }
                  subSql += 'ADD ' + addQuotes( fieldInfo['name'] ) + ' ' + fieldInfo['type'] ;
                  if( isNaN( fieldInfo['length'] ) == false )
                  {
                     switch( fieldInfo['type'] )
                     {
                     case 'varchar':
                     case 'character':
                     case 'char':
                     case 'decimal':
                     case 'numeric':
                        subSql += '(' + fieldInfo['length'] + ') ' ;
                        break ;
                     default:
                        subSql += ' ' ;
                        break ;
                     }
                  }
                  else
                  {
                     subSql += ' ' ;
                  }
                  if( fieldInfo['null'] == true )
                  {
                     subSql += 'NULL ' ;
                  }
                  else
                  {
                     subSql += 'NOT NULL ' ;
                  }
                  if( typeof( fieldInfo['default'] ) == 'string' )
                  {
                     subSql += 'DEFAULT ' + sqlEscape( fieldInfo['default'] ) + ' ' ;
                  }
                  sql += subSql ;
               } ) ;
               execSql( sql ) ;
               $scope.AddFieldWindow['callback']['Close']() ;
            }
         } ) ;
         $scope.AddFieldWindow['callback']['SetIcon']( '' ) ;
         $scope.AddFieldWindow['callback']['SetTitle']( $scope.pAutoLanguage( '添加字段' ) ) ;
         $scope.AddFieldWindow['callback']['Open']() ;
      } ;

      $scope.FieldsSelect = [] ;

      //设置主键 弹窗
      $scope.SetPrimaryWindow = {
         'config': {},
         'callback': {}
      } ;

      //打开 设置主键 弹窗
      $scope.ShowSetPrimary = function(){
         if( $scope.FieldsSelect.length > 0 )
         {
            $scope.SetPrimaryWindow['config'] = {
               'inputList': [
                  {
                     "name": "fields",
                     "webName": $scope.pAutoLanguage( '字段' ),
                     "required": true,
                     "type": "list",
                     "child":[
                        [
                           {
                              "name": "field",
                              "type": "select",
                              "value": $scope.FieldsSelect[0]['key'],
                              "default": $scope.FieldsSelect[0]['key'],
                              "valid": $scope.FieldsSelect
                           }
                        ]
                     ]
                  }
               ]
            } ;
            $scope.SetPrimaryWindow['callback']['SetOkButton']( $scope.pAutoLanguage('确定'), function(){
               var sql = sprintf( 'ALTER TABLE ? ADD PRIMARY KEY ', addQuotes( SdbSwap.tbName ) ) ;
               var formValue = $scope.SetPrimaryWindow['config'].getValue() ;
               var isFrist = true ;
               var existList = {} ;
               $.each( formValue['fields'], function( index, field ){
                  if( typeof( existList[field['field']] ) == 'undefined' )
                  {
                     existList[field['field']] = true ;
                     if( isFrist )
                     {
                        sql += '(' ;
                        isFrist = false ;
                     }
                     else
                     {
                        if( existList[field['field']] )
                        sql += ',' ;
                     }
                     sql += addQuotes( field['field'] ) ;
                  }
               } ) ;
               sql += ')' ;
               execSql( sql ) ;
               $scope.SetPrimaryWindow['callback']['Close']() ;
            } ) ;
            $scope.SetPrimaryWindow['callback']['SetIcon']( '' ) ;
            $scope.SetPrimaryWindow['callback']['SetTitle']( $scope.pAutoLanguage( '设置主键' ) ) ;
            $scope.SetPrimaryWindow['callback']['Open']() ;
         }
      }

      //移除主键
      $scope.ShowDropPrimary = function(){
         $scope.Components.Confirm.type = 3 ;
         $scope.Components.Confirm.context = sprintf( $scope.pAutoLanguage( '是否确定删除主键：?？' ), $scope.PrimaryKey) ;
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.okText = $scope.pAutoLanguage( '确定' ) ;
         $scope.Components.Confirm.ok = function(){
            var sql = sprintf( 'alter table ? drop constraint ?', addQuotes( SdbSwap.tbName ), addQuotes( $scope.PrimaryKey ) ) ;
            execSql( sql ) ;
            $scope.PrimaryKey = '' ;
            $scope.Components.Confirm.isShow = false ;
         }
      }

      //删除字段 弹窗
      $scope.DropFieldWindow = {
         'config': {},
         'callback': {}
      } ;

      SdbSignal.on( 'setPrimarySelect', function( fieldList ){
         $scope.FieldsSelect = [] ;
         if( fieldList.length > 0 )
         {
            $.each( fieldList, function( index, field ){
               $scope.FieldsSelect.push( { 'key': field['column_name'], 'value': field['column_name'] } ) ;
            } ) ;
         }
      } ) ;

      //打开 删除字段 弹窗
      SdbSignal.on( 'ShowDropFieldWindow', function( fieldName ){
         $scope.Components.Confirm.type = 3 ;
         $scope.Components.Confirm.context = sprintf( $scope.pAutoLanguage( '是否确定删除字段：?？' ), fieldName ) ;
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.okText = $scope.pAutoLanguage( '确定' ) ;
         $scope.Components.Confirm.ok = function(){
            var sql = sprintf( 'alter table ? drop column ?', addQuotes( SdbSwap.tbName ), addQuotes( fieldName ) ) ;
            execSql( sql ) ;
            $scope.Components.Confirm.isShow = false ;
         }
      } ) ;

      //重命名字段 弹窗
      $scope.RenameFieldWindow = {
         'config': {},
         'callback': {}
      } ;
      
      //打开 重命名字段 弹窗
      var showRenameField = function( fieldName ){
         $scope.RenameFieldWindow['config'] = {
            'inputList': [
               {
                  "name": "oldFieldName",
                  "webName": $scope.pAutoLanguage( '原字段名' ),
                  "type": "string",
                  "required": true,
                  "disabled": true,
                  "value": fieldName
               },
               {
                  "name": "newFieldName",
                  "webName": $scope.pAutoLanguage( '新字段名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1,
                     "max": 63,
                     "regex": "^[a-zA-Z_]+[0-9a-zA-Z_]*$",
                     "regexError": sprintf( $scope.pAutoLanguage( '?由字母和数字或\"_\"组成，并且以字母或\"_\"起头。' ), $scope.pAutoLanguage( '字段名' ) )
                  }
               }
            ]
         }
         $scope.RenameFieldWindow['callback']['SetTitle']( $scope.pAutoLanguage( '修改字段名' ) ) ;
         $scope.RenameFieldWindow['callback']['SetIcon']( 'fa-edit' ) ;
         $scope.RenameFieldWindow['callback']['SetOkButton']( $scope.pAutoLanguage( '确定' ), function(){
            var isClear = $scope.RenameFieldWindow['config'].check() ;
            if( isClear == true )
            {
               var formVal = $scope.RenameFieldWindow['config'].getValue() ;
               var sql = sprintf( 'alter table ? rename column ? to ?', addQuotes( SdbSwap.tbName ), addQuotes( fieldName ), addQuotes( formVal['newFieldName'] ) ) ;
               execSql( sql ) ;
               $scope.RenameFieldWindow['callback']['Close']() ;
            }
         } ) ;
         $scope.RenameFieldWindow['callback']['Open']() ;
      }

      //设置字段默认值 弹窗
      $scope.SetfFieldDefaultWindow = {
         'config': {},
         'callback': {}
      } ;

      //打开 设置字段默认值 弹窗
      var showSetDefault = function( fieldName ){
         $scope.SetfFieldDefaultWindow['config'] = {
            'inputList': [
               {
                  "name": "fieldName",
                  "webName": $scope.pAutoLanguage( '字段名' ),
                  "type": "string",
                  "required": true,
                  "disabled": true,
                  "value": fieldName
               },
               {
                  "name": "default",
                  "webName": $scope.pAutoLanguage( '默认值' ),
                  "type": "string",
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               }
            ]
         }
         $scope.SetfFieldDefaultWindow['callback']['SetTitle']( $scope.pAutoLanguage( '设置默认值' ) ) ;
         $scope.SetfFieldDefaultWindow['callback']['SetIcon']( 'fa-edit' ) ;
         $scope.SetfFieldDefaultWindow['callback']['SetOkButton']( $scope.pAutoLanguage( '确定' ), function(){
            var isClear = $scope.SetfFieldDefaultWindow['config'].check() ;
            if( isClear == true )
            {
               var formVal = $scope.SetfFieldDefaultWindow['config'].getValue() ;
               var sql = sprintf( 'alter table ? alter column ? set default ?', addQuotes( SdbSwap.tbName ), addQuotes( fieldName ), sqlEscape( formVal['default'] ) ) ;
               execSql( sql ) ;
               $scope.SetfFieldDefaultWindow['callback']['Close']() ;
            }
         } ) ;
         $scope.SetfFieldDefaultWindow['callback']['Open']() ;
      }

      //打开 删除字段默认值 弹窗
      var shwoRemoveDefault = function( fieldName ){
         $scope.Components.Confirm.type = 3 ;
         $scope.Components.Confirm.context = sprintf( $scope.pAutoLanguage( '是否确定删除字段 ? 的默认值？' ), fieldName ) ;
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.okText = $scope.pAutoLanguage( '确定' ) ;
         $scope.Components.Confirm.ok = function(){
            var sql = sprintf( 'alter table ? alter column ? drop default', addQuotes( SdbSwap.tbName ), addQuotes( fieldName ) ) ;
            execSql( sql ) ;
            $scope.Components.Confirm.isShow = false ;
         }
      }

      //修改字段类型 弹窗
      $scope.SetfFieldTypetWindow = {
         'config': {},
         'callback': {}
      } ;

      //打开 修改字段类型 弹窗
      var showSetFieldType = function( fieldName, type ){
         $scope.SetfFieldTypetWindow['config'] = {
            'inputList': [
               {
                  "name": "fieldName",
                  "webName": $scope.pAutoLanguage( '字段名' ),
                  "type": "string",
                  "required": true,
                  "disabled": true,
                  "value": fieldName
               },
               {
                  "name": "oldType",
                  "webName": $scope.pAutoLanguage( '原字段类型' ),
                  "type": "string",
                  "disabled": true,
                  "value": type
               },
               {
                  "name": "newType",
                  "webName": $scope.pAutoLanguage( '新字段类型' ),
                  "type": "select",
                  "value": "integer",
                  "valid": [
                     { "key": 'smallint', "value": "smallint" },
                     { "key": 'integer', "value": "integer" },
                     { "key": 'bigint', "value": "bigint" },
                     { "key": 'decimal', "value": "decimal" },
                     { "key": 'numeric', "value": "numeric" },
                     { "key": 'real', "value": "real" },
                     { "key": 'double precision', "value": "double precision" },
                     { "key": 'serial', "value": "serial" },
                     { "key": 'bigserial', "value": "bigserial" },
                     { "key": 'money', "value": "money" },
                     { "key": 'varchar', "value": "varchar" },
                     { "key": 'character', "value": "character" },
                     { "key": 'char', "value": "char" },
                     { "key": 'text', "value": "text" },
                     { "key": 'timestamp', "value": "timestamp" },
                     { "key": 'interval', "value": "interval" },
                     { "key": 'date', "value": "date" },
                     { "key": 'time', "value": "time" },
                     { "key": 'boolean', "value": "boolean" }
                  ]
               }
            ]
         }
         $scope.SetfFieldTypetWindow['callback']['SetTitle']( $scope.pAutoLanguage( '设置默认值' ) ) ;
         $scope.SetfFieldTypetWindow['callback']['SetIcon']( 'fa-edit' ) ;
         $scope.SetfFieldTypetWindow['callback']['SetOkButton']( $scope.pAutoLanguage( '确定' ), function(){
            var isClear = $scope.SetfFieldTypetWindow['config'].check() ;
            if( isClear == true )
            {
               var formVal = $scope.SetfFieldTypetWindow['config'].getValue() ;
               var sql = sprintf( 'alter table ? alter column ? type ?', addQuotes( SdbSwap.tbName ), addQuotes( fieldName ), formVal['newType'] ) ;
               execSql( sql ) ;
               $scope.SetfFieldTypetWindow['callback']['Close']() ;
            }
         } ) ;
         $scope.SetfFieldTypetWindow['callback']['Open']() ;
      }

      //编辑字段 下拉菜单
      $scope.EditFieldDropdown = {
         'config': [
            { 'key': $scope.pAutoLanguage( '修改字段名' ) },
            { 'key': $scope.pAutoLanguage( '修改字段类型' ) },
            { 'key': $scope.pAutoLanguage( '设置默认值' ) },
            { 'key': $scope.pAutoLanguage( '删除默认值' ) }
         ],
         'callback': {}
      } ;

      //打开 编辑字段 下拉菜单
      SdbSignal.on( 'ShowEditFieldDropdown', function( result ){
         $scope.EditFieldDropdown['OnClick'] = function( index ){
            if( index == 0 )
            {
               showRenameField( result['field'] ) ;
            }
            else if( index == 1 )
            {
               showSetFieldType( result['field'], result['type'] ) ;
            }
            else if( index == 2 )
            {
               showSetDefault( result['field'] ) ;
            }
            else
            {
               shwoRemoveDefault( result['field'] ) ;
            }
            $scope.EditFieldDropdown['callback']['Close']() ;
         }
         $scope.EditFieldDropdown['callback']['Open']( result['event'].currentTarget ) ;
      } ) ;

   } ) ;

   //表格 控制器
   sacApp.controllerProvider.register( 'Data.PostgreSQL.Structure.Table.Ctrl', function( $scope, SdbSwap, SdbSignal ){
      //表格
      $scope.GridTable = {
         'title': {
            'index' : '#',
            'column_name'      : $scope.pAutoLanguage( '字段名' ),
            'operation'        : '',
            'data_type'        : $scope.pAutoLanguage( '类型' ),
            'column_default'   : $scope.pAutoLanguage( '默认值' ),
            'is_nullable'      : $scope.pAutoLanguage( '空' )
         },
         'body': [],
         'options': {
            'width': {
               'index' : '35px',
               'column_name'      : '25%',
               'operation'        : '60px',
               'data_type'        : '25%',
               'column_default'   : '25%',
               'is_nullable'      : '25%'
            },
            'sort': {
               'index' : true,
               'column_name'      : true,
               'operation'        : false,
               'data_type'        : true,
               'column_default'   : true,
               'is_nullable'      : true
            },
            'max': 50
         }
      } ;


      SdbSignal.on( 'setTableData', function( result ){
         $scope.GridTable['body'] = result ;
      } ) ;

      //打开 编辑字段 下拉菜单
      $scope.ShowEditFieldDropdown = function( event, fieldName, fieldType ){
         SdbSignal.commit( 'ShowEditFieldDropdown', { 'event': event, 'field': fieldName, 'type': fieldType } ) ;
      }

      //打开 删除字段 弹窗
      $scope.ShowDropFieldWindow = function( fieldName ){
         SdbSignal.commit( 'ShowDropFieldWindow', fieldName ) ;
      }

   } ) ;

   
}());