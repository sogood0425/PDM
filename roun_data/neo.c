/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>
int LT999(char*);
int msg(int,char*[]);
int step1(char*[]);
int step2(char*[],char*,char*,char*,char*,char*);
int step3(char*[],char*,char*,char*,char*,char*,char*,char*,char*,char*);
int step4(char*[],char*,char*,char*,char*,char*,char*,char*,char*,char*,char*,char*);
int step5(char*[],char*,char*,char*,char*,char*,char*,char*,char*,char*,char*,char*,char*,char*);

/*---------------------------------------------------*/
/* Globals / Default values                          */
/*   - You may want to change the default logon      */
/*     variables here to match your installation.    */
/*   - The function "SetParms" will prompt you for   */
/*     any changes to the logon values.  If this     */
/*     function causes a problem then comment out    */
/*     the call to it located in the "main" routine. */
/*   - If a logon variable is set to "" or NULL,     */
/*     then your pmv3.ini logon values will be used. */
/*---------------------------------------------------*/

char    User[10];
char    Config[10];
char    Server[10];
char    Password[10];
char*   datafile;
char*   TraceFilename = "sdk.log";
char*   InputFilename = "sdk.in";
char*   OutputFilename = "sdk.out";
FILE*   TraceFile;
FILE*   InputFile;
FILE*   OutputFile;
int     RC;
int     LoggedOn = 0;
int     EnvOpen = 0;

/*---------------------------------------------------*/
/* Main                                              */
/*---------------------------------------------------*/
int main( int argc, char** argv ) {

   /* Open the input file */
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf(stderr," Could not open input file [%s]\n", InputFilename );
      return( FAIL );
   }

   fscanf(InputFile,"%s %s %s %s",
          User, Password, Server, Config ) ;

   /* Open the output file */
   OutputFile = NULL;
   OutputFile = fopen( OutputFilename, "w" );
   if( !OutputFile ) {
      fprintf( stderr," Could not open output file [%s]\n", OutputFilename );
      return( FAIL );
   }

   /* Open the trace file */
   TraceFile = NULL;
   TraceFile = fopen( TraceFilename, "w" );
   if( !TraceFile ) {
      fprintf( stderr, " Could not open trace file [%s]\n", TraceFilename );
      return( FAIL );
   }

   /* Open the PM environment */
   RC = PM_Open_Env();
   if( RC >= 0 )
      EnvOpen = 1;
   ErrorCheck( "PM_Open_Env" );

   RC = PM_Logon( User, Password, argv[1], argv[2] );

   if( RC >= 0 )
      LoggedOn = 1;
   ErrorCheck( "PM_Logon" );

   if( argc == 3 )
      datafile = "input.data" ;
   else
      datafile = argv[3] ;

   RC = LT999( datafile ) ;

   CloseSession();

   return( RC );
}

/*---------------------------------------------------------------------*/
/*  Function                                                           */
/*---------------------------------------------------------------------*/
int LT999( char* datafile )
{
   FILE*  InputFile ;
   char*  data[]={
                 "pdmid",
                 "item_number",
                 "ec_number",
                 "modif_day",
                 "modif_by",
                 "mod_description",
                 0 } ;
   int    j ;
   int    seq = 1 ;
   int    is = 0 ;
   char   temp[1024] ;

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf( stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
   }

   while (fgets( temp, 1024, InputFile ) )
   {
      RC = 0 ;
      if( strlen(temp) != 0 ) {
         data[0] = (char *)malloc( strlen(temp) + 1 );
         memset( data[0], 0, strlen(temp) );
         strncpy( data[0], temp, strlen(temp)-1 );
      } else
         break;

      for ( j = 1; data[j]; j++ ) {
         fgets( temp, 1024, InputFile ) ;
         if( strlen(temp) != 0 ) {
            data[j] = (char *)malloc( strlen(temp) + 1 );
            memset( data[j], 0, strlen(temp) );
            if( temp[strlen(temp)-1] == 10 )
               strncpy( data[j], temp, strlen(temp)-1 );
            else {
               strncpy( data[j], temp, strlen(temp) );
               data[j][strlen(temp)] = '\0' ;
            }
         }
      }

      /*printf("%d pdmid=[%s] item_number=[%s] ec_number=[%s] modif_day=[%s] modif_by=[%s] mod_description=[%s]",seq++, data[0], data[1], data[2], data[3], data[4], data[5]);*/
      is = step1( data );
      printf( "%d %s %s %d\n",seq++,data[0],data[1], is );

      RC = msg( is, data );
   }

   fclose( InputFile );
   return (is) ;
}

