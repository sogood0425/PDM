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
int step2(char*[]);
int step3(char*[]);
int step4(char*[]);

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
                 "pid",
                 "item",
                 "ec",
                 "change_action",
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

      if( strlen(data[3]) == 0 ) data[3] = "CHG" ;

      printf("%d %-40s%-30s ", seq++, data[1], data[2] );
      is = step1( data );
      if( is >= 3000 ) {
         printf("%d  ", is );
         RC = step4( data );
         printf("\n");
      } else {
         printf("%d\n", is );
      }
      RC = msg( is, data );
   }

   fclose( InputFile );
   return (is) ;
}

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

/*--------------------------------------------------------------------*/
/*  Function : Get sequence number                                    */
/*--------------------------------------------------------------------*/
int step4(char* data[] )
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
   char*  seq ;
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
   } else {
     RC = PM_Read_List( pList, 0, pObject );
     RC = PM_Get_Object_Attr( pObject,attr_name[17], seq );
     printf("%s ", seq );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
