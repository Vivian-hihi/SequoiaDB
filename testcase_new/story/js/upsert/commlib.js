/*******************************************************************************
*@Description: JavaScript common function library
*@Modify list:
*   2014-4-21 wenjing wang Init
*******************************************************************************/

//构造JSON字符串
function BuildObjStr(obj)
{
   var objstr;
   if ('[object Array]' === toString.apply(obj))
   {
      objstr = "["; 
      var first = true;
      for (item in obj)
      {
         if (!first)
         {
            objstr += ",";
         }
         else
         {
            first = false;
         }
         
         if ("object" == typeof(obj[item]))
         {
            objstr += BuildObjStr(obj[item]);
         }
         else if ("string" == typeof(obj[item]))
         {
            objstr += ("'" +  obj[item] + "'");
         }
         else
         {
            objstr +=  obj[item];
         }
      }
      objstr += "]";
   }
   else
   {
      objstr = "{";
      var first = true;
      for (item in obj)
      {
         if (!first)
         {
            objstr += ",";
         }
         else
         {
            first = false;
         }
         if ("object" == typeof(obj[item]))
         {
            objstr = objstr + item + ":";
            objstr += BuildObjStr(obj[item]);
         }
         else if ("string" == typeof(obj[item]))
         {
            objstr += (item + ":'" + obj[item] + "'");
         }
         else
         {
            objstr += (item + ":" + obj[item]);
         }
      }
      objstr += "}";
   }
   return objstr;
}

