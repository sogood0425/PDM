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
int step3(char*[],char*);
int save(char*[],char*,char*,char*);
int step4(char*[],char*,char*,char*);
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
/* Main : Item Component                             */
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

   RC = PM_Logon( "plmadm01", "tel3259", argv[1], argv[2] );

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
                 "cmp_itm_id",
                 "quantity",
                 "drawing_zone",
                 "note",
                 "addec",
                 "from_item_id",
                 "to_item_id",
                 "thickness",
                 "thickness_unit",
                 "breadth",
                 "breadth_unit",
                 "length",
                 "length_unit",
                 "weight",
                 "weight_unit",
                 "material_uom",
                 "instance_id",
                 "yield_quantity",
                 "description",
                 0 } ;
   int    j ;
   int    is = 0 ;
   int    seq = 1 ;
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
      if( strlen(data[8])==0 ) data[8] = data[2] ;
      if( strlen(data[11])==0 ) data[11] = "0" ;
      if( strlen(data[13])==0 ) data[13] = "0" ;
      if( strlen(data[15])==0 ) data[15] = "0" ;
      if( strlen(data[17])==0 ) data[17] = "0" ;
      if( strlen(data[21])==0 ) data[21] = "0" ;

      if(strcmp(data[0],"I")==0 || strcmp(data[0],"i")==0 )
         is = 1 ;
      else
         is = -1 ;

      printf("%d %s %s ", seq++, data[2], data[3] );
      if( is > 0 ) is = step1( data[1] );
      if( is > 0 ) is = step2( data );
      printf("%d\n", is );
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
   char   tmpstr[1024] ;
   char*  loc_id ;
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
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], loc_id );
      is = step3( data, loc_id );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-------------------------------------------------------------------*/
/*  Function : Check Component Type                                  */
/*-------------------------------------------------------------------*/
int step3(char* data[], char* loc_id )
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
   char   tmpstr[1024] ;
   char*  cmp_item_objid;
   char*  ec;
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );

   strcpy(tmpstr,"&=(logical_key@item_number,'");
   strcat(tmpstr,data[4]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, ai_obj@design_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, ai_obj@design_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, ' ')" );
   RC = PM_Set_Clause( pList, "Where", "&LIKE(ai_obj@ec@chg_control_code, '01%')" );

   RC = PM_Set_Clause( pList, "OrderBy", "ai_obj@design_seq_num" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize == 0 ) {
      is = -30401 ;
   } else {
      is = 30401 ;
      RC = PM_Read_List( pList, ListSize-1, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[0], cmp_item_objid);
      RC = PM_Get_Object_Attr( pObject, attr_name[2], ec);
      is = step4( data, loc_id, cmp_item_objid, ec);
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*-----------------------------------------------------------------------*/
/*  Function : Add component                                             */
/*-----------------------------------------------------------------------*/
int save(char* data[], char* loc_id, char* cmp_type, char* cmp_unit )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_Component";
   char*  attr_name[]={
                   "db_object_id",
                   "logical_key@loc_id",
                   "il_ec_number",
                   "logical_key@item_number",
                   "logical_key@comp_item_number",
                   "measur_obj@virtual_real",
                   "draw_zone",
                   "note_qty",
                   "item_type@curr_key",
                   "from_item_id",
                   "to_item_id",
                   "thickness@virtual_real",
                   "thickness@virtual_unit_id",
                   "breadth@virtual_real",
                   "breadth@virtual_unit_id",
                   "length@virtual_real",
                   "length@virtual_unit_id",
                   "weight@virtual_real",
                   "weight@virtual_unit_id",
                   "wall_thk@virtual_unit_id",
                   "instance_id",
                   "yield_qty",
                   "description",
                   "measur_obj@virtual_unit_id",
                   "add_ec_store",
                   0 } ;
   PMInstanceListRef* pIList;
   PMInstanceRef*     ecINST;
   PMInstanceRef*    indINST;
   char*  subscribeList = NULL;
   char*  tmpobj ;
   int    ListSize;
   int    i ;
   int    is = 0 ;

   RC = PM_Create_Object(pObject, pClass);

   pIList = PMSDK_CreateInstanceList();
   ecINST = PMSDK_CreateInstance();
   indINST = PMSDK_CreateInstance();

   for(i=2; i<23; i++)
      PMSDK_SetAttrValue( pObject, attr_name[i], data[i], pObject );

   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "update_count", "0", pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "item_type@curr_key", cmp_type, pObject );
   PMSDK_SetAttrValue( pObject, "measur_obj@virtual_unit_id", cmp_unit, pObject );
   PMSDK_SetAttrValue( pObject, "add_ec_store", data[2], pObject );

   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "il_ec_number", ecINST );
   PMSDK_AddInstance( pIList, "from_list_indicator", indINST );

   PMSDK_SetValue1( ecINST, data[2], NULL );
   PMSDK_SetValue1( indINST, "1", NULL );

   RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

   if( RC >= 0 ) {
      is = 30410 ;
   } else {
      ErrorCheck("PMSDK_MakeCall");
      is = -30410 ;
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "from_list_indicator" );
   PMSDK_RemoveInstance( pIList, "il_ec_number" );
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstance( indINST );
   PMSDK_DestroyInstance( ecINST );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}

/*------------------------------------------------------------*/
/*  Function : Check Component attribute                      */
/*------------------------------------------------------------*/
int step4( char* data[], char* loc_id, char* cmp_item_objid, char* ec)
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_ItemEngineeringData";
   char*  attr_name[]={
                       "db_object_id",
                       "logical_key@item_number",
                       "il_ec_number",
                       "logical_key@loc_id",
                       "item_type",
                       "unit_of_measure@virtual_unit_id",
                        0 } ;
   PMInstanceListRef* pIList;
   char*  subscribeList = NULL;
   int    i ;
   int    is = 0 ;
   char*  type ;
   char*  unit ;

   RC = PM_Create_Object(pObject, pClass) ;

   pIList = PMSDK_CreateInstanceList();

   PMSDK_AddInstance( pIList, "object", pObject );

   PMSDK_SetAttrValue( pObject, "db_object_id", cmp_item_objid, pObject );
   PMSDK_SetAttrValue( pObject, "il_ec_number", ec, pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@item_number", data[4], pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", "", pObject );

   subscribeList=(char *) malloc(2048);
   strcpy(subscribeList, "");
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   RC = PMSDK_MakeCall( pClass, "cl_open_object", pIList, subscribeList);
   if ( RC == 0 ) {
      is = 3300 ;
      RC = PM_Get_Object_Attr( pObject,attr_name[4], type );
      RC = PM_Get_Object_Attr( pObject,attr_name[5], unit  );
   } else {
      type = "" ;
      unit = "9005" ;
   }

   is = save( data, loc_id, type, unit);

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject );

   return( is );
}