/*-------------------------------------------------------------------*/
/*  Function : Check Item List exists ?                              */
/*-------------------------------------------------------------------*/
int step1(char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Person";
   char*  attr_name[]={
            "db_object_id",
            "dept",
            "person_num",
            "middle_name",
            "first_name",
            "last_name",
            "per_dpt_phone",
            "user_id@system_id",
            "pdi_network",
            "pdi_node",
            "active_flag",
            "resp_type",
            "email_id",
            "email_type",
                    0 } ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;
   char*  first_name ;
   char*  middle_name ;
   char*  last_name;
   char*  pdi_system_id ;
   char*  dept ;
   char*  plmamd01 = "450001";

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr, "&=(user_id@system_id,'");
   strcat(tmpstr, data[4]);
   strcat(tmpstr, "')");
   RC = PM_Set_Clause(pList, "Where", tmpstr);

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -1000 ;
   } else {
      is = 1000 ;
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject, attr_name[4], first_name );
         RC = PM_Get_Object_Attr( pObject, attr_name[3], middle_name );
         RC = PM_Get_Object_Attr( pObject, attr_name[5], last_name );
         RC = PM_Get_Object_Attr( pObject, attr_name[7], pdi_system_id );
         RC = PM_Get_Object_Attr( pObject, attr_name[1], dept );

      /*printf("first_name = %s \n", first_name );
      printf("middle_name = %s \n", middle_name );
      printf("last_name = %s \n", last_name );
      printf("pdi_system_id = %s \n", pdi_system_id );
      printf("dept = %s \n", dept );
      printf("\n" );*/
      }

      is = step2(data, first_name, middle_name, last_name ,pdi_system_id, dept);
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : Check Item List exists ?                              */
/*-------------------------------------------------------------------*/
int step2(char* data[],char* first_name,char* middle_name,char* last_name,char* pdi_system_id,char* dept )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemEngineeringData";
   char*  attr_name[]={
                    "db_object_id",
                    "lai_obj@mfg_seq_num",
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
   char*  locid ;
   char*  mfg_seq_num;
   char*  name;

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
         RC = PM_Get_Object_Attr( pObject, attr_name[1], mfg_seq_num );
         RC = PM_Get_Object_Attr( pObject, attr_name[4], locid );
         RC = PM_Get_Object_Attr( pObject, attr_name[7], name );

      /*printf("db_object_id = %s \n", db_object_id );
      printf("mfg_seq_num = %s \n", mfg_seq_num );
      printf("locid = %s \n", locid );
      printf("name = %s \n", name );
      printf("\n" );*/
      }

      is = step3(data, first_name, middle_name, last_name ,pdi_system_id, dept, db_object_id, locid, mfg_seq_num,name);
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : Get lai_obj@db_object_id                           */
/*-------------------------------------------------------------------*/

