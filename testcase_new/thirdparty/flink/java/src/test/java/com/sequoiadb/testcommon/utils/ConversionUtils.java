package com.sequoiadb.testcommon.utils;

import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.table.data.StringData;
import org.apache.flink.table.types.DataType;
import org.apache.flink.types.Row;
import org.apache.flink.util.CloseableIterator;

import java.sql.Timestamp;
import java.time.*;
import java.util.Date;

/**
 * @author
 * @descreption
 * @date
 * @updateUser
 * @updateDate 2022/2/11
 * @updateRemark
 */
public class ConversionUtils {

    /**
     * @descreption 类型转换中Long类型转换Bytes的规则
     * @param data
     * @return
     */
    public static byte[] toBytes( long data ) {
        byte b1 = ( byte ) ( data & 0xff );
        byte b2 = ( byte ) ( ( data >> 8 ) & 0xff );
        byte b3 = ( byte ) ( ( data >> 16 ) & 0xff );
        byte b4 = ( byte ) ( ( data >> 24 ) & 0xff );
        byte b5 = ( byte ) ( ( data >> 32 ) & 0xff );
        byte b6 = ( byte ) ( ( data >> 40 ) & 0xff );
        byte b7 = ( byte ) ( ( data >> 48 ) & 0xff );
        byte b8 = ( byte ) ( ( data >> 56 ) & 0xff );
        return new byte[] { b1, b2, b3, b4, b5, b6, b7, b8 };
    }

    /**
     * @descreption 类型转换中Int类型转换Bytes的规则
     * @param data
     * @return
     */
    public static byte[] toBytes( int data ) {
        byte b1 = ( byte ) ( data & 0xff );
        byte b2 = ( byte ) ( ( data >> 8 ) & 0xff );
        byte b3 = ( byte ) ( ( data >> 16 ) & 0xff );
        byte b4 = ( byte ) ( ( data >> 24 ) & 0xff );
        return new byte[] { b1, b2, b3, b4 };
    }

    /**
     * @descreption 将value转换为指定的flink数值类型
     * @param value
     * @param dataType
     * @return
     * @throws Exception
     */
    public static Object toDataType( String value, DataType dataType )
            throws Exception {
        if ( dataType.equals( DataTypes.TINYINT() ) ) {
            long data = Long.parseLong( value );
            return ( byte ) ( data & 0xff );
        } else if ( dataType.equals( DataTypes.SMALLINT() ) ) {
            return Short.parseShort( value );
        } else if ( dataType.equals( DataTypes.INT() ) ) {
            return Integer.parseInt( value );
        } else if ( dataType.equals( DataTypes.BIGINT() ) ) {
            return Long.parseLong( value );
        } else if ( dataType.equals( DataTypes.DOUBLE() ) ) {
            return Double.parseDouble( value );
        } else if ( dataType.equals( DataTypes.FLOAT() ) ) {
            return Float.parseFloat( value );
        } else if ( dataType.equals( DataTypes.STRING() ) ) {
            return StringData.fromString( value );
        } else {
            throw new Exception( "datatype should be value type" );
        }
    }

    /**
     * @descreption 将Date类型转换为LocalDate
     * @param date
     * @return
     */
    public static LocalDate getLocalDate( Date date ) {
        return Instant.ofEpochMilli( date.getTime() )
                .atZone( ZoneId.systemDefault() ).toLocalDate();
    }

    /**
     * @descreption 将Timestamp类型转换为LocalDateTime
     * @param timestamp
     * @return
     */
    public static LocalDateTime getLocalDateTime( Timestamp timestamp ) {
        return timestamp.toInstant().atZone( ZoneId.systemDefault() )
                .toLocalDateTime();
    }

    public static Row queryOne( StreamTableEnvironment tableEnv,
            String tableName ) throws Exception {
        CloseableIterator<Row> collect = tableEnv
                .executeSql("select * from " + tableName).collect();
        Row row;
        try {
             row = collect.next();
        }finally {
            collect.close();
        }
        return row;
    }

    public static String initTableName( String tableNameBase, DataType type ) {
        String time = String.valueOf( System.currentTimeMillis() );
        String random_id = time.substring( time.length() - 5 );
        String stype = type.toString();
        int index = stype.indexOf( "(" );
        if ( index > 0 ) {
            String typeName = stype.substring( 0, stype.indexOf( "(" ) );
            return tableNameBase + typeName + "_" + random_id;
        }
        return tableNameBase + stype + "_" + random_id;
    }

    /**
     * @descreption 创建Schema，所有列指定为相同数据类型
     * @param type
     * @param fileds
     * @return
     */
    public static Schema createSchemaByDataType( DataType type,
            String... fileds ) {
        Schema.Builder builder = Schema.newBuilder();
        for ( int i = 0; i < fileds.length; i++ ) {
            builder.column( fileds[ i ], type );
        }
        return builder.build();
    }
}
