> IN: Method Call Class - AIDCPERS.  Method - **FA125**
> IN: ListOpenIn Select clause:
> IN: StringListElem:  "email_type"
> IN: StringListElem:  "email_id"
> IN: StringListElem:  "resp_type"
> IN: StringListElem:  "active_flag"
> IN: StringListElem:  "person_identifier"
> IN: StringListElem:  "pdi_network"
> IN: StringListElem:  "pdi_node"
> IN: StringListElem:  "pdi_system_id"
> IN: StringListElem:  "alternate_name"
> IN: StringListElem:  "delegate"
> IN: StringListElem:  "dept"
> IN: StringListElem:  "per_dpt_phone"
> IN: StringListElem:  "middle_name"
> IN: StringListElem:  "first_name"
> IN: StringListElem:  "last_name"
> IN: StringListElem:  "person_num"
> IN: StringListElem:  "disabled_actions"
> IN: StringListElem:  "update_count"
> IN: StringListElem:  "db_object_id"
> IN: ListOpenIn Expressions clause:
> **IN: ListOpenIn Where clause:**
> **IN: StringListElem:  "&LIKE(person_num, '617816%')"**
> IN: ListOpenIn Order By clause:
> IN: StringListElem:  "person_num"
> IN: ListOpenIn Requesting 50 rows.
> 
> **AVNRPCIF** -- client invoking **AIDC_Person**(AIDCPERS).**cl_open_list(000fa125**)
> &&&&& in AIDC_DYN_STREAM2_CLS's restart
> &&&&& to start_db_process
> &&&& in AIDC_DYN_STREAM2_CLS's set_stream_element2
> &&&&& exit AIDC_DYN_STREAM2_CLS's restart
> Response start:
> OUT: **ListOpenOut:  1 total rows in bound list 00C6BA0DE30B61660080016E00FC0005.**
>      Disabled Actions:
> OUT: StringListElem:  "fa156"
> COLUMN NAME: object.db_object_id
> COLUMN NAME: object.update_count
> COLUMN NAME: object.person_num
> COLUMN NAME: object.last_name
> COLUMN NAME: object.first_name
> COLUMN NAME: object.middle_name
> COLUMN NAME: object.per_dpt_phone
> COLUMN NAME: object.dept
> COLUMN NAME: object.delegate
> COLUMN NAME: object.alternate_name
> COLUMN NAME: object.active_flag
> COLUMN NAME: object.resp_type
> COLUMN NAME: object.email_id
> COLUMN NAME: object.email_type
> COLUMN NAME: object.user_id.system_id
> COLUMN NAME: object.user_id.system_node
> COLUMN NAME: object.user_id.network
> COLUMN NAME: object.pdi_system_id
> COLUMN NAME: object.pdi_node
> COLUMN NAME: object.pdi_network
> COLUMN NAME: object.person_identifier
> OUT: object.db_object_id[0] = "00436A0167B2D5650080012700CE0005" UIFlag=D
> OUT: object.update_count[0] = "1" UIFlag= 
> OUT: object.person_num[0] = "617816" UIFlag=R
> OUT: object.last_name[0] = "Huang" UIFlag=R
> OUT: object.first_name[0] = "Sin-Siou" UIFlag= 
> OUT: object.middle_name[0] = "黃信修" UIFlag=R
> OUT: object.per_dpt_phone[0] = "3410" UIFlag= 
> OUT: object.dept[0] = "450" UIFlag=R
> OUT: object.delegate[0] = "0" UIFlag=N
> OUT: object.alternate_name[0] = "" UIFlag= 
> OUT: object.active_flag[0] = "1" UIFlag= 
> OUT: object.resp_type[0] = "" UIFlag= 
> OUT: object.email_id[0] = "SIN-SIOU HUANG" UIFlag= 
> OUT: object.email_type[0] = "1" UIFlag= 
> OUT: object.user_id.system_id[0] = "n617816" UIFlag= 
> OUT: object.user_id.system_node[0] = "pmserver" UIFlag= 
> OUT: object.user_id.network[0] = "TCPIP" UIFlag= 
> OUT: object.pdi_system_id[0] = "n617816" UIFlag=D
> OUT: object.pdi_node[0] = "pmserver" UIFlag=D
> OUT: object.pdi_network[0] = "TCPIP" UIFlag=D
> OUT: object.person_identifier[0] = "n617816" UIFlag= 
> OUT: ListCloseOut:  Returning 811440208
> Response end:

FA125是要準備cl_open_list了，所以這個很好拿來當作關鍵字來搜尋
可以看到這裡是去做模糊查詢person_sum為617816開頭的人
IN: ListOpenIn Where clause:
IN: StringListElem: "&LIKE(person_num, '617816%')"

AVNRPCIF – client invoking AIDC_Person(AIDCPERS).cl_open_list(000fa125)
AIDC_Person可以看到是在AIDC_Person這個class下進行的
OUT: ListOpenOut: 1 total rows in bound list 00C6BA0DE30B61660080016E00FC0005.
代表是找到一個人 1 total rows (很正常，因為員工帳號基本上不會重複)

