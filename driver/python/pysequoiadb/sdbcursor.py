"""cursor interface of  SequoiaDB
"""

import sdbcursor
import bson

class replicanode(object):
    """Entrance of SequoiaDB


    """
    def err_process(self, result):
        if 2 != len(result):
            pass
        if 0 != result[0]:
            pass
        return result[1]

    def __init__(self, cursor):
        self.cursor = cursor

    def next(self):
        result = sdbcursor.next(self.cursor)
        bson_string = self.err_process(result)
        return bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)


    def current(self):
        result = sdbcursor.current(self.cursor)
        bson_string = self.err_process(result)
        return bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)

    def close(self):
        rc = sdbcursor.close(self.cursor)
        return rc

