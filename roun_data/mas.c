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
             "item_number",
             "ec_number",
             "revision_date",
             "per_id",
             "description",
             0 } ;
   char   temp[1024] ;
   int    j ;
   int    is = 0 ;
   int    seq = 1 ;

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf( stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
   }

   while (fgets( temp, 1024, InputFile ) )
   {
      if( strlen(temp) != 0 ) {
         data[0] = (char *)malloc( strlen(temp) + 1 );
         memset( data[0], 0, strlen(temp) );
         strncpy( data[0], temp, strlen(temp)-1 );
         data[0][strlen(temp)-1] = '\0' ;
      }
      for ( j = 1; data[j]; j++ ) {
         fgets( temp, 1024, InputFile ) ;
         if( strlen(temp) != 0 ) {
            data[j] = (char *)malloc( strlen(temp) + 1 );
            memset( data[j], 0, strlen(temp) );
            strncpy( data[j], temp, strlen(temp)-1 );
            data[j][strlen(temp)-1] = '\0' ;
         }
      }

      printf( "%d %-25s%-40s%-30s ", seq++, data[0], data[1], data[2] );

      is = step1( data );
      RC = msg( is, data );

      printf( "%d\n", is );
   }

   fclose( InputFile ) ;
   return (OK) ;
}

/*--------------------------------------------------------------------*/
/*  Function : Check Item                                             */
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
                    "insert_by",
                    "update_count",
                    "ai_status",
                    0 } ;
   char*  attr_value[20] ;
   char   tmpstr[80] ;
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
   } else {
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject,attr_name[3], attr_value[3] );
         RC = PM_Get_Object_Attr( pObject,attr_name[4], attr_value[4] );
         RC = PM_Get_Object_Attr( pObject,attr_name[5], attr_value[5] );
         RC = PM_Get_Object_Attr( pObject,attr_name[7], attr_value[7] );

         is = step2( data, attr_value[3], attr_value[4], attr_value[5], attr_value[7] );

         RC = PM_Destroy_Object( pObject );
      }
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*---------------------------------------------------------------------*/
/*  Function : Open Item Object                                        */
/*---------------------------------------------------------------------*/
int step2( char* data[], char* loc_id, char* pgm_id, char* name, char* type )
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
            0
            };
   char   tmpstr[10] ;
   char*  cl_seq;
   char*  lai_objid;
   int    j;
   int    is = 0 ;

   RC = PM_Create_Object( pObject, pClass );

   RC = PM_Set_Object_Attr( pObject, "logical_key@item_number", data[1] );
   RC = PM_Set_Object_Attr( pObject, "il_ec_number", data[2] );
   RC = PM_Set_Object_Attr( pObject, "logical_key@loc_id", loc_id );

   for( j=0; attr_name[j]; j++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[j] );

   RC = PM_Open_Object( pObject );

   RC = PM_Get_Object_Attr( pObject, attr_name[1], cl_seq );
   RC = PM_Get_Object_Attr( pObject, attr_name[3], lai_objid );

   is = step4( data, name, loc_id, pgm_id, type, cl_seq, lai_objid );

   RC = PM_Destroy_Object( pObject );
   return( is );
}

