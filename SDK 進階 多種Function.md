接下來會有很多倒檔需求會需要用到List 和 Object畫面操作

List畫面如:
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/LAI.png)

Object畫面如:
![image alt](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/LAI1.png)


```
/*-------------------------------------------------------------------*/
/*  Function : Check EC exists ?                                     */
/*-------------------------------------------------------------------*/
int step1(char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_EngineeringChange";
   char*  attr_name[]={
                   "db_object_id",
                   "programid@program_id",
                   "id",
                   "status@curr_key",
                   0 } ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(id, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );
/*
   RC = PM_Set_Clause( pList, "Where", "&LIKE(chg_control_code, '02%')" );
*/
   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -1000 ;
   } else {
      is = 1000 ;
      is = step2( data );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
```

首先會先去查看EC是否存在
所以會從PM_Create_List、PM_Set_Clause、PM_Open_List到PM_SizeOf_List
ListSize 就是要查看是否有資料
如果 ListSize = 0 就代表還沒有建立過這筆資料，要先請user去建立
才可以去做後續的處理

```
/*--------------------------------------------------------------------*/
/*  Function : Check ITEM+EC exists ?                                 */
/*--------------------------------------------------------------------*/
int step2(char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemEngineeringData";
   char*  attr_name[]={
                    "db_object_id",
                    "logical_key@item_number",
                    "il_ec_number",
                    "logical_key@loc_id",
                    "insert_by",
                    "pgmid@program_id",
                    "name",
                    "wbs",
                    "item_type",
                    "lai_obj@ai@eng_effty_level",
                    "lai_obj@act_eff_type",
                    "lai_obj@ai@change_action@curr_key",
                    "lai_obj@act_rev_level",
                    "lai_obj@item_status",
                    "lai_obj@ec@id",
                    "lai_obj@ec@db_object_id",
                    "lai_obj@ec@chg_control_code",
                    "lai_obj@mfg_seq_num",
                    "lai_obj@rt_revised_dt",
                    "update_count",
                    "ai_status",
                    0 } ;
   char*  attr_value[20] ;
   char*  is_tool ;
   char   tmpstr[512] ;
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(logical_key@item_number,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(lai_obj@ec@id,'");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, lai_obj@mfg_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, lai_obj@loc@loc_id)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize == 0 ) {
      is = -3000 ;
      is = step3( data );
   } else {
      is = 3000 ;
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
```

接下來一樣是判斷ITEM+EC存不存在
這裡的話一樣是PM_Create_List、PM_Set_Clause、PM_Open_List到PM_SizeOf_List
到這邊ListSize 就必須是0 因為代表還沒建立過

```
/*-----------------------------------------------------------------*/
/*  Function : EC LAI add                                          */
/*-----------------------------------------------------------------*/
int step3(char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_LocationAffectedItem";
   char*  attr_name[]={
                   "pid",
                   "item_number",
                   "ec@id",
                   "change_act_lai",
                   "db_object_id",
                   "update_count",
                   "act_re_run_IR_test",
                   "act_eff_type",
                   "act_eff_date",
                   "item_engg_status",
                   "item_status@curr_key",
                   "item_name",
                   "loc@loc_id",
                   "wbs",
                   "plan_finish_date",
                   0 } ;
   char*  objid ;
   char*  update_count ;
   char*  item ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   RC = PM_Create_Object(pObject, pClass);
   RC = PM_Set_Object_Attr(pObject, "db_object_id", "" );
   RC = PM_Set_Object_Attr(pObject, "update_count", "0" );
   RC = PM_Set_Object_Attr(pObject, "ec@id", data[2] );
   RC = PM_Set_Object_Attr(pObject, "item_number", data[1] );
   RC = PM_Set_Object_Attr(pObject, "change_act_lai", data[3] );
   RC = PM_Set_Object_Attr(pObject, "act_eff_type", "PU" );
   RC = PM_Set_Object_Attr(pObject, "act_re_run_IR_test", "N" );
   RC = PM_Set_Object_Attr(pObject, "loc@loc_id", "5100" );
   RC = PM_Set_Object_Attr(pObject, "act_eff_date", "" );
   RC = PM_Save_Object( pObject);
   if( RC >= 0 ) {
      is = 3010 ;
   } else {
      ErrorCheck("PM_Save_Object");
      is = -3010 ;
   }
   RC = PM_Destroy_Object( pObject ) ;
   RC = PM_Destroy_List( pList );
   return( is );
}
```
最後就是到LAI 新增資料
一樣是PM_Create_List、PM_Set_Clause
不過接下來就不是PM_Open_List了
而是 PM_Set_Object_Attr
因為在List的每一個列都是object
要以object的方式存進去
最後PM_Save_Object( pObject)