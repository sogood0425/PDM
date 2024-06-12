/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>
int LT999(char*);
int msg(int,char*[]);
int step1(char*[]);
int step2(char*[],char*,char*,char*,char*);
int step3(char*[],char*,char*);
int step4(char*[],char*,char*);

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

   RC = LT999( datafile ) ;

   CloseSession();

   return( RC );
}

/*---------------------------------------------------------------------*/
/*  Function                                                           */
/*---------------------------------------------------------------------*/
int LT999( char* datafile )
{
   FILE*  InputFile ;
   char*  data[]={
                 "pdmid",
                 "item_number",
                 "ec_number",
                 "fai",
                 "mod_description",
                 "Effective_date",
                 "situation_code",
                 "processing_code",
                 "Model",
                 "cut_in",
                 "cut_out",
                 "qty",
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

      /*printf("%d pdmid=[%s] item_number=[%s] ec_number=[%s] fai=[%s] mod_description=[%s] Effective_date=[%s] situation_code=[%s] processing_code=[%s] Model=[%s] cut_in=[%s] cut_out=[%s] qty=[%s] ",
      seq++, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10]);
      printf("\n");
      printf("\n");*/
      is = step1( data );
      printf( "%d %s %s %d\n",seq++,data[0],data[1], is );

      RC = msg( is, data );
   }

   fclose( InputFile );
   return (is) ;
}

