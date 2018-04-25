/*******************************************************************************
@Description : alter common functions
@Modify list :
               2018-4-25  wuyan  Init
*******************************************************************************/

//inspect the alter field is success or not.
function checkAlterResult(clName, fieldName, expFieldValue)
{
   var clFullName = COMMCSNAME + "." + clName;   
   var cur = db.snapshot(8,{"Name":clFullName});
   var actualFieldValue = cur.current().toObj()[fieldName];
   
   if ( typeof(expFieldValue) === "object" )
   {      
      if (JSON.stringify(expFieldValue) !== JSON.stringify(actualFieldValue))
      {
         throw buildException("test fieldvalue1", "check field", "value is wrong", JSON.stringify(expFieldValue), JSON.stringify(actualFieldValue));
      }
      
   }
   else
   {
      if (expFieldValue  !== actualFieldValue)
      {
         throw buildException("test fieldvalue2", "check field", "value is wrong", expFieldValue, actualFieldValue);
      }
   }
   
}
