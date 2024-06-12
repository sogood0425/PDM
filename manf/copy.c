/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>
int LT105(char*);
int msg(int,char*[]);
int step1(char*[]);
int step2(char*[],char*,char*,char*,char*,char*);
int step3(char*[],char*,char*,char*,char*,char*,char*);
int step4(char*[],char*,char*,char*,char*,char*,char*);
int step5(char*[]);

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
      fprintf(stderr," Could not open output file [%s]\n", OutputFilename );
      return( FAIL );
      }

   /* Open the trace file */
   TraceFile = NULL;
   TraceFile = fopen( TraceFilename, "w" );
   if( !TraceFile ) {
      fprintf(stderr, " Could not open trace file [%s]\n", TraceFilename );
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
   FILE*  InputFile ;
   char*  data[]={
             "pid",
             "item",
             "ec",
             0 } ;
   char   temp[40] ;
   int    j ;
   int    is = 0 ;
   int    seq = 1;

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf(stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
   }

   while (fgets( temp, 40, InputFile ) )
   {
      if( strlen(temp) != 0 ) {
         data[0] = (char *)malloc( strlen(temp) + 1 );
         memset( data[0], 0, strlen(temp) );
         strncpy( data[0], temp, strlen(temp)-1 );
         data[0][strlen(temp)-1] = '\0' ;
      }
      for ( j = 1; data[j]; j++ ) {
         fgets( temp, 40, InputFile ) ;
         if( strlen(temp) != 0 ) {
            data[j] = (char *)malloc( strlen(temp) + 1 );
            memset( data[j], 0, strlen(temp) );
            strncpy( data[j], temp, strlen(temp)-1 );
            data[j][strlen(temp)-1] = '\0' ;
         }
      }

      printf("%d %-40s%-30s ", seq++, data[1], data[2] );
      is = step1( data );
      RC = msg( is, data );
      printf("\n" );
   }

   fclose( InputFile ) ;
   return (OK) ;
}

/*--------------------------------------------------------------------*/
/*  Function                                                          */
/*--------------------------------------------------------------------*/
int step1( char* data[] )
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
      printf("%d", is );
   } else {
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject,attr_name[0], attr_value[0] );
         RC = PM_Get_Object_Attr( pObject,attr_name[3], attr_value[3] );
         RC = PM_Get_Object_Attr( pObject,attr_name[4], attr_value[4] );
         RC = PM_Get_Object_Attr( pObject,attr_name[6], attr_value[6] );
         RC = PM_Get_Object_Attr( pObject,attr_name[8], attr_value[8] );
         if( strcmp(attr_value[8],"11") == 0 )
            is_tool = "Y" ;
         else
            is_tool = "N" ;

         is = step2( data, attr_value[0], attr_value[4], attr_value[3],
                     attr_value[6], is_tool );

         RC = PM_Destroy_Object( pObject );
      }
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*---------------------------------------------------------------------*/
/*  Function                                                           */
/*---------------------------------------------------------------------*/
int step2( char* data[], char* objid,
           char* insert, char* loc_id, char* name, char* is_tool )
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
            0
            };
   char*  master_item_objid ;
   char*  objid2 ;
   int    j;
   int    is = 0 ;

   RC = PM_Create_Object( pObject, pClass );

   RC = PM_Set_Object_Attr( pObject, "db_object_id", objid );
   RC = PM_Set_Object_Attr( pObject, "logical_key@item_number", data[1] );
   RC = PM_Set_Object_Attr( pObject, "il_ec_number", data[2] );
   RC = PM_Set_Object_Attr( pObject, "logical_key@loc_id", loc_id );

   for( j=0; attr_name[j]; j++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[j] );

   RC = PM_Open_Object( pObject );

   RC = PM_Get_Object_Attr( pObject, attr_name[2], master_item_objid );
   RC = PM_Get_Object_Attr( pObject, attr_name[7], objid2 );

   is = step3( data, insert, loc_id, master_item_objid, objid2, name, is_tool );

   RC = PM_Destroy_Object( pObject );
   return( is );
}

/*---------------------------------------------------------------------*/
/*  Function                                                           */
/*---------------------------------------------------------------------*/
int step3( char* data[], char* insert, char* loc_id, char* master_item_objid,
           char* objid2, char* name, char* is_tool )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemMftData_NonEC";
   char*  attr_name[]= {
            "db_object_id",
            "mitem@item_number",
            "ai@ec@id",
            "swbs@manuloc@loc_id",
            "il_action",
            0
            };
   int    j;
   char*  action ;
   int    is = 0 ;

   RC = PM_Create_Object( pObject, pClass );

   RC = PM_Set_Object_Attr( pObject, "db_object_id", "" );
   RC = PM_Set_Object_Attr( pObject, "mitem@item_number", data[1] );
   RC = PM_Set_Object_Attr( pObject, "ai@ec@id", data[2] );
   RC = PM_Set_Object_Attr( pObject, "swbs@manuloc@loc_id", loc_id );

   for( j=0; attr_name[j]; j++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[j] );

   RC = PM_Open_Object( pObject );
   RC = PM_Get_Object_Attr( pObject, attr_name[4], action );

   if( strcmp(action,"C")==0 ) {
      is = 3100 ;
      if( is >= 3100 ) RC = step5( data );
   } else {
      is = step4( data, loc_id, insert, master_item_objid, objid2, name, is_tool );
      if( is == 3110 ) RC = step5( data );
   }
   printf("%d", is );

   RC = PM_Destroy_Object( pObject );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : Mftdata add                                         */
