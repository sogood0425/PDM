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
int step1(char*);
int step2(char*[]);
int add(char*[]);
int del(char*[]);
int del1(char*);
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
char*   datafile;
char*   TraceFilename = "sdk.log";
char*   InputFilename ;
char*   OutputFilename = "sdk.out";
FILE*   TraceFile;
FILE*   InputFile;
FILE*   OutputFile;
int     RC;
int     LoggedOn = 0;
int     EnvOpen = 0;

/*---------------------------------------------------*/
/* Main : Item P.U. Effectivity                      */ 
/*---------------------------------------------------*/
int main( int argc, char** argv )
{
   /* Open the output file */
   OutputFile = NULL;
   OutputFile = fopen( OutputFilename, "a" );
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

   RC = PM_Logon( "aidcaes", "aidcaes", argv[1], argv[2] );

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
                 "key",
                 "program_id",
                 "ec_number",
                 "item_number",
                 "product_id",
                 "cutin",
                 "cutout",
                 "desc",
                 0 } ;
   int    j ;
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

      if(strcmp(data[0],"D")==0 || strcmp(data[0],"d")==0 ||
         strcmp(data[0],"I")==0 || strcmp(data[0],"i")==0 )
         is = 1 ;
      else
         is = -1 ;

      if( is > 0 ) is = step1( data[1] );
      if( is > 0 ) is = step2( data );
      RC = msg( is, data );
   }

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

/*-------------------------------------------------------------------*/
/*  Function : Check EC+ITEM                                         */
/*-------------------------------------------------------------------*/
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
                    "pgmid@program_id",
                    "item_type",
                    "ai_obj@eng_effty_level",
                    "ai_obj@eff_type",
                    "ai_obj@change_action@curr_key",
                    "ai_obj@pln_rev_level",
                    "ai_obj@item_status",
                    "ai_obj@ec@id",
                    "ai_obj@ec@status",
                    "ai_obj@ec@db_object_id",
                    "ai_obj@ec@chg_control_code",
                    "ai_obj@design_seq_num",
                    "insert_by",
                    "ai_status",
                    "update_count",
                    0 } ;
   char   tmpstr[80] ;
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(logical_key@item_number,'");
   strcat(tmpstr,data[3]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(ai_obj@ec@id,'");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(pgmid@program_id,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, ai_obj@design_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, ai_obj@design_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, ' ')" );
   RC = PM_Set_Clause( pList, "Where", "&LIKE(ai_obj@ec@chg_control_code, '01%')" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize == 0 ) {
      is = -3000 ;
   } else {
      is = 3000 ;
      if( strcmp(data[0],"D") == 0 || strcmp(data[0],"d") == 0 ) {
         is = del( data );
      } else
         is = add( data );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : ITEM PU add                                         */
/*-----------------------------------------------------------------*/
int add(char* data[] )
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
                   "mfg_seqnum",
                   0 } ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   RC = PM_Create_Object(pObject, pClass);
   RC = PM_Set_Object_Attr(pObject, "db_object_id", NULL );
   RC = PM_Set_Object_Attr(pObject, "update_count", "0" );
   RC = PM_Set_Object_Attr(pObject, "ecid", data[2] );
   RC = PM_Set_Object_Attr(pObject, "item_number", data[3] );
   RC = PM_Set_Object_Attr(pObject, "prod_id", data[4] );
   RC = PM_Set_Object_Attr(pObject, "cut_in_unit", data[5] );
   RC = PM_Set_Object_Attr(pObject, "cut_out_unit", data[6] );
   RC = PM_Set_Object_Attr(pObject, "desc", data[7] );
   RC = PM_Set_Object_Attr(pObject, "loc_id", " " );
   RC = PM_Save_Object( pObject);
   if( RC == 0 )
      is = 3110 ;
   else {
      ErrorCheck("PM_Save_Object");
      is = -3110 ;
   }

   RC = PM_Destroy_List( pList );
   RC = PM_Destroy_Object( pObject ) ;
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : ITEM PU delete                                      */
/*-----------------------------------------------------------------*/
int del(char* data[] )
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
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(ecid, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(item_number, '");
   strcat(tmpstr,data[3]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(prod_id, '");
   strcat(tmpstr,data[4]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(cut_in_unit, ");
   strcat(tmpstr,data[5]);
   strcat(tmpstr,")");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(cut_out_unit, ");
   strcat(tmpstr,data[6]);
   strcat(tmpstr,")");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&=(eff_view, 1)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -3100 ;
   } else {
      is = 3100 ;
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[0], objid );
      is = del1( objid );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : ITEM PU delete                                      */
/*-----------------------------------------------------------------*/
int del1( char* pu_objid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemProductUnitEffectivity";
   char*  attr_name[]={
                   "db_object_id",
                   0 } ;
   PMInstanceListRef* pIList;
   char*  objectSubscribesString ;
   char*  subscribeList = NULL;
   int    is = 0 ;

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();

   PMSDK_SetAttrValue( pObject, "db_object_id", pu_objid, pObject );

   PMSDK_AddInstance( pIList, "object", pObject );

   RC = PMSDK_MakeCall( pClass, "cl_delete", pIList, subscribeList );
   if( RC < 0 ) {
      ErrorCheck( "PMSDK_MakeCall" );
      is = -3130 ;
   } else
      is = 3130 ;

   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
