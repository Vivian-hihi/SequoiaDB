class sdbcollectionspace(object):
    """CollectionSpace for SequoiaDB"""
    
    def __init__(self):
        """'cs' is short for collection space"""
        self.cs = sdbcs.create_cs()
        if self.cs is None:
            pass

    def __del__(self):
        rc = sdbcs.release_cs(self.cs)
        if rc:
            pass

    def __getitem__(self, item_name, collection):
        rc = get_collection(item_name, collection)
        if rc:
            pass

    def get_collection(self, cl_name, collection):
        rc = sdbcs.get_collection(self.cs, cl_name, collection)
        if rc:
            pass

    def create_collection(self, cl_name, collection, options = static_object):
        rc = sdbcs.get_collection(self.cs, cl_name, options, collection)
        if rc:
            pass

    def drop_collection(self, cl_name):
        rc = sdbcs.drop_collection(self.cs, cl_name)
        if rc:
            pass

    def get_collection_name(self):
        cs_name = sdbcs.get_collection_name(self.cs)
        return cs_name