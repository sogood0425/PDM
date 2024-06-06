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
int step2(int , char*[]);
int msg(int, char*[]);
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
                 "loc_id",
                 "number",
                 "name",
                 "legal",
                 "config",
                 "trans_order",
                 "ps_required",
                 "work_content",
                 0 } ;
   int    j ;
   int    seq = 1 ;
   int    is = 0 ;
   char   temp[32760] ;

   InputFilename = datafile ;
   InputFile = NULL;
   InputFile = fopen( InputFilename, "r" );
   if( !InputFile ) {
      fprintf( stderr, "!!! Could not open input file [%s]", InputFilename );
      return( FAIL );
   }

   while (fgets( temp, 32760, InputFile ) )
   {
      RC = 0 ;
      if( strlen(temp) != 0 ) {
         data[0] = (char *)malloc( strlen(temp) + 1 );
         memset( data[0], 0, strlen(temp) );
         strncpy( data[0], temp, strlen(temp)-1 );
      } else
         break;

      for ( j = 1; data[j]; j++ ) {
         fgets( temp, 32760, InputFile ) ;
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
      is = step2( seq++, data );
      RC = msg( is, data );
   }

   fclose( InputFile );
   return (is) ;
}

/*-------------------------------------------------------------------*/
/*  Function : check workcenter                                      */
/*-------------------------------------------------------------------*/
int step2(int seq, char* data[] )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_WorkCenter";
   char*  attr_name[]={
                   "db_object_id",
                   "update_count",
                   "location@loc_id",
                   "wctr_number",
                   "wctr_name",
                   "legal_center",
                   "config",
                   "trans_order",
                   "ps_required",
                   "work_content",
                   0 } ;
   char   tmpstr[1024];
   char   temp1[32760];
   char   temp2[32760];
   char*  objid ;
   char*  update_count ;
   char*  loc_id ;
   char*  num ;
   char*  name ;
   char*  legal ;
   char*  cfg ;
   char*  trans_order ;
   char*  ps ;
   char*  content ;
   int    ListSize;
   int    i, j, is ;
   int    is_change ;
   int    ln ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(location@loc_id, '");
   strcat(tmpstr,data[0]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(wctr_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      RC = PM_Create_Object(pObject, pClass);
      RC = PM_Set_Object_Attr(pObject, "db_object_id", "" );
      RC = PM_Set_Object_Attr(pObject, "update_count", "0" );
      RC = PM_Set_Object_Attr(pObject, "location@loc_id", data[0] );
      RC = PM_Set_Object_Attr(pObject, "wctr_number", data[1] );
      RC = PM_Set_Object_Attr(pObject, "wctr_name", data[2] );
      RC = PM_Set_Object_Attr(pObject, "legal_center", data[3] );
      RC = PM_Set_Object_Attr(pObject, "config", data[4] );
      RC = PM_Set_Object_Attr(pObject, "trans_order", data[5] );
      RC = PM_Set_Object_Attr(pObject, "ps_required", data[6] );
      strcpy( temp1, data[7] );
      ln = strlen( temp1 ) ;
      for( j = 0; j < ln; j++) {
         if( temp1[j] == 7 )
            temp2[j] = 10 ;
         else
            temp2[j] = temp1[j] ;
      }
      temp2[ln] = '\0' ;
      RC = PM_Set_Object_Attr(pObject, "work_content", temp2 );

      RC = PM_Save_Object( pObject);
      if( RC == 0 )
         fprintf( stdout, "%d %s %s %s created success\n", seq,data[0],data[1],data[2] );
      else
         fprintf( stdout, "%d %s %s %s created fail\n", seq,data[0],data[1],data[2] );

      RC = PM_Destroy_Object( pObject );
   } else {
      is = 1000 ;
      is_change = 0 ;
      RC = PM_Read_List( pList, 0, pObject );

      RC = PM_Get_Object_Attr( pObject,attr_name[0], objid );
      RC = PM_Get_Object_Attr( pObject,attr_name[1], update_count );
      RC = PM_Get_Object_Attr( pObject, attr_name[2], loc_id );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], num    );
      RC = PM_Get_Object_Attr( pObject, attr_name[4], name   );
      RC = PM_Get_Object_Attr( pObject, attr_name[5], legal  );
      RC = PM_Get_Object_Attr( pObject, attr_name[6], cfg  );
      RC = PM_Get_Object_Attr( pObject, attr_name[7], trans_order );
      RC = PM_Get_Object_Attr( pObject, attr_name[8], ps );
      RC = PM_Get_Object_Attr( pObject, attr_name[9], content );
      strcpy( temp1, content );
      ln = strlen( temp1 ) ;
      for( j = 0; j < ln; j++) {
         if( temp1[j] == 10 )
            temp2[j] = 7 ;
         else
            temp2[j] = temp1[j] ;
      }
      temp2[ln] = '\0' ;

      if( strcmp( name, data[2]) != 0 ) is_change = 1 ;
      if( strcmp( legal, data[3]) != 0 ) is_change = 1 ;
      if( strcmp( cfg, data[4]) != 0 ) is_change = 1 ;
      if( strcmp( trans_order, data[5]) != 0 ) is_change = 1 ;
      if( strcmp( ps, data[6]) != 0 ) is_change = 1 ;
      if( strcmp( temp2, data[7]) != 0 ) is_change = 1 ;

      if( is_change == 1 ) {
         RC = PM_Create_Object( pObject, pClass);
         RC = PM_Set_Object_Attr(pObject, "db_object_id", objid );
         RC = PM_Set_Object_Attr(pObject, "update_count", update_count );
         RC = PM_Set_Object_Attr(pObject, "location@loc_id", data[0] );
         RC = PM_Set_Object_Attr(pObject, "wctr_number", data[1] );
         RC = PM_Set_Object_Attr(pObject, "wctr_name", data[2] );
         RC = PM_Set_Object_Attr(pObject, "legal_center", data[3] );
         RC = PM_Set_Object_Attr(pObject, "config", data[4] );
         RC = PM_Set_Object_Attr(pObject, "trans_order", data[5] );
         RC = PM_Set_Object_Attr(pObject, "ps_required", data[6] );
         strcpy( temp1, data[7] );
         ln = strlen( temp1 ) ;
         for( j = 0; j < ln; j++) {
            if( temp1[j] == 7 )
               temp2[j] = 10 ;
            else
               temp2[j] = temp1[j] ;
         }
         temp2[ln] = '\0' ;
         RC = PM_Set_Object_Attr(pObject, "work_content", temp2 );
         RC = PM_Save_Object( pObject);
         if( RC == 0 )
            fprintf( stdout, "%d %s %s %s updated success\n", seq,data[0],data[1],data[2] );
         else
            fprintf( stdout, "%d %s %s %s updated fail\n", seq,data[0],data[1],data[2] );

         RC = PM_Destroy_Object( pObject );
      }
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
