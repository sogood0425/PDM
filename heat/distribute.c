/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>

/*---------------------------------------------------*/
/* sdk.in Input format :                             */
/*                                                   */
/*   userid passwd server configuration              */
/*                                                   */
/*---------------------------------------------------*/

char    User[10];
char    Config[10];
char    Server[10];
char    Password[10];
char*   datafile ;
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
int main( int argc, char** argv )
{
   /* Open the input file */
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf(stderr," Could not open input file [%s]\n", InputFilename );
      return( FAIL );
   }

   fscanf(InputFile,"%s %s %s %s\n", User, Password, Server, Config) ;

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

   /* Record logon data in trace file */
   fprintf( TraceFile, "Logon data :\n" );
   fprintf( TraceFile, " User   : [%s]\n", User );
   fprintf( TraceFile, " Config : [%s]\n", argv[2]);
   fprintf( TraceFile, " Server : [%s]\n", argv[1]);
   fprintf( TraceFile, "\n" );

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

   RC = LT105( datafile ) ;

   /* Log off, close environment, close trace file */
   CloseSession();

   return( 0 );
}

/*---------------------------------------------------------------------*/
/*  Function                                                           */
/*---------------------------------------------------------------------*/
int LT105( char* datafile )
{
   char*  data[]={
             "pid",
             "item",
             "ec",
             "planner",
             "approver",
             "wft",
             0 } ;
   char   temp[80] ;
   int    j ;
   int    seq = 1 ;
   int    is = 0 ;

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf( stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
   }

   while (fgets( temp, 80, InputFile ) )
   {
      if( strlen(temp) != 0 ) {
         data[0] = (char *)malloc( strlen(temp) + 1 );
         memset( data[0], 0, strlen(temp) );
         strncpy( data[0], temp, strlen(temp) );
         data[0][strlen(temp)-1] = '\0' ;
         }
      for ( j = 1; data[j]; j++ ) {
         fgets( temp, 80, InputFile ) ;
         if( strlen(temp) != 0 ) {
            data[j] = (char *)malloc( strlen(temp) + 1 );
            memset( data[j], 0, strlen(temp) );
            strncpy( data[j], temp, strlen(temp) );
            data[j][strlen(temp)-1] = '\0' ;
         }
      }

      printf( "%d %-15s%-40s%-30s ", seq++, data[0], data[1], data[2] );
      is = step1( data );
      printf( "%d\n", is );
      RC = msg( is, data );
   }

   fclose( InputFile ) ;
   return (is) ;
}

/*--------------------------------------------------------------------------*/
/*  Function                                                                */
/*--------------------------------------------------------------------------*/
int step1(char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           ItemClass = "AIDC_ItemEngineeringData";
   char*  attr_name[]={
                   "db_object_id",
                   "logical_key@item_number",
                   "il_ec_number",
                   "logical_key@loc_id",
                   "pgmid@program_id",
                   "item_type",
                   "ai_status",
                   "lai_obj@ai@eng_effty_level",
                   "lai_obj@act_eff_type",
                   "lai_obj@ai@change_action@curr_key",
                   "lai_obj@act_rev_level",
                   "lai_obj@item_status",
                   "lai_obj@ec@id",
                   "lai_obj@ec@db_object_id",
                   "lai_obj@ec@chg_control_code",
                   "disabled_actions",
                   "update_count",
                   0 } ;
   char*  attr_value[100];
   char   folder[80];
   char   tmpstr[80];
   char   tmpin[80];
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, ItemClass );

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

   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, lai_obj@loc@loc_id)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, lai_obj@mfg_seq_num)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -3000 ;
   } else {
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], attr_value[3] );
      is = step2( data, attr_value[3] );
      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*------------------------------------------------------------*/
