package com.mongodb.utils;

import java.io.Serializable;
import java.util.Arrays;

import com.mongodb.BasicDBObject;

import lombok.Data;

/**
 * @Description : 实体类，用来测试spring
 * @author fanyu
 * @version 1.00
 * @Date 2020/4/22
 */
@Data
public class Entity implements Serializable {
    public static final String[] SEXS = { "m", "w" };
    public static final String[] COURSES = { "chinese", "math", "english" };
    private static final long serialVersionUID = 1L;
    private String name;
    private String sex;
    private int age;
    private String id;
    private int grade;
    private String[] courses;

    public Entity() {
    }

    public Entity( String name, String sex, int age, int grade,
            String[] courses ) {
        super();
        this.name = name;
        this.sex = sex;
        this.age = age;
        this.grade = grade;
        this.courses = courses;
    }

    public String getName() {
        return name;
    }

    public void setName( String name ) {
        this.name = name;
    }

    public String getSex() {
        return sex;
    }

    public void setSex( String sex ) {
        this.sex = sex;
    }

    public int getAge() {
        return age;
    }

    public void setAge( int age ) {
        this.age = age;
    }

    public String getId() {
        return id;
    }

    public void setId( String id ) {
        this.id = id;
    }

    public String[] getCourses() {
        return courses;
    }

    public void setCourses( String[] courses ) {
        this.courses = courses;
    }

    public int getGrade() {
        return grade;
    }

    public void setGrade( int grade ) {
        this.grade = grade;
    }

    public BasicDBObject toBSON() {
        return new BasicDBObject( "name", this.name ).append( "sex", this.sex )
                .append( "age", this.age ).append( "grade", this.grade )
                .append( "courses", this.courses );
    }

    @Override
    public String toString() {
        return "Entity{" + "name='" + name + '\'' + ", sex='" + sex + '\''
                + ", age=" + age + ", id='" + id + '\'' + ", grade=" + grade
                + ", courses=" + Arrays.toString( courses ) + '}';
    }

    @Override
    public boolean equals( Object o ) {
        if ( this == o )
            return true;
        if ( o == null || getClass() != o.getClass() )
            return false;

        Entity entity = ( Entity ) o;

        if ( age != entity.age )
            return false;
        if ( grade != entity.grade )
            return false;
        if ( name != null ? !name.equals( entity.name ) : entity.name != null )
            return false;
        if ( sex != null ? !sex.equals( entity.sex ) : entity.sex != null )
            return false;
        if ( id != null ? !id.equals( entity.id ) : entity.id != null )
            return false;
        // Probably incorrect - comparing Object[] arrays with Arrays.equals
        return Arrays.equals( courses, entity.courses );
    }

    @Override
    public int hashCode() {
        int result = name != null ? name.hashCode() : 0;
        result = 31 * result + ( sex != null ? sex.hashCode() : 0 );
        result = 31 * result + age;
        result = 31 * result + ( id != null ? id.hashCode() : 0 );
        result = 31 * result + grade;
        result = 31 * result + Arrays.hashCode( courses );
        return result;
    }
}