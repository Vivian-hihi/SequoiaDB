//@ sourceURL=Index.js
(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Config.MySQL.Index.Ctrl', function( $scope, $location, $rootScope, SdbFunction, SdbRest, SdbSignal, SdbSwap, SdbPromise ){

      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;

      if( clusterName == null || moduleType != 'sequoiasql-mysql' || moduleName == null || moduleMode == null )
      {
         $location.path( '/Transfer' ).search( { 'r': new Date().getTime() } ) ;
         return;
      }

      var template = {} ;
      var confFileList = [] ; //可以写入文件的配置项

      $scope.ModuleName = moduleName ;

      $scope.SettingTable = {
         'table': {
            'tools': false,
            'max': 10000,
            'filter': {
               'VARIABLE_NAME': 'indexof',
               'VARIABLE_VALUE': 'indexof'
            }
         },
         'title': {
            'VARIABLE_NAME':  $scope.pAutoLanguage( '配置项' ),
            'VARIABLE_VALUE': $scope.pAutoLanguage( '值' )
         },
         'content': [],
         'callback': {}
      } ;

      //修改多个配置弹窗
      $scope.ConfigsWindow = {
         'config': {
            'keyWidth': '200px',
            'inputList': []
         },
         'callback': {}
      } ;

      //修改单个配置弹窗
      $scope.ConfigWindow = {
         'config': {
            'keyWidth': '200px',
            'inputList': [
               {
                  'name':     'value',
                  'value':    '',
                  'webName':  '',
                  'type':     'string'
               }
            ]
         },
         'callback': {}
      } ;

      $scope.Refresh = function(){
         queryGlobalVar() ;
      }

      function saveServiceConfig( updator )
      {
         var tmp = {} ;
         for( var key in updator )
         {
            if ( confFileList.indexOf( key ) >= 0 || key.indexOf( 'sequoiadb_' ) >= 0 )
            {
               tmp[key] = updator[key] ;
            }
         }

         if ( isEmpty( tmp ) )
         {
            queryGlobalVar() ;
            return ;
         }

         updator = tmp ;

         var configInfo = {
            'ClusterName': clusterName,
            'BusinessName': moduleName,
            'Config': {
               'property': updator
            }
         } ;

         var data = {
            'cmd': 'update business config',
            'ConfigInfo': JSON.stringify( configInfo )
         } ;

         SdbRest.OmOperation( data, {
            'success': function(){
               queryGlobalVar() ;
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  saveServiceConfig( updator ) ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      function updateServiceConfig( updator )
      {
         var sql = 'set global ' ;
         var isFirst = true ;

         for( var key in updator )
         {
            if ( !isFirst )
            {
               sql += ', ' ;
            }
            if ( isNaN( updator[key] ) || ( isString( updator[key] ) && updator[key].length == 0 ) )
            {
               sql += key + "='" + updator[key] + "'" ;
            }
            else
            {
               sql += key + "=" + updator[key] ;
            }
            isFirst = false ;
         }

         var data = { 'Sql': sql, 'DbName': '', 'Type': 'mysql' } ;
         SdbRest.DataOperationV2( '/sql', data, {
            'success': function( result ){
               saveServiceConfig( updator ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  updateServiceConfig( updator ) ;
                  return true ;
               } ) ;
            }
         }, {
            'showLoading': true
         } ) ;
      }

      //批量
      $scope.OpenConfigsWindow = function(){
     
         $scope.ConfigsWindow['callback']['SetTitle']( $scope.pAutoLanguage( '修改配置项' ) ) ;
         $scope.ConfigsWindow['callback']['SetOkButton']( $scope.pAutoLanguage( '确定' ), function(){
            var isAllClear = $scope.ConfigsWindow['config'].check() ;
            if( isAllClear )
            {
               var configs = $scope.ConfigsWindow['config'].getValue() ;
               var newConfigs = {} ;

               for( var key in configs )
               {
                  if( template[key] != configs[key] )
                  {
                     newConfigs[key] = configs[key] ;
                  }
               }

               if( isEmpty( newConfigs ) )
               {
                  queryGlobalVar() ;
               }
               else
               {
                  updateServiceConfig( newConfigs ) ;
               }
            }
            else
            {
               $scope.ConfigsWindow['config'].scrollToError( null ) ;
            }
            return isAllClear ;
         } ) ;
         $scope.ConfigsWindow['callback']['Open']() ;
      }

      //单个
      $scope.OpenConfigWindow = function( key, value ){

         $scope.ConfigWindow['config']['inputList'][0]['webName'] = key ;
         $scope.ConfigWindow['config']['inputList'][0]['name'] = key ;
         $scope.ConfigWindow['config']['inputList'][0]['value'] = value ;
      
         $scope.ConfigWindow['callback']['SetTitle']( $scope.pAutoLanguage( '修改配置项' ) ) ;
         $scope.ConfigWindow['callback']['SetOkButton']( $scope.pAutoLanguage( '确定' ), function(){
            var isAllClear = $scope.ConfigWindow['config'].check() ;
            if( isAllClear )
            {
               var configs = $scope.ConfigWindow['config'].getValue() ;

               updateServiceConfig( configs ) ;
            }
            return isAllClear ;
         } ) ;
         $scope.ConfigWindow['callback']['Open']() ;
      }

      function restartModule()
      {
         var data = {
            'cmd': 'restart business',
            'ClusterName': clusterName,
            'BusinessName': moduleName,
            'Options': {}
         } ;

         SdbRest.OmOperation( data, {
            'success': function( taskInfo ){
               $rootScope.tempData( 'Deploy', 'Model', 'Update Config' ) ;
               $rootScope.tempData( 'Deploy', 'Module', 'sequoiasql-mysql' ) ;
               $rootScope.tempData( 'Deploy', 'ModuleTaskID', taskInfo[0]['TaskID'] ) ;
               $location.path( '/Deploy/Task/Restart' ).search( { 'r': new Date().getTime() } ) ;
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  restartModule() ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      $scope.RestartModule = function(){
         $scope.Components.Confirm.type = 2 ;
         $scope.Components.Confirm.context = $scope.pAutoLanguage( '该操作将重启服务，是否确定继续？' ) ;
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.okText = $scope.pAutoLanguage( '确定' ) ;
         $scope.Components.Confirm.ok = function(){
            restartModule() ;
            $scope.Components.Confirm.isShow = false ;
         }
      }

      function queryGlobalVar()
      {
         var data = { 'Sql': "select * from global_variables", 'DbName': 'performance_schema', 'Type': 'mysql', 'IsAll': 'true' } ;
         SdbRest.DataOperationV2( '/sql', data, {
            'success': function( result ){

               clearObject( template ) ;
               clearArray( $scope.ConfigsWindow['config']['inputList'] ) ;

               $scope.SettingTable['content'] = result ;

               $.each( result, function( index, info ){
                  template[info['VARIABLE_NAME']] = info['VARIABLE_VALUE'] ;

                  $scope.ConfigsWindow['config']['inputList'].push( {
                     'name':     info['VARIABLE_NAME'],
                     'value':    info['VARIABLE_VALUE'],
                     'webName':  info['VARIABLE_NAME'],
                     'type':     'string'
                  } ) ;

               } ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  queryGlobalVar() ;
                  return true ;
               } ) ;
            }
         }, {
            'showLoading': true
         } ) ;
      }

      queryGlobalVar() ;

      confFileList = ["autocommit","avoid_temporal_upgrade","basedir","binlog_cache_size","binlog_direct_non_transactional_updates","binlog_error_action","binlog_group_commit_sync_delay","binlog_group_commit_sync_no_delay_count","binlog_gtid_simple_recovery","binlog_row_image","binlog_rows_query_log_events","binlog_stmt_cache_size","binlog_transaction_dependency_history_size","binlog_transaction_dependency_tracking","block_encryption_mode","bulk_insert_buffer_size","check_proxy_users","completion_type","concurrent_insert","connect_timeout","datadir","default_authentication_plugin","default_password_lifetime","default_tmp_storage_engine","default_week_format","delayed_insert_limit","delayed_insert_timeout","delayed_queue_size","disabled_storage_engines","disconnect_on_expired_password","div_precision_increment","enforce_gtid_consistency","expire_logs_days","explicit_defaults_for_timestamp","flush","flush_time","ft_boolean_syntax","ft_max_word_len","ft_min_word_len","ft_query_expansion_limit","ft_stopword_file","general_log_file","group_concat_max_len","init_connect","init_slave","innodb_adaptive_flushing","innodb_adaptive_flushing_lwm","innodb_adaptive_hash_index","innodb_adaptive_hash_index_parts","innodb_adaptive_max_sleep_delay","innodb_api_bk_commit_interval","innodb_api_disable_rowlock","innodb_api_enable_binlog","innodb_api_enable_mdl","innodb_api_trx_level","innodb_autoextend_increment","innodb_autoinc_lock_mode","innodb_buffer_pool_chunk_size","innodb_buffer_pool_dump_at_shutdown","innodb_buffer_pool_dump_now","innodb_buffer_pool_dump_pct","innodb_buffer_pool_filename","innodb_buffer_pool_instances","innodb_buffer_pool_load_abort","innodb_buffer_pool_load_at_startup","innodb_buffer_pool_load_now","innodb_buffer_pool_size","innodb_change_buffer_max_size","innodb_change_buffering","innodb_checksum_algorithm","innodb_checksums","innodb_cmp_per_index_enabled","innodb_commit_concurrency","innodb_compression_failure_threshold_pct","innodb_compression_level","innodb_compression_pad_pct_max","innodb_concurrency_tickets","innodb_data_file_path","innodb_data_home_dir","innodb_deadlock_detect","innodb_default_row_format","innodb_disable_sort_file_cache","innodb_doublewrite","innodb_fast_shutdown","innodb_file_format","innodb_file_format_check","innodb_file_format_max","innodb_file_per_table","innodb_fill_factor","innodb_flush_log_at_trx_commit","innodb_flush_method","innodb_flush_neighbors","innodb_flush_sync","innodb_flushing_avg_loops","innodb_force_load_corrupted","innodb_force_recovery","innodb_ft_cache_size","innodb_ft_enable_diag_print","innodb_ft_enable_stopword","innodb_ft_max_token_size","innodb_ft_min_token_size","innodb_ft_num_word_optimize","innodb_ft_result_cache_limit","innodb_ft_server_stopword_table","innodb_ft_sort_pll_degree","innodb_ft_total_cache_size","innodb_ft_user_stopword_table","innodb_io_capacity","innodb_io_capacity_max","innodb_large_prefix","innodb_lock_wait_timeout","innodb_locks_unsafe_for_binlog","innodb_log_buffer_size","innodb_log_checksums","innodb_log_compressed_pages","innodb_log_file_size","innodb_log_files_in_group","innodb_log_group_home_dir","innodb_log_write_ahead_size","innodb_lru_scan_depth","innodb_max_dirty_pages_pct","innodb_max_dirty_pages_pct_lwm","innodb_max_purge_lag","innodb_max_purge_lag_delay","innodb_max_undo_log_size","innodb_monitor_disable","innodb_monitor_enable","innodb_monitor_reset","innodb_monitor_reset_all","innodb_old_blocks_pct","innodb_old_blocks_time","innodb_online_alter_log_max_size","innodb_open_files","innodb_optimize_fulltext_only","innodb_page_cleaners","innodb_page_size","innodb_print_all_deadlocks","innodb_purge_batch_size","innodb_purge_rseg_truncate_frequency","innodb_purge_threads","innodb_random_read_ahead","innodb_read_ahead_threshold","innodb_read_io_threads","innodb_read_only","innodb_replication_delay","innodb_rollback_on_timeout","innodb_rollback_segments","innodb_sort_buffer_size","innodb_spin_wait_delay","innodb_stats_auto_recalc","innodb_stats_include_delete_marked","innodb_stats_method","innodb_stats_on_metadata","innodb_stats_persistent","innodb_stats_persistent_sample_pages","innodb_stats_sample_pages","innodb_stats_transient_sample_pages","innodb_status_output","innodb_status_output_locks","innodb_strict_mode","innodb_support_xa","innodb_sync_array_size","innodb_sync_spin_loops","innodb_table_locks","innodb_temp_data_file_path","innodb_thread_concurrency","innodb_thread_sleep_delay","innodb_tmpdir","innodb_undo_directory","innodb_undo_log_truncate","innodb_undo_logs","innodb_undo_tablespaces","innodb_use_native_aio","innodb_write_io_threads","interactive_timeout","internal_tmp_disk_storage_engine","join_buffer_size","keep_files_on_create","key_buffer_size","key_cache_age_threshold","key_cache_block_size","key_cache_division_limit","lock_wait_timeout","log_bin_use_v1_row_events","log_builtin_as_identified_by_password","log_error_verbosity","log_output","log_slave_updates","log_syslog","log_syslog_facility","log_syslog_include_pid","log_syslog_tag","log_timestamps","long_query_time","lower_case_table_names","master_info_repository","max_allowed_packet","max_binlog_cache_size","max_binlog_size","max_binlog_stmt_cache_size","max_connect_errors","max_connections","max_delayed_threads","max_digest_length","max_error_count","max_execution_time","max_heap_table_size","max_join_size","max_length_for_sort_data","max_points_in_geometry","max_prepared_stmt_count","max_relay_log_size","max_seeks_for_key","max_sort_length","max_sp_recursion_depth","max_user_connections","max_write_lock_count","multi_range_count","myisam_data_pointer_size","myisam_max_sort_file_size","myisam_mmap_size","myisam_repair_threads","myisam_sort_buffer_size","myisam_stats_method","myisam_use_mmap","mysql_native_password_proxy_users","net_buffer_length","net_read_timeout","net_retry_count","net_write_timeout","new","ngram_token_size","offline_mode","old","optimizer_prune_level","optimizer_search_depth","optimizer_switch","parser_max_mem_size","performance_schema","performance_schema_accounts_size","performance_schema_digests_size","performance_schema_events_stages_history_long_size","performance_schema_events_stages_history_size","performance_schema_events_statements_history_long_size","performance_schema_events_statements_history_size","performance_schema_events_transactions_history_long_size","performance_schema_events_transactions_history_size","performance_schema_events_waits_history_long_size","performance_schema_events_waits_history_size","performance_schema_hosts_size","performance_schema_max_cond_classes","performance_schema_max_cond_instances","performance_schema_max_digest_length","performance_schema_max_file_classes","performance_schema_max_file_handles","performance_schema_max_file_instances","performance_schema_max_index_stat","performance_schema_max_memory_classes","performance_schema_max_metadata_locks","performance_schema_max_mutex_classes","performance_schema_max_mutex_instances","performance_schema_max_prepared_statements_instances","performance_schema_max_program_instances","performance_schema_max_rwlock_classes","performance_schema_max_rwlock_instances","performance_schema_max_socket_classes","performance_schema_max_socket_instances","performance_schema_max_sql_text_length","performance_schema_max_stage_classes","performance_schema_max_statement_classes","performance_schema_max_statement_stack","performance_schema_max_table_handles","performance_schema_max_table_instances","performance_schema_max_table_lock_stat","performance_schema_max_thread_classes","performance_schema_max_thread_instances","performance_schema_session_connect_attrs_size","performance_schema_setup_actors_size","performance_schema_setup_objects_size","performance_schema_users_size","plugin_dir","port","preload_buffer_size","profiling_history_size","query_alloc_block_size","query_cache_limit","query_cache_min_res_unit","query_cache_size","query_cache_type","query_cache_wlock_invalidate","query_prealloc_size","range_alloc_block_size","range_optimizer_max_mem_size","read_buffer_size","read_only","read_rnd_buffer_size","relay_log_index","relay_log_info_file","relay_log_purge","relay_log_recovery","relay_log_space_limit","require_secure_transport","rpl_stop_slave_timeout","session_track_gtids","session_track_schema","session_track_state_change","session_track_system_variables","session_track_transaction_info","sha256_password_proxy_users","show_compatibility_56","show_create_table_verbosity","show_old_temporals","skip_external_locking","slave_allow_batching","slave_checkpoint_group","slave_checkpoint_period","slave_compressed_protocol","slave_exec_mode","slave_transaction_retries","slave_type_conversions","slow_launch_time","slow_query_log_file","socket","sort_buffer_size","stored_program_cache","super_read_only","sync_binlog","sync_frm","sync_master_info","sync_relay_log","sync_relay_log_info","thread_cache_size","thread_handling","thread_stack","tls_version","tmp_table_size","tmpdir","transaction_alloc_block_size","transaction_prealloc_size","updatable_views_with_limit","wait_timeout"] ;

   } ) ;

}());