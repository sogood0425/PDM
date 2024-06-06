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
int step21(char*[]);
int step24(char*[], char*);
int step25(char*);
int msg(int,char*[]);
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

      printf("%d %-40s%-30s ", seq++, data[1], data[2] );
      is = step21( data );
      printf("%d\n", is );

      RC = msg( is, data );
   }

   fclose( InputFile );
   return (is) ;
}

/*-------------------------------------------------------------------*/
/*  Function : Check EC                                              */
/*-------------------------------------------------------------------*/
int step21(char* data[] )
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
   char*  ec_objid ;
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

   strcpy(tmpstr,"&=(programid@program_id, '");
   strcat(tmpstr,data[0]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&LIKE(chg_control_code, '02%')" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -10000 ;
   } else {
      is = 10000 ;
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[0], ec_objid );
      is = step24( data, ec_objid );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : EC LAI delete                                       */
/*-----------------------------------------------------------------*/
int step24(char* data[], char* ec_objid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_LocationAffectedItem";
   char*  attr_name[]={
                   "db_object_id",
                   "update_count",
                   "ec@id",
                   "item_number",
                   "act_re_run_IR_test",
                   "act_eff_type",
                   "act_eff_date",
                   "change_act_lai",
                   "item_engg_status",
                   "item_status@curr_key",
                   "item_name",
                   "loc@loc_id",
                   "wbs",
                   "pid",
                   0 } ;
   char*  objid ;
   char*  item ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(ec@db_object_id, '");
   strcat(tmpstr,ec_objid);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(item_key@item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 )
      is = -14000 ;
   else {
      is = 14000 ;
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );
         RC = PM_Get_Object_Attr( pObject, attr_name[0], objid );
         is = step25( objid );
         RC = PM_Destroy_Object( pObject ) ;
      }
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : EC LAI delete                                       */
/*-----------------------------------------------------------------*/
int step25( char* ai_objid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_LocationAffectedItem";
   char*  attr_name[]={
                   "db_object_id",
                   0 } ;
   PMInstanceListRef* pIList;
   char*  objectSubscribesString ;
   char*  subscribeList = NULL;
   int    is = 0 ;

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();

   PMSDK_SetAttrValue( pObject, "db_object_id", ai_objid, pObject );

   PMSDK_AddInstance( pIList, "object", pObject );

   RC = PMSDK_MakeCall( pClass, "cl_delete", pIList, subscribeList );
   if( RC < 0 ) {
      ErrorCheck( "PMSDK_MakeCall" );
      is = -14030 ;
   } else
      is = 14030 ;

   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
