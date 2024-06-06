程式碼:
```
int step1( char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Person";
   char*  attr_name[]={
                   "dept",
                   "person_num",
                   "middle_name",
                   "first_name",
                   "last_name",
                   "per_dpt_phone",
                   "person_identifier",
                   "pdi_network",
                   "pdi_node",
                   "pdi_system_id",
                   "db_object_id",
                   "update_count",
                   "email_id",
                   "email_type",
                   "active_flag",
                   0 } ;
   char*  attr_value[20];
   char   tmpstr[80];
   int    ListSize;
   int    i,j;
   char*  update_count ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(person_num,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "dept", data[0] );
      RC = PM_Set_Object_Attr(pObject, "person_num", data[1] );
      RC = PM_Set_Object_Attr(pObject, "middle_name", data[2] );
      RC = PM_Set_Object_Attr(pObject, "first_name", data[3] );
      RC = PM_Set_Object_Attr(pObject, "last_name", data[4] );
      RC = PM_Set_Object_Attr(pObject, "per_dpt_phone", data[5] );
      RC = PM_Set_Object_Attr(pObject, "person_identifier", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_system_id", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_network", data[7] );
      RC = PM_Set_Object_Attr(pObject, "pdi_node", data[8] );
      RC = PM_Set_Object_Attr(pObject, "active_flag", data[9] );
      RC = PM_Set_Object_Attr(pObject, "resp_type", data[10] );
      RC = PM_Set_Object_Attr(pObject, "email_id", data[11] );
      RC = PM_Set_Object_Attr(pObject, "email_type", data[12] );
      RC = PM_Set_Object_Attr(pObject, "db_object_id", NULL );
      RC = PM_Set_Object_Attr(pObject, "update_count", "0" );

      RC = PM_Save_Object( pObject);
      if( RC == 0 )
         fprintf( stdout, "%s created success\n", data[1] ) ;
      else
         ErrorCheck( "PM_Save_Object" );

      RC = PM_Destroy_Object( pObject );
   } else {
      RC = PM_Read_List( pList, 0, pObject );

      RC = PM_Get_Object_Attr( pObject,attr_name[10], attr_value[10] );
      RC = PM_Get_Object_Attr( pObject,attr_name[11], attr_value[11] );

      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "dept", data[0] );
      RC = PM_Set_Object_Attr(pObject, "person_num", data[1] );
      RC = PM_Set_Object_Attr(pObject, "middle_name", data[2] );
      RC = PM_Set_Object_Attr(pObject, "first_name", data[3] );
      RC = PM_Set_Object_Attr(pObject, "last_name", data[4] );
      RC = PM_Set_Object_Attr(pObject, "per_dpt_phone", data[5] );
      RC = PM_Set_Object_Attr(pObject, "person_identifier", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_system_id", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_network", data[7] );
      RC = PM_Set_Object_Attr(pObject, "pdi_node", data[8] );
      RC = PM_Set_Object_Attr(pObject, "active_flag", data[9] );
      RC = PM_Set_Object_Attr(pObject, "resp_type", data[10] );
      RC = PM_Set_Object_Attr(pObject, "email_id", data[11] );
      RC = PM_Set_Object_Attr(pObject, "email_type", data[12] );
      RC = PM_Set_Object_Attr(pObject, "db_object_id", attr_value[10] );
      RC = PM_Set_Object_Attr(pObject, "update_count", attr_value[11] );

      RC = PM_Save_Object( pObject);
      if( RC == 0 )
         fprintf( stdout, "%s updated success\n", data[1] ) ;
      else
         ErrorCheck( "PM_Save_Object" );

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );

   return( OK );
}
```

   char*           pClass = "AIDC_Person";
   char*  attr_name[]={
                   "dept",
                   "person_num",
                   "middle_name",
                   "first_name",
                   "last_name",
                   "per_dpt_phone",
                   "person_identifier",
                   "pdi_network",
                   "pdi_node",
                   "pdi_system_id",
                   "db_object_id",
                   "update_count",
                   "email_id",
                   "email_type",
                   "active_flag",
                   0 } ;
                   
### pClass的宣告尤為重要，因為這會對應到畫面上的的類別是哪一個
### attr_name的宣告也是，不同的pClass所需要的attr_name就不一樣