/*-------------------------------------------------------------------*/
/*  Function : Check Item List exists ?                              */
/*-------------------------------------------------------------------*/
int step1(char* data[] )
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
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;
   char*  db_object_id ;
   char*  locid ;\
   char*  il_ec_number;
   char*  item_number ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&LIKE(logical_key@item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&LIKE(lai_obj@ec@id, '");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, lai_obj@mfg_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, lai_obj@loc@loc_id)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -1000 ;
   } else {
      is = 1000 ;
      for( i = 0; i < ListSize; i++ ) {
         RC = PM_Read_List( pList, i, pObject );

         RC = PM_Get_Object_Attr( pObject, attr_name[0], db_object_id );
         RC = PM_Get_Object_Attr( pObject, attr_name[2], il_ec_number );
         RC = PM_Get_Object_Attr( pObject, attr_name[1], item_number );
         RC = PM_Get_Object_Attr( pObject, attr_name[3], locid );

      /*printf("db_object_id = %s \n", db_object_id );
      printf("il_ec_number = %s \n", il_ec_number );
      printf("item_number = %s \n", item_number );
      printf("locid = %s \n", locid );
      printf("\n" );*/
      }

      is = step2(data, db_object_id, il_ec_number, item_number ,locid );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : Get lai_obj@db_object_id                           */
/*-------------------------------------------------------------------*/

int step2(char* data[], char* db_object_id, char* il_ec_number, char* item_number, char* locid)
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
            "db_object_id2",
            "rev_level",
            0
            };
   char*  lai_obj_db_object_id ;
   int    j;
   int    is = 0 ;
   char*  rev_level ;

   RC = PM_Create_Object( pObject, pClass );

   RC = PM_Set_Object_Attr( pObject, "db_object_id", db_object_id );
   RC = PM_Set_Object_Attr( pObject, "logical_key@item_number", item_number );
   RC = PM_Set_Object_Attr( pObject, "il_ec_number", il_ec_number );
   RC = PM_Set_Object_Attr( pObject, "logical_key@loc_id", locid );

   for( j=0; attr_name[j]; j++)
      RC = PM_Subscribe_Object_Attr( pObject ,attr_name[j] );

   RC = PM_Open_Object( pObject );

   RC = PM_Get_Object_Attr( pObject, attr_name[3], lai_obj_db_object_id );
   RC = PM_Get_Object_Attr( pObject, attr_name[8], rev_level );
   /*printf("lai_obj_db_object_id = %s \n", lai_obj_db_object_id );*/

   is = step3(data,lai_obj_db_object_id,rev_level );

   RC = PM_Destroy_Object( pObject );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : EC IAD add                          */
/*-------------------------------------------------------------------*/

int step3(char* data[],char* lai_obj_db_object_id, char* rev_level)
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemActualDisposition";
   char*  attr_name[]={
                   "act_re_run_FAI",
                   "actual_disp",
                   "db_object_id",
                   "il_action",
                   "item_number",
                   "item_rev",
                   "lai_obj.db_object_id",
                   "physical_st",
                   "item_number",
                   "prod_id",
                   "sort_seq",
                   "cut_in_unit",
                   "quantity",
                   0 } ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is ;
   char*  db_object_id ;
   char*  locid ;
   char*  il_ec_number;
   char*  item_number ;


   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );


   strcpy(tmpstr,"&LIKE(prod_id, '");
   strcat(tmpstr,"%");
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(lai_obj.db_object_id, '");
   strcat(tmpstr,lai_obj_db_object_id);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   /*printf("***%s \n", lai_obj_db_object_id );*/


   strcpy(tmpstr,"&=(item_number, '");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   /*printf("***%s \n", data[1] );*/

   strcpy(tmpstr,"&=(physical_st,'");
   strcat(tmpstr,data[6]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(actual_disp,'");
   strcat(tmpstr,data[7]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(cut_in_unit, ");
   strcat(tmpstr,data[9]);
   strcat(tmpstr," )");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(quantity, ");
   strcat(tmpstr,data[11]);
   strcat(tmpstr," )");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

/*
   RC = PM_Set_Clause( pList, "Where", "&LIKE(chg_control_code, '02%')" );
*/
   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if( ListSize == 0 ) {
      is = -3000 ;
      is = step4(data, lai_obj_db_object_id, rev_level );
   } else {
      is = 3000 ;

   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------*/
/*  Function : EC IAD add                                          */
/*-----------------------------------------------------------------*/
int step4(char* data[],char* lai_obj_db_object_id, char* rev_level )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemActualDisposition";
   char*  attr_name[]={
                     "db_object_id",
                     "item_number",
                     "affect_op",
                     "act_re_run_FAI",
                     "desc",
                     "effective_date",
                     "physical_st",
                     "actual_disp",
                     "prod_id",
                     "cut_in_unit",
                     "cut_out_unit",
                     "quantity",
                     "item_rev",
                     "master_body_remake",
                     "leak_req",
                     "update_count",
                     "lai_obj@db_object_id",
                   0 } ;
   char*  item ;
   char   tmpstr[1024];
   int    ListSize;
   int    i, is = 0 ;
   char*  subscribeList = NULL;
   PMInstanceListRef* pIList;
   PMInstanceRef*     confINST;

   RC = PM_Create_Object(pObject,pClass);

   pIList = PMSDK_CreateInstanceList();
   confINST = PMSDK_CreateInstance();

   for(i=1; i < 12; i++) {
      PMSDK_SetAttrValue( pObject, attr_name[i], data[i], pObject );
   }

   PMSDK_SetAttrValue( pObject, "lai_obj@db_object_id", lai_obj_db_object_id, pObject );
   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
   PMSDK_SetAttrValue( pObject, "affect_op", "", pObject );
   PMSDK_SetAttrValue( pObject, "master_body_remake", "N", pObject );
   PMSDK_SetAttrValue( pObject, "leak_req", "N", pObject );
   if( strlen(rev_level)==0 )
      PMSDK_SetAttrValue( pObject, "item_rev", "-", pObject );
   else
      PMSDK_SetAttrValue( pObject, "item_rev", rev_level, pObject );
   RC = PM_Create_List( pList, pClass );
   PMSDK_SetAttrValue( pObject, "effective_date", data[5], pObject );

   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "config", confINST );

   subscribeList=(char *) malloc(2048);
   strcpy(subscribeList, NULL);
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   PMSDK_SetValue1( confINST, data[0], NULL );

   RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

   if( RC != 0 ) {
       ErrorCheck( "PMSDK_MakeCall" );
       is = -3010 ;
   } else
       is = 3010 ;

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "config" );
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstance( confINST );
   PMSDK_DestroyInstanceList( pIList );

   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
