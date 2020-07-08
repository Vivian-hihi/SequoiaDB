package com.mongodb.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Random;

import org.apache.commons.codec.binary.Hex;
import org.apache.log4j.Logger;

public class TestTools {
    private static final Logger logger = Logger.getLogger( TestTools.class );

    /**
     * get file's md5
     *
     * @param pathName
     * @return
     * @throws IOException
     */
    public static String getMD5( String pathName ) throws IOException {
        return getMD5( new FileInputStream( new File( pathName ) ) );
    }

    public static String getMD5( InputStream inputStream ) throws IOException {
        try {
            MessageDigest md5 = MessageDigest.getInstance( "MD5" );
            byte[] buffer = new byte[ 8192 ];
            int length;
            while ( ( length = inputStream.read( buffer ) ) != -1 ) {
                md5.update( buffer, 0, length );
            }
            return new String( Hex.encodeHex( md5.digest() ) );
        } catch ( NoSuchAlgorithmException e ) {
            e.printStackTrace();
            return null;
        } finally {
            if ( inputStream != null ) {
                inputStream.close();
            }
        }
    }

    public static String getMD5( Object buffer ) {
        try {
            MessageDigest md5 = MessageDigest.getInstance( "MD5" );
            if ( buffer instanceof ByteBuffer ) {
                md5.update( ( ByteBuffer ) buffer );
            } else if ( buffer instanceof byte[] ) {
                md5.update( ( byte[] ) buffer );
            } else {
                throw new IllegalArgumentException( "invalid type of buffer" );
            }
            return new String( Hex.encodeHex( md5.digest() ) );
            // have bug,it will get rid of the 0 in front
            /*
             * BigInteger bi = new BigInteger(1, md5.digest()); value =
             * bi.toString(16);
             */
        } catch ( NoSuchAlgorithmException e ) {
            e.printStackTrace();
            throw new RuntimeException( "fail to get md5!" + e.getMessage() );
        }
    }

