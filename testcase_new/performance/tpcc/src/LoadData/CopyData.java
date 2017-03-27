import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Copyright (c) 2017, SequoiaDB Ltd. File Name:CopyData.java ¿‡µƒœÍœ∏√Ë ˆ
 * 
 * @author wangwenjing Date:2017-3-27…œŒÁ9:49:18
 * @version 1.00
 */

public class CopyData {
    private static Properties ini = new Properties();

    public static void main( String[] args ) {
        try {
            ini.load( new FileInputStream( System.getProperty( "prop" ) ) );
        } catch ( IOException e ) {
            System.err.println( "ERROR: " + e.getMessage() );
            System.exit( 1 );
        }

        final String sdbUrl = "sdburl";
        if ( !ini.containsKey( sdbUrl ) ) {
            System.out.println( String.format( "need config %s", sdbUrl ) );
            System.exit( 1 );
        }

        final String warehouses = "warehouses";
        if ( !ini.containsKey( warehouses ) ) {
            System.out.println( String.format( "need config %s", warehouses ) );
            System.exit( 1 );
        }

        String url = ini.getProperty( sdbUrl );
        List< String > hosts = new ArrayList< String >();

        String[] urls = url.split( "," );
        for ( int i = 0; i < urls.length; ++i ) {
            String[] pair = urls[ i ].split( ":" );

            hosts.add( pair[ 0 ] );
        }

        ExecutorService cachedThreadPool = Executors.newCachedThreadPool();
        List< String > paras = new ArrayList< String >();
        paras.add( ini.getProperty( warehouses ) );
        for ( int j = 0; j < hosts.size(); ++j ) {
            cachedThreadPool.execute( new RemoteExecuter( "./misc/copyData.py",
                    hosts.get( j ), "sdbadmin", paras ) );
        }

        cachedThreadPool.shutdown();
    }
}
