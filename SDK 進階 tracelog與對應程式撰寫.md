[disposition]

```
/*-------------------------------------------------------------------*/
/*  Function : Check Item List exists ?                              */
/*-------------------------------------------------------------------*/
int step1(char* data[] )
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
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;
   char*  db_object_id ;
   char*  locid ;\
   char*  il_ec_number;
   char*  item_number ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&LIKE(logical_key@item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&LIKE(lai_obj@ec@id, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, lai_obj@mfg_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, lai_obj@loc@loc_id)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -1000 ;
   } else {
      is = 1000 ;
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject, attr_name[0], db_object_id );
         RC = PM_Get_Object_Attr( pObject, attr_name[2], il_ec_number );
         RC = PM_Get_Object_Attr( pObject, attr_name[1], item_number );
         RC = PM_Get_Object_Attr( pObject, attr_name[3], locid );

      /*printf("db_object_id = %s \n", db_object_id );
      printf("il_ec_number = %s \n", il_ec_number );
      printf("item_number = %s \n", item_number );
      printf("locid = %s \n", locid );
      printf("\n" );*/
      }

      is = step2(data, db_object_id, il_ec_number, item_number ,locid );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
```

因為是從AIDC_ItemEngineeringData這個class進去

   strcpy(tmpstr,"&LIKE(logical_key@item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&LIKE(lai_obj@ec@id, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, lai_obj@mfg_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, lai_obj@loc@loc_id)" );

所以就開始把一些固定的欄位資訊丟進去where條件裡面
像是logical_key@item_number、lai_obj@ec@id這兩個就蠻重要的，剩下個三個就固定照抄
最後是openlist以及看Listsize=多少
如果=0就代表沒有任何一筆資料,這邊把問題回報給user請他新增之後，才能夠把資料加進去
如果!=0就代表已經建立好資料可以進入後續的處理

```
   if( ListSize == 0 ) {
      is = -1000 ;
   } else {
      is = 1000 ;
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject, attr_name[0], db_object_id );
         RC = PM_Get_Object_Attr( pObject, attr_name[2], il_ec_number );
         RC = PM_Get_Object_Attr( pObject, attr_name[1], item_number );
         RC = PM_Get_Object_Attr( pObject, attr_name[3], locid );

      is = step2(data, db_object_id, il_ec_number, item_number ,locid );
   }
```

因為在AIDC_ItemEngineeringData這個class下的某些欄位資料是蠻重要的，會需要在後續的操作上用到
如:db_object_id、logical_key@item_number、il_ec_number、logical_key@loc_id
所以會使用PM_Get_Object_Attr去抓到資料並傳給相對應的宣告變數
db_object_id、il_ec_number、item_number、locid
最後把它丟到step2進行使用

```
/*-------------------------------------------------------------------*/
/*  Function : Get lai_obj@db_object_id                           */
/*-------------------------------------------------------------------*/

int step2(char* data[], char* db_object_id, char* il_ec_number, char* item_number, char* locid)
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemEngineeringData";
   char*  attr_name[]= {
            "db_object_id",
            "cl_seq",
            "master_item@db_object_id",
            "lai_obj@db_object_id",
            "logical_key@item_number",
            "il_ec_number",
            "logical_key@loc_id",
            "db_object_id2",
            "rev_level",
            0
            };
   char*  lai_obj_db_object_id ;
   int    j;
   int    is = 0 ;
   char*  rev_level ;

   RC = PM_Create_Object( pObject, pClass );

   RC = PM_Set_Object_Attr( pObject, "db_object_id", db_object_id );
   RC = PM_Set_Object_Attr( pObject, "logical_key@item_number", item_number );
   RC = PM_Set_Object_Attr( pObject, "il_ec_number", il_ec_number );
   RC = PM_Set_Object_Attr( pObject, "logical_key@loc_id", locid );

   for( j=0; attr_name[j]; j++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[j] );

   RC = PM_Open_Object( pObject );

   RC = PM_Get_Object_Attr( pObject, attr_name[3], lai_obj_db_object_id );
   RC = PM_Get_Object_Attr( pObject, attr_name[8], rev_level );
   /*printf("lai_obj_db_object_id = %s \n", lai_obj_db_object_id );*/

   is = step3(data,lai_obj_db_object_id,rev_level );

   RC = PM_Destroy_Object( pObject );
   return( is );
}
```

