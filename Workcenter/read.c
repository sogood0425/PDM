/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>
int step2(char*);

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
char*   OutputFilename = "workcenter.data";
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

   RC = step2( argv[3] ) ;

   CloseSession();

   return( RC );
}

/*-------------------------------------------------------------------*/
/*  Function : get workcenter                                        */
/*-------------------------------------------------------------------*/
int step2(char* config )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_WorkCenter";
   char*  attr_name[]={
                   "db_object_id",
                   "location@loc_id",
                   "wctr_number",
                   "wctr_name",
                   "legal_center",
                   "config",
                   "trans_order",
                   "ps_required",
                   "work_content",
                   0 } ;
   char*  loc_id ;
   char*  num ;
   char*  name ;
   char*  legal ;
   char*  cfg ;
   char*  trans_order ;
   char*  ps ;
   char*  content ;
   char   tmpstr[1024];
   char   temp1[32760];
   char   temp2[32760];
   int    ListSize;
   int    i, j, is ;
   int    ln ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&LIKE(config, '%");
   strcat(tmpstr,config);
   strcat(tmpstr,"%')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&LIKE(location@loc_id, '%')" );
   RC = PM_Set_Clause( pList, "Where", "&LIKE(wctr_number, 'T%')" );
   RC = PM_Set_Clause( pList, "OrderBy", "location@loc_id" );
   RC = PM_Set_Clause( pList, "OrderBy", "wctr_number" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -1000 ;
   } else {
      is = 1000 ;
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject, attr_name[1], loc_id );
         RC = PM_Get_Object_Attr( pObject, attr_name[2], num    );
         RC = PM_Get_Object_Attr( pObject, attr_name[3], name   );
         RC = PM_Get_Object_Attr( pObject, attr_name[4], legal  );
         RC = PM_Get_Object_Attr( pObject, attr_name[5], cfg  );
         RC = PM_Get_Object_Attr( pObject, attr_name[6], trans_order );
         RC = PM_Get_Object_Attr( pObject, attr_name[7], ps );
         RC = PM_Get_Object_Attr( pObject, attr_name[8], content );

         strcpy( temp1, content );
         ln = strlen( temp1 ) ;
         for( j = 0; j < ln; j++) {
            if( temp1[j] == 10 )
               temp2[j] = 7 ;
            else
               temp2[j] = temp1[j] ;
         }
         temp2[ln] = '\0' ;

         fprintf(OutputFile, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", loc_id, num, name, legal, cfg, trans_order, ps, temp2 );

         RC = PM_Destroy_Object( pObject );
      }
   }

   RC = PM_Destroy_List( pList );
   return( is );
}
