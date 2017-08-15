##NAME##

import - Import and eval js file.

##SYNOPSIS##

**import(\<filename\>)**

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

1. Import and eval helloWorld.js

    helloWorld.js as follow：

    ```lang-javascript
    function sayHello()
    {
        println( "hello world" ) ;
    }
    println( "import helloWorld.js" ) ;
    ```

	Import and eval helloWorld.js, then call the sayHello().

	```lang-javascript
	> import( 'helloWorld.js' )
    import helloWorld.js
    Takes 0.000901s.
    > sayHello()
    hello world
    Takes 0.000475s.
 	```