/*-----------------------------------------------------------------*/
int step4( char* data[], char* loc_id, char* insert, char* master_oid,
           char* ai_oid, char* name, char* is_tool )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemMftData_NonEC";
   char*  attr_name[]={
                   "db_object_id",
                   "MOI_requirement",
                   "PI_requirement",
                   "ai@db_object_id",
                   "ai@ec@id",
                   "area",
                   "bendradius",
                   "bom_batch_size",
                   "breadth@virtual_unit_id",
                   "breadth_batch",
                   "breadth_single",
                   "case_not_pi",
                   "ccg",
                   "cell",
                   "chemical_process",
                   "crst",
                   "crst_num",
                   "cycle_ac",
                   "cycle_day",
                   "design_req",
                   "diameter@virtual_unit_id",
                   "diameter_batch",
                   "diameter_single",
                   "disburse_code@curr_key",
                   "doc@db_object_id",
                   "doc@doc_number",
                   "dt_grp",
                   "dt_req",
                   "faa",
                   "fastflow",
                   "frozen_adhesive",
                   "grain_dir",
                   "grp_of_s_part",
                   "height@virtual_unit_id",
                   "height_batch",
                   "height_single",
                   "il_action",
                   "indenture",
                   "insert_seq",
                   "item_lead_time",
                   "item_yield",
                   "length@virtual_unit_id",
                   "length_batch",
                   "length_single",
                   "level",
                   "mat_shared_itm",
                   "mat_shared_per",
                   "mat_shared_yield",
                   "material_type",               /* 48 */
                   "mrouting_type",               /* 49 */
                   "mitem@db_object_id",
                   "mitem@item_number",
                   "mat_shared_yield",
                   "nesting",
                   "non_frozen_adh",
                   "pcqty",
                   "planning_code@curr_key",
                   "qty_per_prod",
                   "rcpt_control_cd@curr_key",
                   "ref_doc",
                   "save_date",
                   "save_time",
                   "sheet_no532",
                   "size_req_of_raw",             /* 63 */
                   "source_code@curr_key",
                   "spec_comp",
                   "spec_for_mat",
                   "swbs@manuloc@loc_id",
                   "swbs@swbs",
                   "tclass",
                   "tdcn_req",
                   "thickness@virtual_unit_id",
                   "thickness_batch",
                   "thickness_single",
                   "user_id",
                   "wctr_name_mft",
                   "wctr_name_used",
                   "wctr_num_mft",
                   "wctr_num_used",               /* 78  */
                   "weight@virtual_unit_id",
                   "weight_batch",
                   "weight_single",
                   "yield_qty",
                   "lai@db_object_id",
                   "pmrouting_type",
                   "ni_req",      /* 85 */
                   "ni_grp",
                   "ni_cycle_ac",
                   "ni_cycle_day",
                   "ni_ref_doc",
                   "honey_dens",
                   "heat_auth@middle_name",    /* 91  */
                   "heat_auth@pdi_system_id",
                   "update_count",
                   0 } ;
   char* value[128] ;
   char   mrtype[1] ;
   PMInstanceListRef* pIList;
   PMInstanceRef*     toolINST;
   PMInstanceRef*     pidINST;
   PMInstanceRef*     serINST;
   PMInstanceRef*     locINST;
   char*              subscribeList = NULL;
   int    ListSize;
   int    i ;
   int    is = 0 ;
   int    RCC ;

   RC = PM_Create_Object(pObject, pClass);

   pIList = PMSDK_CreateInstanceList();
   toolINST = PMSDK_CreateInstance();
   pidINST = PMSDK_CreateInstance();
   serINST = PMSDK_CreateInstance();
   locINST = PMSDK_CreateInstance();

   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "mitem@db_object_id", master_oid, pObject );
   PMSDK_SetAttrValue( pObject, "ai@db_object_id", ai_oid, pObject );
   PMSDK_SetAttrValue( pObject, "user_id", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "ai@ec@id", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "mitem@item_number", data[1], pObject );

   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "mfg_locid", locINST );

   PMSDK_SetValue1( locINST, loc_id, NULL );

   subscribeList=(char *) malloc(8192);
   memset(subscribeList,0,8192);
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   RC = PMSDK_MakeCall( pClass, "cl_copy_mftdata2", pIList, subscribeList );

   if( RC == 0 ) {
      for(i=1; i < 91; i++) {
         value[i] = PMSDK_GetAttrValue( pObject, attr_name[i], pObject );
      }

      if( strcmp(value[5]," ")==0 ) value[5]="" ;                                      /* area */
      if( strcmp(value[85],"0")==0 || strlen(value[85])==0 ) value[85]="2" ;           /* ni_req */

      PMSDK_AddInstance( pIList, "is_tool", toolINST );
      PMSDK_SetValue1( toolINST, is_tool, NULL );
      PMSDK_AddInstance( pIList, "pid_str", pidINST );
      PMSDK_SetValue1( pidINST, data[0], NULL );
      PMSDK_AddInstance( pIList, "server_msg", serINST );
      PMSDK_SetValue1( serINST, "", NULL );

      for(i=1; i < 91; i++) {
         PMSDK_SetAttrValue( pObject, attr_name[i], value[i], pObject );
      }

      PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
      PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
      PMSDK_SetAttrValue( pObject, "il_action", "A", pObject );
/*    PMSDK_SetAttrValue( pObject, "honey_dens", "3", pObject );    */

      if( strcmp(value[9],"0.000")!=0 ||
          strcmp(value[21],"0.000")!=0 ||
          strcmp(value[34],"0.000")!=0 ||
          strcmp(value[42],"0.000")!=0 ||
          strcmp(value[72],"0.000")!=0 ||
          strcmp(value[80],"0.000")!=0 )
          PMSDK_SetAttrValue( pObject, attr_name[63], "1", pObject );
      if( strcmp(value[10],"0.000")!=0 ||
          strcmp(value[22],"0.000")!=0 ||
          strcmp(value[35],"0.000")!=0 ||
          strcmp(value[43],"0.000")!=0 ||
          strcmp(value[73],"0.000")!=0 ||
          strcmp(value[81],"0.000")!=0 )
          PMSDK_SetAttrValue( pObject, attr_name[63], "1", pObject );

      PMSDK_SetAttrValue( pObject, "swbs@manuloc@loc_id", loc_id, pObject );
      PMSDK_SetAttrValue( pObject, "mitem@db_object_id", master_oid, pObject );
      PMSDK_SetAttrValue( pObject, "mitem@item_number", data[1], pObject );
      PMSDK_SetAttrValue( pObject, "ai@ec@id", data[2], pObject );
      PMSDK_SetAttrValue( pObject, "insert_seq", insert, pObject );

      memset( mrtype, '\0', 1);
      strncpy( mrtype, value[49], 1);
      mrtype[1] = '\0' ;
      if ( strcmp(mrtype,"S")==0  || strcmp(mrtype,"M") == 0 || strcmp(mrtype,"W") == 0 )
      {
         PMSDK_SetAttrValue( pObject, attr_name[91], "���ɱM��", pObject );
         PMSDK_SetAttrValue( pObject, attr_name[92], "plmadm02", pObject );
      }

      RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

      if( RC < 0 ) {
         is = -3110 ;
         ErrorCheck("PMSDK_MakeCall");
      } else {
         is = 3110 ;
      }
   } else {
     ErrorCheck("PMSDK_MakeCall");
     is = -3115 ;
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "is_tool" );
   PMSDK_RemoveInstance( pIList, "pid_str" );
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_RemoveInstance( pIList, "mfg_locid" );
   PMSDK_RemoveInstance( pIList, "server_msg" );
   PMSDK_DestroyInstance( toolINST );
   PMSDK_DestroyInstance( pidINST );
   PMSDK_DestroyInstance( serINST );
   PMSDK_DestroyInstance( locINST );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}