接下來是從List(紅框)裡面抓到object(藍框),因為我們會需要點進去按鈕(紫框)
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/list_object.png)
所以要從object裡面抓到一些必要的欄位
會把db_object_id、logical_key@item_number、il_ec_number、logical_key@loc_id這四個值丟進去
因為lai_obj@db_object_id、rev_level是後續需要用到的欄位資訊
所以在這邊一樣使用所以會使用PM_Get_Object_Attr去抓到資料並傳給相對應的宣告變數
最後把它丟到step3進行使用

```
/*-------------------------------------------------------------------*/
/*  Function : EC IAD add                          */
/*-------------------------------------------------------------------*/

int step3(char* data[],char* lai_obj_db_object_id, char* rev_level)
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemActualDisposition";
   char*  attr_name[]={
                   "act_re_run_FAI",
                   "actual_disp",
                   "db_object_id",
                   "il_action",
                   "item_number",
                   "item_rev",
                   "lai_obj.db_object_id",
                   "physical_st",
                   "item_number",
                   "prod_id",
                   "sort_seq",
                   "cut_in_unit",
                   "quantity",
                   0 } ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;
   char*  db_object_id ;
   char*  locid ;
   char*  il_ec_number;
   char*  item_number ;


   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );


   strcpy(tmpstr,"&LIKE(prod_id, '");
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(lai_obj.db_object_id, '");
   strcat(tmpstr,lai_obj_db_object_id);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   /*printf("***%s \n", lai_obj_db_object_id );*/


   strcpy(tmpstr,"&=(item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   /*printf("***%s \n", data[1] );*/

   strcpy(tmpstr,"&=(physical_st,'");
   strcat(tmpstr,data[6]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(actual_disp,'");
   strcat(tmpstr,data[7]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(cut_in_unit, ");
   strcat(tmpstr,data[9]);
   strcat(tmpstr," )");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(quantity, ");
   strcat(tmpstr,data[11]);
   strcat(tmpstr," )");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

/*
   RC = PM_Set_Clause( pList, "Where", "&LIKE(chg_control_code, '02%')" );
*/
   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -3000 ;
      is = step4(data, lai_obj_db_object_id, rev_level );
   } else {
      is = 3000 ;

   }

   RC = PM_Destroy_List( pList );
   return( is );
}
```
接下來就是點進去按鈕裡面的操作
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/Item_disposition.png)
這邊會先把AIDC_ItemActualDisposition這個class先以openlist的方式打開
透過where條件下的限制去找出我們到底有沒有新增過這筆資料
在上面的程式碼中,where條件越詳細越好
因為要避免重複新增資料的問題發生

如果Listsize=0時代表沒有新增過資料 !=0時代表資料已經存在就不必再新增

最後是比較複雜的地方，先從trace.log來看

> **IN: Method Call Class - AIDCADIS.  Method - FA12B**
> **IN: Required: - config = AJT**
> **IN: Required: - object.db_object_id = 
> IN: Required: - object.update_count = 0
> IN: Required: - object.lai_obj.db_object_id = 00E93B06DB564B6600800186009C0005
> IN: Required: - object.effective_date = 05/23/2024
> IN: Required: - object.quantity = 1**
> **IN: IfDirty: - object.act_re_run_FAI = NR
> IN: IfDirty: - object.actual_disp = 03
> IN: IfDirty: - object.affect_op = 
> IN: IfDirty: - object.cut_in_unit = 0
> IN: IfDirty: - object.cut_out_unit = 2147483647
> IN: IfDirty: - object.db_object_id = 
> IN: IfDirty: - object.desc = 
> IN: IfDirty: - object.effective_date = 05/23/2024
> IN: IfDirty: - object.item_number = 10-24-0153-001
> IN: IfDirty: - object.item_rev = -
> IN: IfDirty: - object.lai_obj.db_object_id = 00E93B06DB564B6600800186009C0005
> IN: IfDirty: - object.leak_req = N
> IN: IfDirty: - object.master_body_remake = N
> IN: IfDirty: - object.physical_st = O
> IN: IfDirty: - object.prod_id = 
> IN: IfDirty: - object.quantity = 1
> IN: IfDirty: - object.update_count = 0**
> IN: Subscribe: - object.act_re_run_FAI
> IN: Subscribe: - object.actual_disp
> IN: Subscribe: - object.affect_op
> IN: Subscribe: - object.cut_in_unit
> IN: Subscribe: - object.cut_out_unit
> IN: Subscribe: - object.db_object_id
> IN: Subscribe: - object.desc
> IN: Subscribe: - object.effective_date
> IN: Subscribe: - object.item_number
> IN: Subscribe: - object.item_rev
> IN: Subscribe: - object.lai_obj.db_object_id
> IN: Subscribe: - object.leak_req
> IN: Subscribe: - object.master_body_remake
> IN: Subscribe: - object.physical_st
> IN: Subscribe: - object.prod_id
> IN: Subscribe: - object.quantity
> IN: Subscribe: - object.update_count
> IN: Subscribe: - object.disabled_actions
> 
> **AVNRPCIF -- client invoking AIDC_ItemActualDisposition(AIDCADIS).cl_save(000fa12b)**

