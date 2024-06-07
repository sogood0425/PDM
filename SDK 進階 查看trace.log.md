> **IN: Method Call Class - AIDCB600.  Method - FA125**
> IN: ListOpenIn Select clause:
> IN: StringListElem:  "act_release_time"
> IN: StringListElem:  "release_date"
> IN: StringListElem:  "act_effective_date"
> IN: StringListElem:  "ALOO_planner"
> IN: StringListElem:  "ALOO_accept_date"
> IN: StringListElem:  "OSM_finish_date"
> IN: StringListElem:  "OSM_planner"
> IN: StringListElem:  "act_finish_date"
> IN: StringListElem:  "ec_pre_acc"
> IN: StringListElem:  "x_sec_level_code"
> IN: StringListElem:  "x_sec_cat_code"
> IN: StringListElem:  "db_object_id"
> IN: StringListElem:  "plan_finish_date"
> IN: StringListElem:  "type_code.curr_key"
> IN: StringListElem:  "priority.curr_key"
> IN: StringListElem:  "coordinator.person_identifier"
> IN: StringListElem:  "coordinator.last_name"
> IN: StringListElem:  "coordinator.first_name"
> IN: StringListElem:  "coordinator.middle_name"
> IN: StringListElem:  "coordinator.dept"
> IN: StringListElem:  "initiator.last_name"
> IN: StringListElem:  "initiator.first_name"
> IN: StringListElem:  "initiator.middle_name"
> IN: StringListElem:  "initiator.dept"
> IN: StringListElem:  "cust_req"
> IN: StringListElem:  "class.curr_key"
> IN: StringListElem:  "projcode_ec.proj_code"
> IN: StringListElem:  "programid.program_id"
> IN: StringListElem:  "initiator.person_identifier"
> IN: StringListElem:  "chg_control_code.curr_key"
> IN: StringListElem:  "engr_lab.loc_id"
> IN: StringListElem:  "design_lab.loc_id"
> IN: StringListElem:  "status.curr_key"
> IN: StringListElem:  "desc"
> IN: StringListElem:  "id"
> IN: StringListElem:  "release_flag"
> IN: StringListElem:  "virtual_pre_rel"
> IN: StringListElem:  "virtual_distr"
> IN: StringListElem:  "virtual_netting"
> IN: StringListElem:  "virtual_is_mec"
> IN: StringListElem:  "disabled_actions"
> IN: StringListElem:  "update_count"
> IN: StringListElem:  "db_object_id"
> IN: ListOpenIn Expressions clause:
> IN: **ListOpenIn Where clause:**
> IN: **StringListElem:  "&<>(programid.program_id,'ICT')"**
> IN: **StringListElem:  "&LIKE(id, 'MPEQ2404059%')"**
> IN: ListOpenIn Order By clause:
> IN: StringListElem:  "&ASC(id)"
> IN: StringListElem:  "programid.program_id"
> IN: ListOpenIn Requesting 50 rows.
> 
> **AVNRPCIF -- client invoking AIDC_EngineeringChange(AIDCB600).cl_open_list(000fa125)**
> @@@@@@@ in AIDC_EngineeringChange's cl_open_list.
> @@@@@@@ validate_where_clause's local_rc=0
> &&&&& in AIDC_DYN_STREAM2_CLS's restart
> &&&&& to start_db_process
> &&&& in AIDC_DYN_STREAM2_CLS's set_stream_element2

根據上述trace.log可以發現是要先從cl_open_list [AIDC_EngineeringChange]
然後所需要的where條件是:
StringListElem: "&<>(programid.program_id,'ICT')"
StringListElem: "&LIKE(id, 'MPEQ2404059%')"
但DJ有說過"&<>(programid.program_id,'ICT')"  這個不用