/*  Function : check Heat treatment data exists ?             */
/*------------------------------------------------------------*/
int step2(char* data[], char* locid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_HeatTreatment";
   char*  attr_name[]={
                   "db_object_id",
                   "logical_key@item_number",   /*  1  */
                   "il_ec_number",
                   "start_tmp",
                   "final_tmp",
                   "ht_par",      /*  5  */
                   "sub_not_ht",
                   "cmmt",
                   "req_heat",
                   "alumin",      /*  9  */
                   "sol_heat_tre",
                   "aging1",
                   "aging2",
                   "non_alumin",    /*  13  */
                   "hard_tmp",
                   "aust_con",
                   "pre_hard",
                   "normalize",
                   "stablize",
                   "stress_rel",    /*  19  */
                   "ec_id",
                   "not_req_heat",
                   "logical_key@loc_id",
                   "update_count",
                   "ec_id",
                   "rclrsn",
                   "chg_by@middle_name",
                   "chg_date",
                   0 } ;
   PMInstanceListRef* pIList;
   char*  subscribeList = NULL;
   int    i ;
   int    is = 0 ;
   char*  objid ;
   char*  updt_cnt ;
   char*  ec ;

   RC = PM_Create_Object(pObject, pClass) ;

   pIList = PMSDK_CreateInstanceList();

   subscribeList=(char *) malloc(4096);
   strcpy(subscribeList, NULL);
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   PMSDK_AddInstance( pIList, "object", pObject );

   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "il_ec_number", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@item_number", data[1], pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", locid, pObject );

   RC = PMSDK_MakeCall( pClass, "cl_open_object", pIList, subscribeList);

   if ( RC < 0 ) {
      is = -3200 ;
   } else {
      ec = PMSDK_GetAttrValue( pObject, attr_name[20], pObject );
      if( strcmp(data[2],ec) == 0 ) {
         is = 3200 ;
         objid = PMSDK_GetAttrValue( pObject, attr_name[0], pObject );
         is = step4( data, objid, locid );
      } else {
         is = -3200 ;
      }
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject );

   return( is );
}

/*-----------------------------------------------------------------------*/
/*  Function : create folder                                             */
/*-----------------------------------------------------------------------*/
int step4(char* data[], char* heat_objid, char* loc_id )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Folder";
   char*  attr_name[]={
                   "db_object_id",
                   "date_last_changed",
                   "status_value",
                   "originator@name",
                   "name",
                   "description",
                   "disabled_actions",
                   "update_count",
                   0 } ;
   char*  attr_value[10];
   char   ff_id[70];
   char*  p;
   char   tmpstr[120];
   char   desc[200];
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(ff_id,data[1]);
   strcat(ff_id,data[2]);
   strcat(ff_id,"-H");

   strcpy(desc,"熱處理卷宗夾送審說明： 請至【OSM報表】選單選擇【熱處理】選項查詢相關資訊。" );
   desc[22] = 10 ;

   strcpy(tmpstr,"&=(name,'");
   strcat(tmpstr,ff_id);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "db_object_id", "" );
      RC = PM_Set_Object_Attr(pObject, "name", ff_id );
      RC = PM_Set_Object_Attr(pObject, "update_count", "0" );
      RC = PM_Set_Object_Attr(pObject, "description", desc);

      RC = PM_Save_Object( pObject);

      if( RC == 0 ) {
         is = 3310 ;
         RC = PM_Get_Object_Attr( pObject, attr_name[0], attr_value[0] );
         is = step5( data, ff_id, heat_objid, attr_value[0], "1" );
      } else {
         is = -3310 ;
         ErrorCheck("PM_Save_Object") ;
      }
   } else {
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject,attr_name[2], attr_value[2] );
      if( strcmp(attr_value[2],"3") == 0 ) {
         is = 3303 ;
      } else if( strcmp(attr_value[2],"5") == 0 ) {
         is = 3305 ;
      } else {
         is = 3399 ;
      }
   }

   RC = PM_Destroy_Object( pObject );
   RC = PM_Destroy_List( pList );

   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : save folder to Heat                                 */