/*---------------------------------------------------------------------*/
/*  Function : Check Item Routing Data                                 */
/*---------------------------------------------------------------------*/
int step3( char* data[], char* name, char* loc_id, char* pgm_id, char* type, char* cl_seq,
           char* lai_objid, char* dept, char* first, char* last, char* middle )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           ItemClass = "AIDC_ItemRoutingData";
   char*  attr_name[]={
                   "db_object_id",
                   "logical_key@item_number",
                   "logical_key@loc_id",
                   "aidc_routid",
                   "logical_key@routid",
                   "ec_number",
                   "rout_type_ind@curr_key",
                   "revision_level",
                   "item_physical_status",
                   "disposition_code",
                   "work_order_loc",
                   0 } ;
   char*  attr_value[10];
   char   tmpstr[256];
   int    ListSize;
   int    i ;
   int    is = 0 ;
   int    js = 0 ;

   RC = PM_Create_List( pList, ItemClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(logical_key@item_number,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(logical_key@loc_id,'");
   strcat(tmpstr,loc_id);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&>(extract_by, ");
   strcat(tmpstr,cl_seq);
   strcat(tmpstr,")");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&BETWEEN(insert_by, 0, ");
   strcat(tmpstr,cl_seq);
   strcat(tmpstr,")");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(ECNUM,'");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(ec_number,'");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&LIKE(rout_type_ind@cuur_key,'2%')" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize != 0 ) {
      is = 3500 ;
   } else {
      is = -3500 ;
      if ( strcmp(type,"11")==0 || strcmp(type,"12")==0 || strcmp(type,"23")==0 || strcmp(type,"24")==0 || strcmp(type,"25")==0 )
         js = 1 ;
      else
         js = step8( data, loc_id );
      if( js <= 0 )
         is = -3520 ;  /* no PU exists */
      else
         is = step6( data, name, loc_id, pgm_id, lai_objid, dept, first, last, middle );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*--------------------------------------------------------------------------*/
/*  Function : Check Person                                                 */
/*--------------------------------------------------------------------------*/
int step4( char* data[], char* name, char* loc_id, char* pgm_id, char* type, char* cl_seq, char* lai_objid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           PersonClass = "AIDC_Person";
   char*  attr_name[]={
                   "db_object_id",
                   "dept",
                   "first_name",
                   "last_name",
                   "middle_name",
                   "user_id@system_id",
                   0 } ;
   char*  attr_value[10];
   char   tmpstr[80];
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, PersonClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(user_id@system_id,'");
   strcat(tmpstr,data[4]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize == 0 ) {
      is = -200 ;
   } else {
      RC = PM_Read_List( pList, 0, pObject );

      for( j = 0; attr_name[j] ; j++)
         RC = PM_Get_Object_Attr( pObject,attr_name[j], attr_value[j] );

      is = step3( data, name, loc_id, pgm_id, type, cl_seq, lai_objid,
                  attr_value[1], attr_value[2], attr_value[3], attr_value[4] );

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------------*/
/*  Function : Add Item Routing Data                                     */
/*-----------------------------------------------------------------------*/
int step5( char* data[], char* name, char* loc_id, char* pgm_id, char* lai_objid,
           char* dept, char* first, char* last, char* middle )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemRoutingData";
   char*  attr_name[]={
                      "program_id",
                      "logical_key@item_number",
                      "ec_number",
                      "revision_date",
                      "revised_by@pdi_system_id",
                      "description",
                      "memo_description",
                      "material_description",
                      "master_routing_type",
                      "disposition_code",
                      "revision_level",
                      "itm_name",
                      "rout_type_ind@curr_key",
                      "aidc_routid",
                      "tool_id",
                      "logical_key@loc_id",
                      "revised_by@dept",
                      "revised_by@first_name",
                      "revised_by@middle_name",
                      "revised_by@last_name",
                      "il_ec_number",
                      "logical_key@routid",
                      "work_order_loc",
                      "db_object_id",
                      "update_count",
                      "change_by@middle_name",
                   0 } ;
   PMInstanceListRef* pIList;
   PMInstanceRef*     ecINST;
   PMInstanceRef*     toolINST;
   PMInstanceRef*     laiINST;
   char*  subscribeList = NULL;
   char   tmpstr[80] ;
   int    ListSize;
   int    i ;
   int    is = 0 ;
   time_t today;
   char   Y[8];
   char   m[4];
   char   d[4];

   RC = PM_Create_Object(pObject, pClass);

   pIList = PMSDK_CreateInstanceList();
   ecINST   = PMSDK_CreateInstance();
   toolINST = PMSDK_CreateInstance();
   laiINST  = PMSDK_CreateInstance();

   for(i=0; i<6; i++) {
      PMSDK_SetAttrValue( pObject, attr_name[i], data[i], pObject );
   }

   strcpy(tmpstr,data[1]);
   strcat(tmpstr,"2");

   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", loc_id, pObject );
   PMSDK_SetAttrValue( pObject, "rout_type_ind@curr_key", "2", pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@dept", dept, pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@first_name", first, pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@middle_name", middle, pObject );
   PMSDK_SetAttrValue( pObject, "revised_by@last_name", last, pObject );
   PMSDK_SetAttrValue( pObject, "tool_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@routid", tmpstr, pObject );
   PMSDK_SetAttrValue( pObject, "il_ec_number", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "aidc_routid", data[1], pObject );
   PMSDK_SetAttrValue( pObject, "itm_name", name, pObject );
   PMSDK_SetAttrValue( pObject, "revision_level", "-", pObject );
   PMSDK_SetAttrValue( pObject, "work_order_loc", loc_id, pObject );
   PMSDK_SetAttrValue( pObject, "change_by@middle_name", middle, pObject );

   if( strlen(data[3]) == 0 ) {
      today = time(NULL);
      strftime(Y,8,"%Y",localtime(&today));
      strftime(m,4,"%m",localtime(&today));
      strftime(d,4,"%d",localtime(&today));
      sprintf( tmpstr, "%s/%s/%s", m,d,Y);
      PMSDK_SetAttrValue( pObject, "revision_date", tmpstr, pObject );
   }

   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "il_ec_number", ecINST );
   PMSDK_AddInstance( pIList, "is_tooling", toolINST );
   PMSDK_AddInstance( pIList, "oid_lai", laiINST );
   PMSDK_SetValue1( ecINST, data[2], ecINST );
   PMSDK_SetValue1( toolINST, "0", toolINST );
   PMSDK_SetValue1( laiINST, lai_objid, laiINST );

   ErrorCheck("PMSDK_SetValue1");

   RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

   if( RC == 0 ) {
      is = 3510 ;
   } else {
      is = -3510 ;
      fprintf( TraceFile, "step5 : %s\t%s\n", data[1], data[2] );
      ErrorCheck("PMSDK_MakeCall");
   }

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

/*-----------------------------------------------------------------------*/
/*  Function : Add Item Document Note                                    */
/*-----------------------------------------------------------------------*/
int step6( char* data[], char* name, char* loc_id, char* pgm_id, char* lai_objid,
           char* dept, char* first, char* last, char* middle )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_DocumentNote";
   char*  attr_name[]={
                      "master_item@item_number",
                      "ndi_req",
                      "change_by@pdi_system_id",
                      "change_by@middle_name",
                      "change_date",
                      "db_object_id",
                      "update_count",
                   0 } ;
   PMInstanceListRef* pIList;
   char*  subscribeList = NULL;
   char*  objid ;
   int    ListSize;
   int    i ;
   int    is = 0 ;
   int    ds = 0 ;

   RC = PM_Create_Object(pObject, pClass);
   pIList = PMSDK_CreateInstanceList();

   RC = PM_Set_Object_Attr( pObject, "db_object_id", "" );
   RC = PM_Set_Object_Attr( pObject, "master_item@item_number", data[1] );

   for( i=0; attr_name[i]; i++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[i] );

   RC = PM_Open_Object( pObject );

   if( RC < 0 ) {
      PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
      PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
      PMSDK_SetAttrValue( pObject, "master_item@item_number", data[1], pObject );
      PMSDK_SetAttrValue( pObject, "ndi_req", "0", pObject );
      PMSDK_AddInstance( pIList, "object", pObject );
      RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

      if( RC == 0 ) {
         ds = 3610 ;
         RC = PM_Open_Object( pObject );
         RC = PM_Get_Object_Attr( pObject, "db_object_id", objid );
      } else {
         ErrorCheck("PMSDK_MakeCall");
         ds = -3610 ;
      }
   } else {
      ds = 3600 ;
   }

   if( ds >= 0 ) is = step5( data, name, loc_id, pgm_id, lai_objid, dept, first, last, middle );
   if( ds == 3610 ) RC = step7( objid );

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}

/*-----------------------------------------------------------------------*/
/*  Function : Delete Item Document Note                                 */
/*-----------------------------------------------------------------------*/
int step7( char* objid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_DocumentNote";
   char*  attr_name[]={
                      "db_object_id",
                   0 } ;
   PMInstanceListRef* pIList;
   char*  subscribeList = NULL;
   int    i ;

   RC = PM_Create_Object(pObject, pClass);
   pIList = PMSDK_CreateInstanceList();

   RC = PM_Set_Object_Attr( pObject, "db_object_id", objid );

   PMSDK_AddInstance( pIList, "object", pObject );

   RC = PMSDK_MakeCall( pClass, "cl_delete", pIList, subscribeList );

   if( RC != 0 ) ErrorCheck("PMSDK_MakeCall");

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( OK );
}

/*-----------------------------------------------------------------*/
/*  Function : check PU exists ?                                   */
/*-----------------------------------------------------------------*/
int step8(char* data[], char* loc_id )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemProductUnitEffectivity";
   char*  attr_name[]={
                   "db_object_id",
                   "update_count",
                   "prod_id",
                   "item_number",
                   "loc_id",
                   "ecid",
                   "cut_in_unit",
                   "cut_out_unit",
                   "desc",
                   "eff_view",
                   0 } ;
   char*  objid ;
   char*  attr_value[20];
   char   tmpstr[1024];
   int    ListSize;
   int    i, j, is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(ecid, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&OR(&=(loc_id, '");
   strcat(tmpstr,loc_id);
   strcat(tmpstr,"'), &=(loc_id, ''))");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&=(eff_view, 2)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   is = ListSize ;

   RC = PM_Destroy_List( pList );
   return( is );
}
