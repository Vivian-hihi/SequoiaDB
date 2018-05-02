package com.sequoiadb.meta;

import java.util.ArrayList;

import org.testng.Assert;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

public class Commlib {
    private static ArrayList<String> groupList ;
    public static boolean isStandAlone(Sequoiadb sdb){
        try{
            sdb.listReplicaGroups();        
        }catch(BaseException e){
            if( e.getErrorCode() == -159 ){
                System.out.printf("run mode is standalone");     
                return true;
            }   
        }   
        return false;
    }
    
    public static boolean OneGroupMode(Sequoiadb sdb){
         if(getDataGroups(sdb).size() < 2){
            System.out.printf("only one group");
            return true;
         }
         return false;
     }
    
    public static ArrayList<String> getDataGroups(Sequoiadb sdb){        
         try{
             groupList = sdb.getReplicaGroupNames();
             groupList.remove("SYSCatalogGroup");
             groupList.remove("SYSCoord");
             groupList.remove("SYSSpare");
         }catch(BaseException e){               
             Assert.assertTrue(false,"getDataGroups fail " + e.getMessage());               
         }          
         return groupList;
     } 
}
