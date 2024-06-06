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
int step1(char*);
int step2(char*, char*);
int step3(char*, char*);

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
char    per_num_cond[40];
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

   /* Record logon data in trace file */
   fprintf( TraceFile, "Logon data :\n" );
   fprintf( TraceFile, " User   : [%s]\n", User );
   fprintf( TraceFile, " Config : [%s]\n", Config );
   fprintf( TraceFile, " Server : [%s]\n", Server );
   fprintf( TraceFile, "\n" );

   /* Open the PM environment */
   RC = PM_Open_Env();
   if( RC >= 0 )
      EnvOpen = 1;

   RC = PM_Logon( User, Password, Server, Config );

   if( RC >= 0 )
      LoggedOn = 1;

   if( argc == 1 )
      datafile = "person.data" ;
   else
      datafile = argv[1] ;

   fprintf( stdout, "\nConfig : %s\n", Config ) ;
   RC = LT104( datafile ) ;

   /* Log off, close environment, close trace file */
   CloseSession();

   return( 0 );
   }

/*----------------------------------------------------------------*/
/*  Function                                                      */
/*----------------------------------------------------------------*/
int LT104(char* datafile)
{
   FILE*  InputFile ;
   char   per_num_cond[40];

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf( stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
      }

   while( fscanf(InputFile,"%s", per_num_cond ) != EOF ) {
      RC = step1( per_num_cond );
   }

   fclose( InputFile ) ;
   return (OK) ;
}

/*-----------------------------------------------------------------*/
/*  Function                                                       */
/*-----------------------------------------------------------------*/
int step1( char* per_num_cond )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "User";
   char*  attr_name[]={
                    "db_object_id",
                    "person@person_num",
                    "nls_pref@db_object_id",
                    "security_groups",
                    "pdi_network",
                    "pdi_node",
                    "pdi_userid",
                    "disabled_actions",
                    "update_count",
                    0 } ;
   char*  attr_value[20];
   char   tmpstr[80];
   int    ListSize;
   int    i,j;
   int    is = 0 ;
   char*  update_count ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++) {
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );
   }

   strcpy(tmpstr,"&=(person@person_num,'");
   strcat(tmpstr,per_num_cond);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      fprintf( stdout, "%s not exist\n", per_num_cond ) ;
   } else {
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );
         RC = PM_Get_Object_Attr( pObject,attr_name[0], attr_value[0] );
         RC = PM_Get_Object_Attr( pObject,attr_name[6], attr_value[6] );
      }

      RC = PM_Create_Object( pObject, pClass);
      RC = PM_Set_Object_Attr(pObject, "db_object_id", attr_value[0] );

      RC = PM_Delete_Object( pObject);
      if( RC != 0 ) {
         fprintf( stdout, "User   %s can not delete\n", per_num_cond ) ;
      } else {
         fprintf( stdout, "User   %s delete\n", per_num_cond ) ;
         RC = step2( per_num_cond, attr_value[6] );
      }

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );

   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function                                                       */
/*-----------------------------------------------------------------*/
int step2( char* per_num_cond, char* per_userid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Person";
   char*  attr_name[]={
                   "db_object_id",
                   "person_num",
                   "dept",
                   "middle_name",
                   "first_name",
                   "last_name",
                   "per_dpt_phone",
                   "person_identifier",
                   "pdi_network",
                   "pdi_node",
                   "pdi_system_id",
                   "update_count",
                   0 } ;
   char*  attr_value[20];
   char   tmpstr[80];
   int    ListSize;
   int    i,j;
   char*  update_count ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++) {
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );
   }

   strcpy(tmpstr,"&=(person_num,'");
   strcat(tmpstr, per_num_cond);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      fprintf( stdout, "%s not exist\n", per_num_cond ) ;
   } else {
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         for( j = 0; attr_name[j] ; j++) {
            RC = PM_Get_Object_Attr( pObject,attr_name[j], attr_value[j] );
         }
      }

      RC = PM_Create_Object( pObject, pClass);
      RC = PM_Set_Object_Attr(pObject, "db_object_id", attr_value[0] );

      RC = PM_Delete_Object( pObject);
      if( RC != 0 ) {
         fprintf( stdout, "Person %s can not delete\n", per_num_cond ) ;
         RC = step3( per_num_cond, per_userid );
      } else {
         fprintf( stdout, "Person %s delete\n", per_num_cond ) ;
      }

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );

   return( OK );
}

/*-----------------------------------------------------------------*/
/*  Function                                                       */
/*-----------------------------------------------------------------*/
int step3( char* per_id, char* per_userid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "User";
   char*  attr_name[]={
                   "db_object_id",
                   "update_count",
                   "person@person_num",
                   "pdi_userid",
                   "pdi_network",
                   "pdi_node",
                   "nls_pref@db_object_id",
                   0 } ;
   char*  attr_value[20];
   char   tmpstr[80];
   int    ListSize;
   int    i,j;
   char*  update_count ;

   RC = PM_Create_List( pList, pClass );

   for (i=0; attr_name[i]; i++) {
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );
   }

   strcpy(tmpstr,"&=(person@person_num,'");
   strcat(tmpstr,per_id);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize > 0 ) {
     fprintf( stdout, "%s exist\n", per_id ) ;
   } else {
      RC = PM_Create_Object( pObject, pClass);

      RC = PM_Set_Object_Attr(pObject, "person@person_num", per_id );
      RC = PM_Set_Object_Attr(pObject, "pdi_userid", per_userid );
      RC = PM_Set_Object_Attr(pObject, "pdi_network", "TCPIP" );
      RC = PM_Set_Object_Attr(pObject, "pdi_node", "pmserver" );
      RC = PM_Set_Object_Attr(pObject, "db_object_id", "" );
      RC = PM_Set_Object_Attr(pObject, "update_count", "0" );
      RC = PM_Set_Object_Attr(pObject, "nls_pref@db_object_id", "" );

      RC = PM_Save_Object( pObject);
      if( RC == 0 )
         fprintf( stdout, "User   %s add again\n", per_id ) ;
      else
         ErrorCheck( "PM_Save_Object" );

      RC = PM_Destroy_Object( pObject );
   }

   RC = PM_Destroy_List( pList );

   return( OK );
}
