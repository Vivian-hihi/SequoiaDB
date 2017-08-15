##NAME##

importOnce - Global only import and eval js file once.

##SYNOPSIS##

**importOnce(\<filename\>)**

##CATEGORY##

Global

##DESCRIPTION##

Meets the demand to import existing js files.

##PARAMETERS##

* `filename` ( *String*， *Required* )

   The relative path or the full path of js file。

##RETURN VALUE##

// TODO:

##ERRORS##

// TODO:

##HISTORY##

Since v2.9

##EXAMPLES##

1. Import and eval helloWorld.js repeatly,only import and eval for the first time.

    helloWorld.js as follow：

    ```lang-javascript
    function sayHello()
    {
        println( "hello world" ) ;
    }
    println( "import helloWorld.js" ) ;
    ```

    Import and eval helloWorld.js repeatly,only import and eval for the first time.

	```lang-javascript
	> importOnce( 'helloWorld.js' )
    import helloWorld.js
    Takes 0.000849s.
    > importOnce( 'helloWorld.js' )
    Takes 0.000354s.
    > sayHello()
    hello world
    Takes 0.000436s.
 	```