* PM_Create_List
* PM_Set_Clause Select
* PM_Set_Clause Where
* PM_Open_List
* PM_SizeOf_List
* PM_Read_List
* PM_Get_Object_Attr
* PM_Create_Object
* PM_Set_Object_Attr
* PM_Delete_Object
* PM_Destroy_Object
* PM_Destroy_List

這些為常見出現在進入讀取PDM系統內進行新增刪除修改查詢(CRUD)的Function

## PM_Create_List
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Create_List.png)

PM_Object_List* pList;  要記得宣告

呼叫方式:
`RC = PM_Create_List( pList, pClass );`

##  PM_Set_Clause

![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Set_Clause.png)

呼叫方式:
```
for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   //這樣寫是要配合客製化資料
   strcpy(tmpstr,"&=(person_num,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );
   
   //這樣寫是因為固定值就照寫
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
```
這裡有很詳盡的解釋Where、Select、OrderBy等條件該如何寫的語法
Where、Select在編寫客製化程式時很常需要用到
OrderBy的話則可以忽略沒關西,那是在畫面上呈現要給User看的時候方便用的

## PM_Open_List
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Open_List.png)

## PM_SizeOf_List
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_SizeOf_List.png)

int    ListSize;  要記得宣告

呼叫方式:
```
RC = PM_Open_List( pList );
RC = PM_SizeOf_List( pList, ListSize );
```

## PM_Create_Object
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Create_Object.png)

## PM_Set_Object_Attr
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Set_Object_Attr.png)

## PM_Save_Object
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Save_Object.png)

其實就把變數宣告好，丟進去Function就好

呼叫方式:
```
      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "dept", data[0] );
      RC = PM_Set_Object_Attr(pObject, "person_num", data[1] );
      RC = PM_Set_Object_Attr(pObject, "middle_name", data[2] );
      RC = PM_Set_Object_Attr(pObject, "first_name", data[3] );
      RC = PM_Set_Object_Attr(pObject, "last_name", data[4] );
      RC = PM_Set_Object_Attr(pObject, "per_dpt_phone", data[5] );
      RC = PM_Set_Object_Attr(pObject, "person_identifier", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_system_id", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_network", data[7] );
      RC = PM_Set_Object_Attr(pObject, "pdi_node", data[8] );
      RC = PM_Set_Object_Attr(pObject, "active_flag", data[9] );
      RC = PM_Set_Object_Attr(pObject, "resp_type", data[10] );
      RC = PM_Set_Object_Attr(pObject, "email_id", data[11] );
      RC = PM_Set_Object_Attr(pObject, "email_type", data[12] );
      RC = PM_Set_Object_Attr(pObject, "db_object_id", NULL );
      RC = PM_Set_Object_Attr(pObject, "update_count", "0" );

      RC = PM_Save_Object( pObject);
```

要注意的是PM_Set_Object_Attr裡面

![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Set_Object_Attr2.png)

**attr_name 放的是pclass中含有的欄位
而value則是預期會放我們要倒檔進去的值 OR 系統內定預設的值(建議就照寫不要修改)**

## ErrorCheck
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/ErrorCheck_code.png)
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/ErrorCheck_text.png)
```
void ErrorCheck( char* FuncName )
{
   int i, size ;
   fprintf( TraceFile, "RC = %d ---- %s\n", RC, FuncName );
   size = PM_Error_Size();
   for( i = 0; i < size; i++ ) {
      const char* code = PM_Error_MsgCode( i );
      const char* text = PM_Error_MsgTxt( i );
      fprintf( TraceFile, "   Error[%d] :\n", i );
      fprintf( TraceFile, "      Code : [%s]\n", code );
      fprintf( TraceFile, "      Text : [%s]\n", text );
   }
   fprintf( TraceFile, "\n" );
}
```
ErrorCheck()寫在msg.c這支程式碼裡面用來被呼叫
會將錯誤訊息寫進TraceFile : sdk.log裡
包含ErrorCode和ErrorText

## PM_Destroy_Object
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Destroy_Object.png)

## PM_Destroy_List
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/PM_Destroy_List.png)

在C語言中，記憶體管理是手動進行的。這意味著開發者需要自己負責記憶體的分配和釋放。C語言中沒有像Java那樣的垃圾回收機制（Garbage Collector）來自動處理不再使用的內存。如果在C語言中忘記釋放記憶體，就會造成記憶體洩漏，長期運行可能導致記憶體耗盡，系統崩潰。
