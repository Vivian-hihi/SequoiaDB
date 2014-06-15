"""Client interface of connectting SequoiaDB
"""

import sdbclient

class sdbclient(object):
    """Entrance of SequoiaDB
    

    """
    def __init__(self, host = default_host, port = default_port,
                       user = default_user, psw  = default_psw):

        self.cclient = sdbclient.create_client()
        if self.cclient is None:
           raise expection()
        
        rc = sdbclient.init_connect(self.cclient, host, port)
        if rc:
            pass
      
    def __del__(self):
        rc = sdbclient.release_cilent(self.cclient)
        if rc:
            pass

        self.cclient = None
            #todo: raise exception
                    
#        if self._connection is not None:
#            disconnect()
#        if self._sendbuffer is not None:
#            del _sendbuffer
#        self._sendbuffersize = 0
#        if self._recvbuffer is not None:
#            del _recvbuffer
#        self._recvbuffersize = 0

    def __getitem__(self, item_name):
        rc = sdbclient.get_collect_space(self.cclient, item_name, page_size,
                                         collection_space)
        if rc:
            pass

    def connect(self, host = default_host, port = default_port,
                      user = default_user, psw  = default_psw):
        rc = sdbclient.connect_by_host(self.cclient, host, port, user, psw)
        if rc:
            pass

    def connect(self, host, service_name,
                user = default_user,
                psw  = default_psw):
        rc = sdbclient.connect_by_service(self.cclient, host, service_name,
                                                        user, psw)
        if rc:
            pass

    def connect(self, addr, addr_size, user = default_user, psw = default_psw):
        rc = sdbclient.connect_by_address(self.cclient, addr, addr_size,
                                                        user, psw)
        if rc:
            pass

    def disconnect(self):
        rc = sdbclient.disconnect(self.cclient)
        if rc:
            pass

    def is_connected(self):
        rc = sdbclient.is_connected(self.cclient)
        if not rc:
            pass

    def create_user(self, name, psw):
        rc = sdbclient.create_user(self.cclient, name, psw)
        if rc:
            pass

    def remove_user(self, user, psw):
        rc = sdb.client.remove_user(self.cclient, user, psw)
        if rc:
            pass

    def get_snapshot(self, result, snap_type, condition = static_object,
                                              selector  = static_object,
                                              order_by  = static_object):
        rc = sdbclient.get_snapshot(self.cclient, result, snap_type,
                                    condition, selector, order_by)
        if rc:
            pass

    def reset_snapshot(self, condition = static_object):
        # todo:
        # dict to bson
        bson_condition = 0
        rc = sdbclient.reset_snapshot(self.cclient, bson_conditiion)
        if rc:
            pass

    def get_list(self, result, list_type,
                 condition = static_object,
                 selector  = static_object,
                 order_by  = static_object):
        rc = sdbclient.get_list(self.cclient, result, list_type,
                                condition, selector, order_by)
        if rc:
            pass

    def lock(self):
        sdbclient.lock(self.cclient)

    def unlock(self):
        sdbclient.unlock(self.cclient)

    def get_collection_space(self, cs_name, page_size, collection_space):
        rc = sdbclient.get_collection_space(self.cclient, cs_name, page_size,
                                            collection_space)
        if rc:
            pass

    def create_collection_space(self, cs_name, page_size, collection_space):
        rc = sdbclient.create_collection_space(self.cclient, cs_name, page_size,
                                               collection_space)
        if rc:
            pass

    def drop_collection_space(self, cs_name):
        rc = sdbclient.drop_collection_space(self.cclient, cs_name)
        if rc:
            pass

    def list_collection_spaces(self, result):
        rc = sdbclient.list_collection_spaces(self.cclient, result)
        if rc:
            pass

    def list_replica_groups(self, result):
        rc = sdbclient.list_replica_groups(self.cclient, result)
        if rc:
            pass

    def get_replica_group(self, group_name, result):
        rc = sdbclient.get_replica_group_by_name(self.cclient,
                                                 group_name, result)
        if rc:
            pass

    def get_replica_group(self, id, result):
        rc = sdbclient.get_replica_group_by_id(self.cclient, id, result)
        if rc:
            pass

    def creat_replica_group(self, group_name, replica_group):
        rc = sdbclient.create_replica_group(self.cclient,
                                            group_name, replica_group)
        if rc:
            pass

    def remove_replica_group(self, group_name):
        rc = sdbclient.remove_replica_group(self.cclient, group_name)
        if rc:
            pass

    def create_replica_cata_group(self, host, service_name, dbpath, configure):
        rc = sdbclient.create_replica_cata_group(self.cclient, host,
                                                 service_name,
                                                 dbpath,
                                                 configure)
        if rc:
            pass

    def active_replica_group(self, group_name, replica_group):
        rc = sdbclient.active_replica_group(self.cclient, group_name,
                                            replica_group)
        if rc:
            pass

    def exec_update(self, sql):
        rc = sdbclient.exec_update(self.cclient, sql)
        if rc:
            pass

    def exec_sql(self, sql, result):
        rc = sdbclient.exec_sql(self.cclient, sql, result)
        if rc:
            pass

    def transaction_begin(self):
        rc = sdbclient.transaction_begin(self.cclient)
        if rc:
            pass

    def transaction_commit(self):
        rc = sdbclient.transaction_commit(self.cclient)
        if rc:
            pass

    def transaction_rollback(self):
        rc = sdbclient.transaction_rollback(self.cclient)
        if rc:
            pass

    def flush_configure(self, options):
        #TODO: transfor options to bson
        bson_options = 0
        rc = sdbclient.flush_configure(self.cclient, bson_options)
        if rc:
            pass

#    def crt_js_procedure(self, code)
#    def rm_procedure(self, sp_name)
#    def list_procedures(self, cursor, condition)
#    def eval_js(self, cursor, code, type, err_msg)

    def backup_offline(self, options):
        #TODO: transfor options to bson
        rc = sdbclient.backup_offline(self.cclient, bson_options)
        if rc:
            pass

    def list_backup(self, cursor, options, conditions = static_object,
                                           selector = static_object,
                                           order_by = static_object):
        rc = sdbclient.list_backup(self.cclient, cursor, option,
                                   condition, selector, order_by)
        if rc:
            pass

    def list_task(self, cursor, condition = static_object,
                                selector  = static_object,
                                order_by  = static_object,
                                hint      = static_object):
        rc = sdbclient.list_task(self.cclient, cursor, condition,
                                 selector, order_by, hint)
        if rc:
            pass

    def wait_task(self, task_ids, num):
        rc = sdbclient.wait_task(self.cclient, task_ids, num)
        if rc:
            pass

    def cancel_task(self, task_id, is_async):
        rc = sdbclient.cancel_task(self.cclient, task_id, is_async)
        if rc:
            pass

    def set_session_attri(self, options = static_object):
        #TODO: transfor options to bson
        rc = sdbclient.set_session_attri(self.cclient, bosn_options)
        if rc:
            pass

    def close_all_cursors(self):
        rc = sdbclient.close_all_cursors(self.cclient)
        if rc:
            pass

    def is_valid(self, result):
        rc = sdbclient.is_valid(self.cclient, result)
        if rc:
            pass
