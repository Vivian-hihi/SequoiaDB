##NAME##

createLobID - Create a lob ID, not a lob

##SYNOPSIS##
***db.collectionspace.collection.createLobID([Timestamp])***

##CATEGORY##

Collection

##DESCRIPTION##

Create a lob ID from Server

##PARAMETERS##

* `Timestamp`( *Object*， *Optional* )
    
    Create a lob ID by Timestamp, the minimum precision is second. See [Timestamp](reference/Sequoiadb_command/SpecialObjects/Timestamp.md) for more detail.

* When no parameter is specified, Lob ID will be created by the Timestamp in server side.

##RETURN VALUE##

On success, return a lob ID.

On error, exception will be thrown.

##ERRORS##

When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##EXAMPLES##

* Create a lob ID by the Timestamp in server side.

    ```lang-javascript
    > db.foo.bar.createLobID()
    00005d36d096350002de7f3a
    Takes 0.329455s.
    ```

* Create a lob ID by TimeStamp.

    ```lang-javascript
    > db.foo.bar.createLobID( Timestamp( "2015-06-05-16.10.33.000000" ) )
    0000557159793e0002de7f6e
    Takes 0.108214s.
    ```