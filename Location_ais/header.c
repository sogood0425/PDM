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
int step1(char*);
int step11(char*,char*);
int step2(char*[]);
int step3(char*[]);
int step4(char*[],char*,char*,char*);
int step6(char*[]);

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
/* Main : Item Header                                */ 
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

   CloseSession();

   return( RC );
}

/*---------------------------------------------------------------------*/
/*  Function                                                           */
/*---------------------------------------------------------------------*/
int LT105( char* datafile )
{
   FILE*  InputFile ;
   char*  data[]={
                   "program_id",
                   "item_number",
                   "ec_number",
                   "item_type",
                   "uom",
                   "item_name",
                   "per_minit",
                   "per_usr",
                   "ser_req",
                   "sn_format",
                   "wbs",
                   0 } ;
   int    j ;
   int    is = 0 ;
   char   temp[1024] ;
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

      printf("%d %-40s%-30s ", seq++, data[1], data[2] );
      is = step3( data );
      if( is > 0 ) is = step6( data );
      printf("%d\n", is );
      RC = msg( is, data );
   }

   printf("\n");
   fclose( InputFile );
   return (is) ;
}

/*--------------------------------------------------------------------*/
/*  Function : Check Program_ID                                       */
/*--------------------------------------------------------------------*/
int step1(char* pid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ProgramID";
   char*  attr_name[]={
                   "db_object_id",
                   "program_id",
                   "update_count",
                   0 } ;
   char   tmpstr[1024] ;
   int    ListSize;
   int    i ;
   int    find_ind = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(program_id,'");
   strcat(tmpstr,pid);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 )
      find_ind = -100 ;
   else
      find_ind = 100 ;

   RC = PM_Destroy_List( pList );
   return( find_ind );
}

/*--------------------------------------------------------------------*/
/*  Function : Check WBS                                              */
/*--------------------------------------------------------------------*/
int step11( char* pid, char* wbs )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_WBS";
   char*  attr_name[]={
                      "db_object_id",
                      "wbs_num",
                   0 } ;
   char   tmpstr[1024] ;
   int    ListSize;
   int    i ;
   int    find_ind = 0 ;

   if( strlen(wbs) > 0 ) {
      RC = PM_Create_List( pList, pClass );

      for(i=0; attr_name[i]; i++)
         RC = PM_Set_Clause( pList, "Select", attr_name[i] );

      strcpy(tmpstr,"&=(prog_id@program_id,'");
      strcat(tmpstr,pid);
      strcat(tmpstr,"')");
      RC = PM_Set_Clause( pList, "Where", tmpstr );

      strcpy(tmpstr,"&=(wbs_num,'");
      strcat(tmpstr,wbs);
      strcat(tmpstr,"')");
      RC = PM_Set_Clause( pList, "Where", tmpstr );

      RC = PM_Open_List( pList );
      RC = PM_SizeOf_List( pList, ListSize );

      if( ListSize == 0 )
         find_ind = -300 ;
      else
         find_ind = 300 ;

      RC = PM_Destroy_List( pList );
   } else
      find_ind = 340 ;

   return( find_ind );
}

/*-------------------------------------------------------------------*/
/*  Function : Check User_ID                                         */
/*-------------------------------------------------------------------*/
int step2(char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Person";
   char*  attr_name[]={
                   "db_object_id",
                   "dept",
                   "first_name",
                   "middle_name",
                   "last_name",
                   "pdi_system_id",
                   "person_num",
                   0 } ;
   char*  middle ;
   char*  pdi ;
   char   tmpstr[1024];
   int    ListSize;
   int    i,j;
   int    is = 0;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(person_num,'");
   strcat(tmpstr,data[6]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -200 ;
   } else {
      is = 200 ;
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], middle );
      RC = PM_Get_Object_Attr( pObject, attr_name[5], pdi );
      is = step3( data );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : Check EC+ITEM                                         */
/*-------------------------------------------------------------------*/
int step3(char* data[] )
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
                    "ai_status",
                    "update_count",
                    0 } ;
   char   tmpstr[1024] ;
   char*  locid ;
   char*  objid ;
   char*  update_count ;
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(lai_obj@ec@id,'");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(logical_key@item_number,'");
   strcat(tmpstr,data[1]);
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
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[0], objid );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], locid );
      RC = PM_Get_Object_Attr( pObject, attr_name[18], update_count );
      is = step4( data, objid, update_count, locid );
   }


   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : Item herder add                                     */
/*-----------------------------------------------------------------*/
int step4(char* data[], char* objid, char* update_count, char* locid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemEngineeringData";
   char*  attr_name[]={
                   "db_object_id",
                   "pgmid@program_id",
                   "logical_key@item_number",
                   "il_ec_number",
                   "item_type",
                   "unit_of_measure@virtual_unit_id",
                   "name",
                   "serial_num_reqd",
                   "sn_format",
                   "description",
                   "wbs",
                   "class",
                   "pfcn_code",
                   "change_by@middle_name",
                   "change_by@pdi_system_id",
                   "logical_key@loc_id",
                   "design_loc_id",
                   "update_count",
                   0 } ;
   int    ListSize;
   int    i ;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );
   for(i=0; attr_name[i]; i++) RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   RC = PM_Create_Object(pObject, pClass);
   RC = PM_Set_Object_Attr(pObject, "db_object_id", objid );
   RC = PM_Set_Object_Attr(pObject, "update_count", update_count );
   RC = PM_Set_Object_Attr(pObject, "pgmid@program_id", data[0] );
   RC = PM_Set_Object_Attr(pObject, "logical_key@item_number", data[1] );
   RC = PM_Set_Object_Attr(pObject, "logical_key@loc_id", locid );
   RC = PM_Set_Object_Attr(pObject, "il_ec_number", data[2] );
   RC = PM_Set_Object_Attr(pObject, "name", data[5] );
   RC = PM_Set_Object_Attr(pObject, "design_loc_id", locid );
   RC = PM_Set_Object_Attr(pObject, "item_type", data[3] );
   RC = PM_Set_Object_Attr(pObject, "unit_of_measure@virtual_unit_id", data[4] );
   RC = PM_Set_Object_Attr(pObject, "change_by@middle_name", data[6] );
   RC = PM_Set_Object_Attr(pObject, "change_by@pdi_system_id", data[7] );
   RC = PM_Set_Object_Attr(pObject, "serial_num_reqd", data[8] );
   RC = PM_Set_Object_Attr(pObject, "sn_format", data[9] );
   RC = PM_Set_Object_Attr(pObject, "wbs", data[10] );
   RC = PM_Save_Object( pObject);

   if( RC == 0 ) {
     is = 3020 ;
   } else {
      ErrorCheck("PMSDK_MakeCall");
      is = -3020 ;
   }

   RC = PM_Destroy_Object( pObject ) ;
   RC = PM_Destroy_List( pList );

   return( is );
}

/*---------------------------------------------------------------------*/
/*  Function : save to SAPDATA                                         */
/*---------------------------------------------------------------------*/
int step6(char* data[] )
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
   PMSDK_SetAttrValue( pObject, "terms", "2", pObject ); 
   PMSDK_SetAttrValue( pObject, "save_date", end_date, pObject ); 

   RC = PMSDK_MakeCall( pClass, "cl_update", pIList, subscribeList);
   if ( RC == 0 ) {
      is = 3022 ;
   } else
      is = -3022 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
