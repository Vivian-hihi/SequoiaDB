class sdbcollection(object):
    """Collection for SequoiaDB"""

    def __init__(self):
        self.cl = sdbcl.create()

    def __del__(self):
        if self.cl is not None:
            sdbcl.release(self.cl)
            self.cl = None

    def get_count(self, count, condition = static_object):
        rc = sdbcl.get_count(self.cl, count, condition)
        if rc:
            pass

    def split(self, source_group_name, target_group_name,
                    split_condition, split_end_contiditon = static_object):
        rc = sdbcl.split(self.cl, source_group_name, target_group_name,
                                  split_condition, split_end_condition)
        if rc:
            pass

    def split(self, source_group_name, target_group_name, precent):
        rc = sdbcl.split(self.cl, source_group_name, target_group_name, precent)
        if rc:
            pass

    def split_async(self, task_id, source_group_name, target_group_name,
                                   split_condition,
                                   split_end_conditon = static_object):
        rc = sdbcl.split_async(self.cl, task_id, source_group_name,
                                                 target_group_name,
                                                 split_condition,
                                                 split_end_condition)
        if rc:
            pass

    def split_async(self, source_group_name, target_group_name,
                          percent, task_id):
        rc = sdbcl.splite_async(self.cl, source_group_name, target_group_name,
                                         precent, task_id)
        if rc:
            pass

    def bulk_insert(self, falgs, *obj):
        rc =sdbcl.bulk_insert(self.cl, flags, obj)
        if rc:
            pass

    def insert(obj, id = None):
        rc = sbdcl.insert(self.cl. obj, id)
        if rc:
            pass

    def update(self, rule, condition = static_object, hint = static_object):
        rc = sdbcl.update(self.cl, rule, condition, hint)
        if rc:
            pass

    def upsert(self, rule, condition = static_object, hint = static_object):
        rc = sdbcl.upsert(self.cl, rule, condition, hint)
        if rc:
            pass

    def delete(self, condition = static_objec, hint = static_object):
        rc = sdbcl.delete(self.cl, condition, hint)
        if rc:
            pass

    def query(self, cursor, condition = static_object, selected = static_object,
                            order_by = static_object, hint = stactic_object,
                            num_to_skip = 0, num_to_return = -1):
        rc = sdbcl.query(self.cl, cursor, condition, selected, order_by, hint,
                                          num_to_skip, num_to_return)
        if rc:
            pass

    def create_index(self, idx_name, is_unique, is_enforced):
        rc = sdbcl.create_index(self.cl, idx_name, is_unique, is_enforced)
        if rc:
            pass

    def get_indexes(self, cursor, idx_name):
        rc = sdbcl.get_indexes(self.cl, cursor, idx_name)
        if rc:
            pass

    def drop_index(self, idx_name):
        rc = sdbcl.drop_index(self.cl, idx_name)
        if rc:
            pass

    def get_collection_name(self):
        cl_name = sdbcl.get_collection_name(self.cl)
        return cl_name

    def get_cs_name(self):
        cs_name = sdbcl.get_cs_name(self.cl)
        return cs_name

    def get_full_name(self):
        full_name = sdbcl.gey_full_name(self.cl)
        return full_name

    def aggregate(self, cursor, obj):
        rc = sdbcl.aggregate(self.cl, cursor, obj)
        if rc:
            pass

    def get_query_meta(self, cursor, condition, order_by, hint,
                             num_to_skip, num_to_return):
        rc = sdbcl.get_query_meta(self.cl, cursor, condition, order_by, hint,
                                           num_to_skip, num_to_return)
        if rc:
            pass

    def attach_collection(self, cl_full_name, options):
        rc = sdbcl.attach_collection(self.cl, cl_full_name, options)
        if rc:
            pass

    def detach_collection(self, sub_cl_full_name):
        rc = sdbcl.detach_collection(self.cl, sub_cl_full_name)
        if rc:
            pass