/*---------------------------------------------------------------------*/
/*  Function : save to SAPDATA                                         */
/*---------------------------------------------------------------------*/
int step5(char* data[] )
{
   PMInstanceListRef*  pIList;
   PM_Object*          pObject;
   char*               pClass = "SapData" ;
   char*  attr_name[]= {
                      "db_object_id",
                      "program_id",
                      "item_id",
                      "ec_id",
                      "type",
                      "save_date",
                      "terms",
                      "update_count",
                      0
                      };
   time_t today;
   char   end_date[10] ;
   char   Y[8];
   char   m[4];
   char   d[4];
   char   tmpstr[60] ;
   char*  subscribeList = NULL;
   int    i,j;
   int    is = 0 ;

   today = time(NULL);
   strftime(Y,8,"%Y",localtime(&today));
   strftime(m,4,"%m",localtime(&today));
   strftime(d,4,"%d",localtime(&today));
   sprintf( end_date, "%s/%s/%s", m,d,Y);

   RC = PM_Create_Object( pObject, pClass );

   pIList = PMSDK_CreateInstanceList();

   PMSDK_AddInstance( pIList, "object", pObject );

   subscribeList=(char *) malloc(4096);
   strcpy(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   PMSDK_SetAttrValue( pObject, "program_id", data[0], pObject );
   PMSDK_SetAttrValue( pObject, "item_id", data[1], pObject );
   PMSDK_SetAttrValue( pObject, "ec_id", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "type", "MM", pObject );
   PMSDK_SetAttrValue( pObject, "terms", "3", pObject );
   PMSDK_SetAttrValue( pObject, "save_date", end_date, pObject );

   RC = PMSDK_MakeCall( pClass, "cl_update", pIList, subscribeList);
   if ( RC == 0 ) {
      is = 3120 ;
   } else
      is = -3120 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