/*-----------------------------------------------------------------*/
int step5(char* data[], char* ff_id, char* heat_objid, char* ff_objid,
          char* distribute_key )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_HeatTreatment";
   char*  attr_name[]={
                 "db_object_id",
                 "folder@db_object_id",
                 0 } ;
   char*  attr_value[90];
   int    ListSize;
   int    i,j;
   int    is =  0 ;
   char   tmpstr[80];
   PMInstanceListRef* pIList;
   char*  objectSubscribesString ;
   char*  subscribeList = NULL;

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();

   PMSDK_SetAttrValue( pObject, "db_object_id", heat_objid, pObject );
   PMSDK_SetAttrValue( pObject, "folder@db_object_id", ff_objid, pObject );
   PMSDK_AddInstance( pIList, "object", pObject );

   subscribeList=(char *) malloc(8192);
   strcpy(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   RC = PMSDK_MakeCall( pClass, "cl_update_folder_oid", pIList, subscribeList );

   if( RC >= 0 ) {
      is = 3320 ;
      if( strcmp(distribute_key,"1")==0 || strcmp(distribute_key,"2")==0 )
         is = step9( data, ff_objid, ff_id );
   } else {
      is = -3320 ;
      ErrorCheck("PMSDK_MakeCall") ;
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : Distribute folder                                   */
/*-----------------------------------------------------------------*/
int step6(char* ff_objid, char* ff_id, char* wft )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Folder";
   char*  attr_name[]={
                 "db_object_id",
                 "active_recip@action",
                 "active_recip@db_object_id",
                 "active_recip@due_date",
                 "active_recip@due_time",
                 "active_recip@recipient_name",
                 "date_created",
                 "date_last_changed",
                 "description",
                 "enabled_actions",
                 "i_sec_cat_code",
                 "i_sec_level_code",
                 "multiple_active_recips",
                 "name",
                 "assign_user",
                 "originator@name",
                 "status_value",
                 "stop_distribution",
                 "update_count",
                 "valid_responses",
                 "wft_name",
                 "workflow_template@pdlname",
                 "x_sec_cat_code",
                 "x_sec_level_code",
                 0 } ;
   char*  attr_value[100];
   char*  wft_cond ;
   char   tmpstr[80];
   PMInstanceListRef* pIList;
   PMInstanceRef*     locationsINST;
   PMInstanceRef*     onlineINST;
   PMInstanceRef*     sys_pwINST;
   char*  objectSubscribesString ;
   char*  subscribeList = NULL;
   int    RCC, RCCC ;
   int    i ;
   int    is = 0 ;

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();

   PMSDK_SetAttrValue( pObject, "db_object_id", ff_objid, pObject );
   PMSDK_SetAttrValue( pObject, "name", ff_id, pObject );
   PMSDK_SetAttrValue( pObject, "assign_user", "", pObject );

   PMSDK_AddInstance( pIList, "folder", pObject );

   subscribeList=(char *) malloc(8192);
   strcpy(subscribeList,"folder@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   RC = PMSDK_MakeCall( pClass, "cl_pre_assign_wft", pIList, subscribeList );
   if( RC == 0 ) {
      is = 3330 ;
      PMSDK_SetAttrValue( pObject, "wft_name", wft, pObject );
      RCC = PMSDK_MakeCall( pClass, "cl_assign_wft", pIList, subscribeList );
      if( RCC == 0 ) {
         is = 3340 ;
         RCCC = PMSDK_MakeCall( pClass, "cl_distribute_heat", pIList, subscribeList );
         if( RCCC == 0 ) {
            is = 3350 ;
         } else {
            is = -3350 ;
            ErrorCheck("PMSDK_MakeCall") ;
         }
      } else {
         is = -3340 ;
         ErrorCheck("PMSDK_MakeCall") ;
      }
   } else {
      is = -3330 ;
      ErrorCheck("PMSDK_MakeCall") ;
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }

   PMSDK_RemoveInstance( pIList, "folder" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function                                                       */
/*-----------------------------------------------------------------*/
int step9(char* data[], char* ff_objid, char* ff_id )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           ItemClass = "WorkFlowTemplate";
   char*  attr_name[]={
                   "db_object_id",
                   "description",
                   "coordinator@pdi_system_id",
                   "pdlname",
                   0 } ;
   char*  wft;
   int    ListSize;
   int    i,j,k;
   int    is = 0 ;

   RC = PM_Create_List( pList, ItemClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   RC = PM_Set_Clause( pList, "Where", "&LIKE(pdlname, 'PDM INTERNAL REVIEW%')" );
   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize == 0 ) {
      is = -7000 ;
   } else {
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], wft );
      is = step6( ff_objid, ff_id, wft );
      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