> IN: Method Call Class - AIDCPERS.  Method - 3110
> IN: Required: - object.db_object_id = 
> IN: Required: - object.update_count = 0
> IN: Required: - object.person_num = TEST999
> IN: Required: - object.person_identifier = 
> IN: IfDirty: - object.dept = 450
> IN: IfDirty: - object.last_name = Huang
> IN: IfDirty: - object.middle_name = test
> IN: IfDirty: - object.person_num = TEST999
> IN: Subscribe: - object.active_flag
> IN: Subscribe: - object.alternate.db_object_id
> IN: Subscribe: - object.alternate.person_num
> IN: Subscribe: - object.alternate_name
> IN: Subscribe: - object.db_object_id
> IN: Subscribe: - object.delegate
> IN: Subscribe: - object.dept
> IN: Subscribe: - object.disabled_actions
> IN: Subscribe: - object.email_id
> IN: Subscribe: - object.email_type
> IN: Subscribe: - object.enabled_actions
> IN: Subscribe: - object.first_name
> IN: Subscribe: - object.i_sec_cat_code
> IN: Subscribe: - object.i_sec_level_code
> IN: Subscribe: - object.il_action
> IN: Subscribe: - object.last_name
> IN: Subscribe: - object.middle_name
> IN: Subscribe: - object.name
> IN: Subscribe: - object.pdi_network
> IN: Subscribe: - object.pdi_node
> IN: Subscribe: - object.pdi_system_id
> IN: Subscribe: - object.per_dpt_phone
> IN: Subscribe: - object.person_identifier
> IN: Subscribe: - object.person_num
> IN: Subscribe: - object.resp_type
> IN: Subscribe: - object.saved_batch_jobname
> IN: Subscribe: - object.update_count
> IN: Subscribe: - object.user_id.network
> IN: Subscribe: - object.user_id.system_id
> IN: Subscribe: - object.user_id.system_node
> IN: Subscribe: - object.x_sec_cat_code
> IN: Subscribe: - object.x_sec_level_code
> 
> AVNRPCIF -- client invoking AIDC_Person(AIDCPERS).cl_save(00003110)
> #### in AIDC_Person's object_find
> #### to locate one of person_num, pdi_system_id, person_identifier
> #### attr_value=TEST999
> ***** in AIDC_Network's connect_and_find, dbid_in=5, in_net=TCPIP, in_node=pmserver
> ***** use_dns_for_clientip=0, usr_str=plmadm01
> *****1 node_str1=pmserver
> ***** myaddr == INADDR_NONE, not an IP address, must be a hostname.
> *****2 node_str1=pmserver
> ***** is an IP address? ipaddr=0
> ***** net_str=TCPIP, node_str1=pmserver
> ***** sql_stmt=SELECT OBJID FROM PM.AVNVCSMNCT WHERE NET_TO_T = &PARM_MARKER() AND NODE_TO_T = &PARM_MARKER()
> ***** sql_code=0
> ***** dynamically_add_client=1
> ***** desc_str=[DESC] dynamic_description=[Dynamically added]
> Response start:
> OUT: object.active_flag="1" UIFlag=N
> OUT: object.alternate_name="" UIFlag=N
> OUT: object.db_object_id="0025E7096B0F61660080012700FC0005" UIFlag=D
> OUT: object.delegate="0" UIFlag=N
> OUT: object.dept="450" UIFlag=R
> OUT: object.disabled_actions="fa156" UIFlag=N
> OUT: object.email_id="" UIFlag=N
> OUT: object.email_type="" UIFlag=N
> OUT: object.enabled_actions="fa120,fa121,fa124,fa125,fa126,fa127,fa129,fa12b,fa11b,fa11c,fa11e,fa11f" UIFlag=N
> OUT: object.first_name="" UIFlag=N
> OUT: object.i_sec_cat_code="0" UIFlag=N
> OUT: object.i_sec_level_code="0" UIFlag=N
> OUT: object.il_action="N" UIFlag=N
> OUT: object.last_name="Huang" UIFlag=R
> OUT: object.middle_name="test" UIFlag=R
> OUT: object.name="testHuang" UIFlag=N
> OUT: object.pdi_network="" UIFlag=N
> OUT: object.pdi_node="" UIFlag=N
> OUT: object.pdi_system_id="" UIFlag=N
> OUT: object.per_dpt_phone="" UIFlag=N
> OUT: object.person_identifier="" UIFlag=N
> OUT: object.person_num="TEST999" UIFlag=R
> OUT: object.resp_type="" UIFlag=N
> OUT: object.saved_batch_jobname="" UIFlag=N
> OUT: object.update_count="0" UIFlag=N
> OUT: object.user_id.network="" UIFlag=N
> OUT: object.user_id.system_id="" UIFlag=N
> OUT: object.user_id.system_node="" UIFlag=N
> OUT: object.x_sec_cat_code="DEFAULT" UIFlag=N
> OUT: object.x_sec_level_code="UNCLASSIFIED" UIFlag=N
> Response end:

這裡可以看到在cl_save之前有幾項東西要做
> IN: Required: - object.db_object_id = 
> IN: Required: - object.update_count = 0
> IN: Required: - object.person_num = TEST999
> IN: Required: - object.person_identifier = 
> IN: IfDirty: - object.dept = 450
> IN: IfDirty: - object.last_name = Huang
> IN: IfDirty: - object.middle_name = test
> IN: IfDirty: - object.person_num = TEST999
> IN: Subscribe: - object.active_flag
> IN: Subscribe: - object.alternate.db_object_id
> IN: Subscribe: - object.alternate.person_num
> IN: Subscribe: - object.alternate_name
> IN: Subscribe: - object.db_object_id

在Input的地方有Required、IfDirty、Subscribe這三種

Required是PDM內部要求一定要填上的資料，如果不填系統會不給過(綠色部分)
![image](https://raw.githubusercontent.com/sogood0425/PDM/main/base_images/Person.png)

IfDirty則是你選擇性可以多填上的資料，原則上IfDirty>=Required，不可以比Required少

Subscribe是其他剩下沒使用到的欄位名稱