從上面的部分可以看到第一個IN: Required: - config = AJT
並沒有像是之前所看到是以object.開頭的欄位名字
這個時候要用另外一種方式宣告

```
/*-----------------------------------------------------------------*/
/*  Function : EC IAD add                                          */
/*-----------------------------------------------------------------*/
int step4(char* data[],char* lai_obj_db_object_id, char* rev_level )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemActualDisposition";
   char*  attr_name[]={
                     "db_object_id",
                     "item_number",
                     "affect_op",
                     "act_re_run_FAI",
                     "desc",
                     "effective_date",
                     "physical_st",
                     "actual_disp",
                     "prod_id",
                     "cut_in_unit",
                     "cut_out_unit",
                     "quantity",
                     "item_rev",
                     "master_body_remake",
                     "leak_req",
                     "update_count",
                     "lai_obj@db_object_id",
                   0 } ;
   char*  item ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;
   char*  subscribeList = NULL;
   PMInstanceListRef* pIList;
   PMInstanceRef*     confINST;

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();
   confINST = PMSDK_CreateInstance();

   for(i=1; i < 12; i++) {
      PMSDK_SetAttrValue( pObject, attr_name[i], data[i], pObject );
   }

   PMSDK_SetAttrValue( pObject, "lai_obj@db_object_id", lai_obj_db_object_id, pObject );
   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
   PMSDK_SetAttrValue( pObject, "affect_op", "", pObject );
   PMSDK_SetAttrValue( pObject, "master_body_remake", "N", pObject );
   PMSDK_SetAttrValue( pObject, "leak_req", "N", pObject );
   if( strlen(rev_level)==0 )
      PMSDK_SetAttrValue( pObject, "item_rev", "-", pObject );
   else
      PMSDK_SetAttrValue( pObject, "item_rev", rev_level, pObject );
   RC = PM_Create_List( pList, pClass );
   PMSDK_SetAttrValue( pObject, "effective_date", data[5], pObject );

   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "config", confINST );

   subscribeList=(char *) malloc(2048);
   strcpy(subscribeList, NULL);
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   PMSDK_SetValue1( confINST, data[0], NULL );

   RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

   if( RC != 0 ) {
       ErrorCheck( "PMSDK_MakeCall" );
       is = -3010 ;
   } else
       is = 3010 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "config" );
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstance( confINST );
   PMSDK_DestroyInstanceList( pIList );

   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
```

IN: Required: - config = AJT
要先宣告
PMInstanceListRef* pIList;
PMInstanceRef*     confINST;
然後要實例化
pIList = PMSDK_CreateInstanceList();
confINST = PMSDK_CreateInstance();

PMSDK_AddInstance( pIList, "object", pObject );
PMSDK_AddInstance( pIList, "config", confINST );

最後SetAttrValue
PMSDK_SetValue1( confINST, data[0], NULL );

**只有這種非object.開頭的需要以這種方式進行**

其他的都可以以PMSDK_SetAttrValue的方式進行操作
如:PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );

*猜: 可能是以手動的方式新增object.config

最後請一樣要記得要做記憶體釋放
PMSDK_RemoveInstance( pIList, "config" );
PMSDK_RemoveInstance( pIList, "object" );
PMSDK_DestroyInstance( confINST );
PMSDK_DestroyInstanceList( pIList );
RC = PM_Destroy_Object( pObject ) ;


## **補充說明文件**
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/PMSDK_AddInstance.png)
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/PMSDK_SetValue1.png)