    public static String inputStream2File( InputStream inputStream,
            String downloadPath ) throws IOException {
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream( new File( downloadPath ), true );
            byte[] read_buf = new byte[ 1024 ];
            int read_len = 0;
            while ( ( read_len = inputStream.read( read_buf ) ) > -1 ) {
                fos.write( read_buf, 0, read_len );
            }
        } finally {
            if ( fos != null ) {
                fos.close();
            }
        }
        return downloadPath;
    }

    /**
     * random generate string
     *
     * @param length
     * @return character string
     */
    public static String getRandomString( int length ) {
        String str = "adcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        Random random = new Random();
        StringBuffer sb = new StringBuffer();
        for ( int i = 0; i < length; i++ ) {
            int number = random.nextInt( str.length() );
            sb.append( str.charAt( number ) );
        }
        return sb.toString();
    }

    /**
     * get buffer
     *
     * @param filePath
     * @return byte[]
     * @throws IOException
     */
    public static byte[] getBuffer( String filePath ) throws IOException {
        File file = new File( filePath );
        long fileSize = file.length();
        if ( fileSize > Integer.MAX_VALUE ) {
            System.out.println( "file too big..." );
            return null;
        }
        FileInputStream fi = new FileInputStream( file );
        byte[] buffer = new byte[ ( int ) fileSize ];
        int offset = 0;
        int numRead = 0;
        while ( offset < buffer.length && ( numRead = fi.read( buffer, offset,
                buffer.length - offset ) ) >= 0 ) {
            offset += numRead;
        }
        // 确保所有数据均被读取
        if ( offset != buffer.length ) {
            extracted( file );
        }
        fi.close();
        return buffer;
    }

    private static void extracted( File file ) throws IOException {
        throw new IOException( "failed to get buffer, file=" + file.getName() );
    }

    /**
     * get method... name
     */
    public static String getMethodName() {
        return Thread.currentThread().getStackTrace()[ 2 ].getMethodName();
    }

    public static String getClassName() {
        String fullClassName = Thread.currentThread().getStackTrace()[ 2 ]
                .getClassName();
        int index = fullClassName.lastIndexOf( "." );
        return fullClassName.substring( index + 1 );
    }

    public static class LocalFile {

        /**
         * read the specify file length after seek, to compare the read results
         * with SCM
         *
         * @param filePath
         * @param size
         * @param len
         * @param downloadPath
         * @throws FileNotFoundException
         * @throws IOException
         */
        public static void readFile( String filePath, int size, int len,
                String downloadPath )
                        throws FileNotFoundException, IOException {
            RandomAccessFile raf = null;
            OutputStream fos = null;
            try {
                raf = new RandomAccessFile( filePath, "rw" );
                fos = new FileOutputStream( downloadPath );
                raf.seek( size );
                int off = 0;
                int readSize = 0;
                byte[] buf = new byte[ off + len ];
                readSize = raf.read( buf, off, len );
                fos.write( buf, off, readSize );
            } finally {
                if ( raf != null )
                    raf.close();
                if ( fos != null )
                    fos.close();
            }
        }

        /**
         * read the entire file length after the seek, to compare the read
         * results with SCM
         *
         * @param sourceFile
         * @param size
         * @param outputFile
         * @throws FileNotFoundException
         * @throws IOException
         */
        public static void readFile( String sourceFile, int size,
                String outputFile ) throws FileNotFoundException, IOException {
            RandomAccessFile raf = null;
            OutputStream fos = null;
            try {
                raf = new RandomAccessFile( sourceFile, "rw" );
                fos = new FileOutputStream( outputFile );
                raf.seek( size );
                int readSize = 0;
                int off = 0;
                int len = 1024 * 1024;
                byte[] buf = new byte[ off + len ];
                while ( true ) {
                    readSize = raf.read( buf, off, len );
                    if ( readSize <= 0 ) {
                        break;
                    }
                    fos.write( buf, off, readSize );
                }
            } finally {
                if ( raf != null )
                    raf.close();
                if ( fos != null )
                    fos.close();
            }
        }

        /**
         * create local directory
         *
         * @param dir
         */
        public static void createDir( String dir ) {
            mkdir( new File( dir ) );
        }

        private static void mkdir( File filePath ) {
            if ( !filePath.getParentFile().exists() ) {
                mkdir( filePath.getParentFile() );
            }
            filePath.mkdir();
        }

        /**
         * remove directory including directories and sub files
         *
         * @param filePath
         */
        public static void removeFile( String filePath ) {
            File file = new File( filePath );
            if ( file.exists() ) {
                file.delete();
            }
        }

        public static void removeFile( File file ) {
            if ( file.exists() ) {
                if ( file.isFile() ) {
                    file.delete();
                } else {
                    File[] files = file.listFiles();
                    for ( File subFile : files ) {
                        removeFile( subFile );
                    }

                    file.delete();
                }
            }
        }

        /**
         * create empty file
         *
         * @param filePath
         * @throws IOException
         */
        public static void createFile( String filePath ) throws IOException {
            File file = new File( filePath );
            if ( file.exists() ) {
                file.delete();
            }
            mkdir( file.getParentFile() );
            file.createNewFile();
        }

        /**
         * create file, the file content are randomly generated character
         *
         * @param filePath
         * @param size
         * @throws IOException
         */
        public static void createFile( String filePath, long size )
                throws IOException {
            FileOutputStream fos = null;
            try {
                createFile( filePath );
                File file = new File( filePath );
                fos = new FileOutputStream( file );
                long written = 0;
                byte[] fileBlock = new byte[ 1024 ];
                while ( written < size ) {
                    new Random().nextBytes( fileBlock );
                    long toWrite = size - written;
                    long len = fileBlock.length < toWrite ? fileBlock.length
                            : toWrite;
                    fos.write( fileBlock, 0, ( int ) len );
                    written += len;
                }
            } catch ( IOException e ) {
                System.out.println( "create file failed, file=" + filePath );
                throw e;
            } finally {
                if ( fos != null ) {
                    fos.close();
                }
            }
        }

        /**
         * create file, the file content are specify characters
         *
         * @param filePath
         * @param content
         * @param size
         * @throws IOException
         */
        public static void createFile( String filePath, String content,
                int size ) throws IOException {
            FileOutputStream fos = null;
            byte[] contentBytes = content.getBytes();
            int written = 0;
            try {
                File file = new File( filePath );
                fos = new FileOutputStream( file );
                while ( written < size ) {
                    int toWrite = size - written;
                    int len = contentBytes.length < toWrite
                            ? contentBytes.length : toWrite;
                    fos.write( contentBytes, 0, len );
                    written += len;
                }
            } catch ( IOException e ) {
                System.out.println( "create file failed:file=" + filePath );
                e.printStackTrace();
            } finally {
                if ( fos != null )
                    fos.close();
            }
        }

        /**
         * create download path and file, by methodName and threadId
         */
        public static String initDownloadPath( File localPath,
                String methodName, long threadId ) throws Exception {
            String downloadPath = null;
            try {
                int randomId = new Random().nextInt( 10000 );
                String downLoadDir = localPath + File.separator + methodName;
                createDir( downLoadDir );
                downloadPath = downLoadDir + File.separator + "thread-"
                        + threadId + "_" + System.currentTimeMillis() + "_"
                        + randomId + ".lob";
            } catch ( Exception e ) {
                logger.info( "downloadPath\n" + downloadPath );
                throw e;
            }
            return downloadPath;
        }
    }
}
