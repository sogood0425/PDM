/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>
int LT104(char*);
int step1(char*[]);

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
char*   TraceFilename = "sdk.log";
char*   InputFilename = "sdk.in";
char*   OutputFilename = "sdk.out";
char*   datafile;
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
   char* configs[] = {"A321", "AIDC-MRO", "AIRBUS", "AJT","AT3","BD100","BELL","Boeing","C27J","CAPU","C_Series","EC120","F16"
   ,"IDF","LearJet45","NCSIST","RE100","S76","S92",NULL};

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
   else
      ErrorCheck( "PM_Open_Env" );


   RC = PM_Logon( User, Password, argv[1], argv[2] );

   if( RC >= 0 )
      LoggedOn = 1;
   else
      ErrorCheck( "PM_Logon" );

   fprintf( stdout, "\n[%s]\n", argv[2] );

   if( argc == 3 )
      datafile = "person.data" ;
   else
      datafile = argv[3] ;

   RC = LT104( datafile );

   CloseSession();

   return( 0 );
}

/*--------------------------------------------------------------------------*/
/*  Function                                                                */
/*--------------------------------------------------------------------------*/
int LT104(char* datafile )
{
   char*  InputFilename ;
   FILE*  InputFile ;
   char*  data[]={
             "dept",
             "person_num",
             "middle_name",
             "first_name",
             "last_name",
             "per_dpt_phone",
             "person_identifier",
             "pdi_network",
             "pdi_node",
             "active_flag",
             "resp_type",
             "email_id",
             "email_type",
             0 } ;
   char   temp[80] ;
   int    j ;

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );

   if( !InputFile ) {
      fprintf( stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
   }

   while (fgets( temp, 80, InputFile ) )
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

      RC = step1( data ) ;

   }

   fclose( InputFile ) ;
   return (OK) ;
}

/*-----------------------------------------------------------------*/
/*  Function                                                       */
/*-----------------------------------------------------------------*/
int step1( char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Person";
   char*  attr_name[]={
                   "dept",
                   "person_num",
                   "middle_name",
                   "first_name",
                   "last_name",
                   "per_dpt_phone",
                   "person_identifier",
                   "pdi_network",
                   "pdi_node",
                   "pdi_system_id",
                   "db_object_id",
                   "update_count",
                   "email_id",
                   "email_type",
                   "active_flag",
                   0 } ;
   char*  attr_value[20];
   char   tmpstr[80];
   int    ListSize;
   int    i,j;
   char*  update_count ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(person_num,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "dept", data[0] );
      RC = PM_Set_Object_Attr(pObject, "person_num", data[1] );
      RC = PM_Set_Object_Attr(pObject, "middle_name", data[2] );
      RC = PM_Set_Object_Attr(pObject, "first_name", data[3] );
      RC = PM_Set_Object_Attr(pObject, "last_name", data[4] );
      RC = PM_Set_Object_Attr(pObject, "per_dpt_phone", data[5] );
      RC = PM_Set_Object_Attr(pObject, "person_identifier", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_system_id", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_network", data[7] );
      RC = PM_Set_Object_Attr(pObject, "pdi_node", data[8] );
      RC = PM_Set_Object_Attr(pObject, "active_flag", data[9] );
      RC = PM_Set_Object_Attr(pObject, "resp_type", data[10] );
      RC = PM_Set_Object_Attr(pObject, "email_id", data[11] );
      RC = PM_Set_Object_Attr(pObject, "email_type", data[12] );
      RC = PM_Set_Object_Attr(pObject, "db_object_id", NULL );
      RC = PM_Set_Object_Attr(pObject, "update_count", "0" );

      RC = PM_Save_Object( pObject);
      if( RC == 0 )
         fprintf( stdout, "%s created success\n", data[1] ) ;
      else
         ErrorCheck( "PM_Save_Object" );

      RC = PM_Destroy_Object( pObject );
   } else {
      RC = PM_Read_List( pList, 0, pObject );

      RC = PM_Get_Object_Attr( pObject,attr_name[10], attr_value[10] );
      RC = PM_Get_Object_Attr( pObject,attr_name[11], attr_value[11] );

      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "dept", data[0] );
      RC = PM_Set_Object_Attr(pObject, "person_num", data[1] );
      RC = PM_Set_Object_Attr(pObject, "middle_name", data[2] );
      RC = PM_Set_Object_Attr(pObject, "first_name", data[3] );
      RC = PM_Set_Object_Attr(pObject, "last_name", data[4] );
      RC = PM_Set_Object_Attr(pObject, "per_dpt_phone", data[5] );
      RC = PM_Set_Object_Attr(pObject, "person_identifier", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_system_id", data[6] );
      RC = PM_Set_Object_Attr(pObject, "pdi_network", data[7] );
      RC = PM_Set_Object_Attr(pObject, "pdi_node", data[8] );
      RC = PM_Set_Object_Attr(pObject, "active_flag", data[9] );
      RC = PM_Set_Object_Attr(pObject, "resp_type", data[10] );
      RC = PM_Set_Object_Attr(pObject, "email_id", data[11] );
      RC = PM_Set_Object_Attr(pObject, "email_type", data[12] );
      RC = PM_Set_Object_Attr(pObject, "db_object_id", attr_value[10] );
      RC = PM_Set_Object_Attr(pObject, "update_count", attr_value[11] );

      RC = PM_Save_Object( pObject);
      if( RC == 0 )
         fprintf( stdout, "%s updated success\n", data[1] ) ;
      else
         ErrorCheck( "PM_Save_Object" );

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );

   return( OK );
}