```
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
因此這段程式碼是從trace.log對應的程式碼

接下來是直接做AIDC_LocationAffectedItem的cl_save
在trace.log的部分如下

> **IN: Method Call Class - AIDCB701.  Method - FA12B
> IN: object.db_object_id[0] = ""
> IN: object.update_count[0] = "0"
> IN: object.ec.id[0] = "MPEQ2404059"
> IN: object.item_number[0] = "TEST123123123"
> IN: object.loc.loc_id[0] = "5100"
> IN: object.act_eff_type[0] = "PU"
> IN: object.act_eff_date[0] = ""
> IN: object.db_object_id[0] = ""
> IN: object.ec.id[0] = "MPEQ2404059"
> IN: object.ir_part[0] = "No"
> IN: object.item_number[0] = "TEST123123123"
> IN: object.loc.loc_id[0] = "5100"**
> IN: Subscribe: - object.accept_by.middle_name
> IN: Subscribe: - object.accept_date
> IN: Subscribe: - object.act_disp_uom.virtual_real
> IN: Subscribe: - object.act_disp_uom.virtual_unit_id
> IN: Subscribe: - object.act_disposition.curr_key
> IN: Subscribe: - object.act_eff_date
> IN: Subscribe: - object.act_eff_date_AIDC
> IN: Subscribe: - object.act_eff_type
> IN: Subscribe: - object.act_re_run_FAI
> IN: Subscribe: - object.act_re_run_IR_test
> IN: Subscribe: - object.act_rev_level
> IN: Subscribe: - object.appc_rdate
> IN: Subscribe: - object.change_act_lai
> IN: Subscribe: - object.change_by.middle_name
> IN: Subscribe: - object.change_date
> IN: Subscribe: - object.chg_plan_fini_date
> IN: Subscribe: - object.create_by.middle_name
> IN: Subscribe: - object.create_by.pdi_system_id
> IN: Subscribe: - object.create_date
> IN: Subscribe: - object.create_time
> IN: Subscribe: - object.db_object_id
> IN: Subscribe: - object.demander.middle_name
> IN: Subscribe: - object.demander.pdi_system_id
> IN: Subscribe: - object.ec.id
> IN: Subscribe: - object.enabled_actions
> IN: Subscribe: - object.finish_days
> IN: Subscribe: - object.ir_part
> IN: Subscribe: - object.item_engg_status
> IN: Subscribe: - object.item_name
> IN: Subscribe: - object.item_number
> IN: Subscribe: - object.item_status.curr_key
> IN: Subscribe: - object.loc.loc_id
> IN: Subscribe: - object.mrouting_type
> IN: Subscribe: - object.pid
> IN: Subscribe: - object.plan_finish_date
> IN: Subscribe: - object.remark
> IN: Subscribe: - object.update_count
> IN: Subscribe: - object.wbs
> IN: Subscribe: - object.x_sec_cat_code
> IN: Subscribe: - object.x_sec_level_code
> IN: Subscribe: - object.disabled_actions
> 
> **AVNRPCIF -- client invoking AIDC_LocationAffectedItem(AIDCB701).cl_save(000fa12b)**
> @@@@@@@@@ in AIDC_LocationAffectedItem's cl_save
> @@@@@@@@@ reorder_save_list's local_rc=0
> @@@@@@@@@ save_list has 1 lai
> @@@@@@@@@ located db_object_id=
> @@@@@@@@@ mode=N
> @@@@@@@@@ find_instance's local_rc=0
> @@@@@@@@@ authorized, local_rc=0
> @@@@@@@@@ to validate_instance
> @@@@@@@ in AIDC_LocationAffectedItem's validate_instance, mode=[N]
> @@@@@@@ current user=[plmadm01]
> @@@@@@@1 objid=0021AC0D6450626600800186008A0005, mfg_seq_num=0

在trace.log的部分並沒有明確要求requested的部分
因此把需要的填上就好

```
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
   RC = PM_Set_Object_Attr(pObject, "act_re_run_IR_test", "N" ); 這行一定要
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

要特別小心雖然在trace.log上沒有明確要求Required的欄位
但在PDM上綠色框的部分為act_re_run_IR_test 這個是必要欄位，那麼就在PM_Set_Object_Attr要把它存進去
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/LAI2.png)


最後複習一下邏輯順序
先判斷EC有無 ECnumber 所以要先Openlist AIDC_EngineeringChange (一定要有)

![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/LAI.png)

接著才能做LAI內的新增object

![image](https://raw.githubusercontent.com/sogood0425/PDM/main/advanced_images/LAI1.png)