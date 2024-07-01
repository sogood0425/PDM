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
/* sdk.in input format :                             */
/*                                                   */
/*   userid passwd server configuration              */
/*                                                   */
/*---------------------------------------------------*/
char    User[10];
char    Config[10];
char    Server[10];
char    Password[10];
char    temp[80];
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
int main( int argc, char** argv ) {

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
   fprintf( TraceFile, " Config : [%s]\n", argv[2] );
   fprintf( TraceFile, " Server : [%s]\n", argv[1] );
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
   FILE*  InputFile ;
   char*  data[]={
             "pid",
             "item",
             "ec",
             "planner",
             "int_approver",
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
         strncpy( data[0], temp, strlen(temp)-1 );
         data[0][strlen(temp)-1] = '\0' ;
      }
      for ( j = 1; data[j]; j++ ) {
         fgets( temp, 80, InputFile ) ;
         if( strlen(temp) != 0 ) {
            data[j] = (char *)malloc( strlen(temp) + 1 );
            memset( data[j], 0, strlen(temp) );
            strncpy( data[j], temp, strlen(temp)-1 );
            data[j][strlen(temp)-1] = '\0' ;
         }
      }

      printf( "%d %-15s%-40s%-30s ", seq++, data[0], data[1], data[2] );
      is = step2( data );
      printf( "%d\n", is );
      RC = msg( is, data );
   }

   fclose( InputFile ) ;
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : check folder                                        */
/*-----------------------------------------------------------------*/
int step2( char* data[] )
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
                   "disabled_actions",
                   "update_count",
                   0 } ;
   char*  attr_value[10];
   char*  ff_id ;
   char*  p;
   char   tmpstr[1024];
   char   dtstr[80];
   int    ListSize;
   int    i,j;
   time_t today;
   char   Y[8];
   char   m[4];
   char   d[4];
   int    is = 0 ;

   today = time(NULL);
   strftime(Y,8,"%Y",localtime(&today));
   strftime(m,4,"%m",localtime(&today));
   strftime(d,4,"%d",localtime(&today));
   sprintf( dtstr, "%s/%s/%s", m,d,Y);

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(name,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -3600 ;
      fprintf( TraceFile, "%s %s\n", data[1], data[2] );
   } else {
      RC = PM_Read_List( pList, 0, pObject );

      RC = PM_Get_Object_Attr( pObject, attr_name[0], attr_value[0] );
      RC = PM_Get_Object_Attr( pObject, attr_name[1], attr_value[1] );
      RC = PM_Get_Object_Attr( pObject, attr_name[2], attr_value[2] );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], attr_value[3] );
      RC = PM_Get_Object_Attr( pObject, attr_name[4], ff_id );
      if( strcmp(attr_value[2],"5")==0 && strcmp(attr_value[1],dtstr)==0 ) {
         is = step5( data ) ;
      } else
         is = 3680 ;

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_Object( pObject );
   RC = PM_Destroy_List( pList );
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
   PMSDK_SetAttrValue( pObject, "type", "Routing", pObject ); 
   PMSDK_SetAttrValue( pObject, "terms", "7", pObject ); 
   PMSDK_SetAttrValue( pObject, "save_date", end_date, pObject ); 

   RC = PMSDK_MakeCall( pClass, "cl_update", pIList, subscribeList);
   if ( RC == 0 ) {
      is = 3681 ;
      is = step6( data );
   } else
      is = -3681 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

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
   PMSDK_SetAttrValue( pObject, "terms", "7", pObject ); 
   PMSDK_SetAttrValue( pObject, "save_date", end_date, pObject ); 

   RC = PMSDK_MakeCall( pClass, "cl_update", pIList, subscribeList);
   if ( RC == 0 ) {
      is = 3682 ;
   } else
      is = -3682 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