int step3(char* data[], char* first_name,char* middle_name,char* last_name ,char* pdi_system_id,char* dept,char* db_object_id,char* locid,char* mfg_seq_num,char* name)
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
   RC = PM_Set_Object_Attr( pObject, "logical_key@item_number", data[1] );
   RC = PM_Set_Object_Attr( pObject, "il_ec_number", data[2]  );
   RC = PM_Set_Object_Attr( pObject, "logical_key@loc_id", locid );

   for( j=0; attr_name[j]; j++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[j] );

   RC = PM_Open_Object( pObject );

   RC = PM_Get_Object_Attr( pObject, attr_name[3], lai_obj_db_object_id );
   RC = PM_Get_Object_Attr( pObject, attr_name[8], rev_level );
   /*printf("lai_obj_db_object_id = %s \n", lai_obj_db_object_id );
   printf("rev_level = %s \n", rev_level );*/

   is = step4(data, first_name, middle_name, last_name ,pdi_system_id, dept, db_object_id, locid, mfg_seq_num,name ,lai_obj_db_object_id, rev_level);

   RC = PM_Destroy_Object( pObject );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : EC IRD add                          */
/*-------------------------------------------------------------------*/

int step4(char* data[], char* first_name,char* middle_name,char* last_name ,char* pdi_system_id,char* dept,char* db_object_id,char* locid,char* mfg_seq_num,char* name,char* lai_obj_db_object_id,char* rev_level)
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemRoutingData";
   char*  attr_name[]={
                   "db_object_id",
                   "update_count@item_number",
                   "disabled_actions",
                   "logical_key@routid",
                   "logical_key@item_number",
                   "logical_key@loc_id",
                   "rout_type_ind@curr_key",
                   "aidc_routid",
                   "ec_number",
                   "tool_id",
                   "item_physical_status",
                   "disposition_code",
                   "revision_level",
                   "material_description",
                   "memo_description",
                   "master_routing_type",
                   "revision_date",
                   "work_order_loc",
                   "tacc",
                   "ma_routing",
                   "plan_finish_date",
                   "revised_by@dept",
                   "revised_by@pdi_system_id",
                   "revised_by@middle_name",
                   "revised_by@first_name",
                   "revised_by@last_name",
                   "folder@name",
                   "folder@status_value",
                   "description",
                   "plan_appd_no",
                   "client_approve_date",
                   "req_cust_review",
                   "rout_change_seq",
                   "fpq_control",
                   "fpq_typical_part",
                   "req_change_by@middle_name",
                   "req_change_date",
                   "change_by@middle_name",
                   "change_date",
                   "change_time",
                   0 } ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;
   char*  curr_key;
   char*  routid;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   /*strcpy(tmpstr,"&BETWEEN(insert_by, 0, ");
    strcat(tmpstr,mfg_seq_num);
    strcat(tmpstr,"')");
    RC = PM_Set_Clause( pList, "Where", tmpstr );

    strcpy(tmpstr,"&>(extract_by, ");
    strcat(tmpstr,mfg_seq_num);
    strcat(tmpstr,"')");
    RC = PM_Set_Clause( pList, "Where", tmpstr );*/
   strcpy(tmpstr,"&=(ec_number, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(logical_key.loc_id, '");
   strcat(tmpstr,locid);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(logical_key.item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   /*printf("***%s \n", data[1] );*/

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   /*for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject, attr_name[6], curr_key );
         RC = PM_Get_Object_Attr( pObject, attr_name[3], routid );

      printf("curr_key = %s \n", curr_key );
      printf("routid = %s \n", routid );
      }*/



   if( ListSize == 0 ) {
      is = -3000 ;
      /*printf("is = %d \n", is );*/
      is = step5(data, first_name, middle_name, last_name ,pdi_system_id, dept, db_object_id, locid, mfg_seq_num,name,lai_obj_db_object_id, rev_level, curr_key, routid);
   } else {
      is = 3000 ;
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : EC IR1o4D add                                          */
/*-----------------------------------------------------------------*/
int step5(char* data[], char* first_name,char* middle_name,char* last_name ,char* pdi_system_id,char* dept,char* db_object_id,char* locid,char* mfg_seq_num,char* name,char* lai_obj_db_object_id,char* rev_level ,char* curr_key ,char* routid)
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemRoutingData";
   char*  attr_name[]={
                   "db_object_id",
                   "update_count@item_number",
                   "disabled_actions",
                   "logical_key@routid",
                   "logical_key@item_number",
                   "logical_key@loc_id",
                   "rout_type_ind@curr_key",
                   "aidc_routid",
                   "ec_number",
                   "tool_id",
                   "item_physical_status",
                   "disposition_code",
                   "revision_level",
                   "material_description",
                   "memo_description",
                   "master_routing_type",
                   "revision_date",
                   "work_order_loc",
                   "tacc",
                   "itm_name",
                   "ma_routing",
                   "plan_finish_date",
                   "revised_by@dept",
                   "revised_by@pdi_system_id",
                   "revised_by@middle_name",
                   "revised_by@first_name",
                   "revised_by@last_name",
                   "folder@name",
                   "folder@status_value",
                   "description",
                   "plan_appd_no",
                   "client_approve_date",
                   "req_cust_review",
                   "rout_change_seq",
                   "fpq_control",
                   "fpq_typical_part",
                   "req_change_by@middle_name",
                   "req_change_date",
                   "change_by@middle_name",
                   "change_date",
                   "change_time",
                   0 } ;
   char*  item ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;
   char*  subscribeList = NULL;
   PMInstanceListRef* pIList;
   PMInstanceRef*     ecINST;
   PMInstanceRef*     toolINST;
   PMInstanceRef*     laiINST;

   strcpy(tmpstr,data[1]);
   strcat(tmpstr,"2");
   /*printf("routid = %s \n", tmpstr );*/

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();
   ecINST = PMSDK_CreateInstance();
   toolINST = PMSDK_CreateInstance();
   laiINST  = PMSDK_CreateInstance();

   PMSDK_SetAttrValue( pObject, "program_id", data[0], pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@item_number", data[1], pObject );
   PMSDK_SetAttrValue( pObject, "ec_number", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "revision_date", data[3], pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@pdi_system_id", pdi_system_id, pObject );
   PMSDK_SetAttrValue( pObject, "description", data[5], pObject );

   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", locid, pObject );
   PMSDK_SetAttrValue( pObject, "rout_type_ind@curr_key", "2", pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@dept", dept, pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@first_name", first_name, pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@middle_name", middle_name, pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@last_name", last_name, pObject );
   PMSDK_SetAttrValue( pObject, "tool_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@routid", tmpstr, pObject );
   PMSDK_SetAttrValue( pObject, "il_ec_number", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "aidc_routid", data[1], pObject );
   PMSDK_SetAttrValue( pObject, "itm_name", name, pObject );



   if( strlen(rev_level)==0 )
      PMSDK_SetAttrValue( pObject, "revision_level", "-", pObject );
   else
      PMSDK_SetAttrValue( pObject, "revision_level", rev_level, pObject );
   RC = PM_Create_List( pList, pClass );
   PMSDK_SetAttrValue( pObject, "work_order_loc", locid, pObject );

   PMSDK_SetAttrValue( pObject, "ma_routing", "N", pObject );
   PMSDK_SetAttrValue( pObject, "tacc", "N", pObject );




   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "il_ec_number", ecINST );
   PMSDK_AddInstance( pIList, "is_tooling", toolINST );
   PMSDK_AddInstance( pIList, "oid_lai", laiINST );
   PMSDK_SetValue1( ecINST, data[2], ecINST );
   PMSDK_SetValue1( toolINST, "0", toolINST );
   PMSDK_SetValue1( laiINST, lai_obj_db_object_id, laiINST );

   ErrorCheck("PMSDK_SetValue1");
   /*subscribeList=(char *) malloc(2048);
   strcpy(subscribeList, NULL);
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");*/

   RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

   if( RC != 0 ) {
      fprintf( TraceFile, "step5 : %s\t%s\n", data[1], data[2] );
      ErrorCheck( "PMSDK_MakeCall" );
      is = -3010 ;
   } else
      is = 3010 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "il_ec_number" );
   PMSDK_RemoveInstance( pIList, "is_tooling" );
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstance( ecINST );
   PMSDK_DestroyInstance( toolINST );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
