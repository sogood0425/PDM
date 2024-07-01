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
/* Main : Item Header                                */ 
/*---------------------------------------------------*/
int main( int argc, char** argv )
{
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
                  "program_id",
                  "item_number",   /*  1  */
                  "ec_number",
                  "start_tmp",
                  "final_tmp",
                  "ht_par",      /*  5  */
                  "sub_not_ht",
                  "cmmt",
                  "req_heat",    /*  8  */
                  "alumin",      /*  9  */
                  "sol_heat_tre",
                  "aging1",
                  "aging2",
                  "non_alumin",    /*  13  */
                  "hard_tmp",
                  "aust_con",
                  "pre_hard",
                  "normalize",
                  "stablize",
                  "stress_rel",    /*  19  */
                  0 } ;
   int    j, k ;
   int    is = 0 ;
   char   temp[1024] ;
   int    seq = 1 ;

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

      for ( k = 9; k < 20 ; k++ ) {
         if( strcmp(data[k],"V") ==0 )  data[k]="1" ;
         if( strcmp(data[k]," ") ==0 )  data[k]="0" ;
         if( strlen(data[k])== 0 )      data[k]="0" ;
      }

      printf("%d %-40s%-30s ", seq++, data[1], data[2] );
      is = step3( data );
      printf("%d\n", is );
      RC = msg( is, data );
   }

   printf("\n");
   fclose( InputFile );
   return (is) ;
}

/*-------------------------------------------------------------------*/
/*  Function : Check EC+ITEM                                         */
/*-------------------------------------------------------------------*/
int step3(char* data[] )
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
                    "insert_by",
                    "ai_status",
                    "update_count",
                    0 } ;
   char   tmpstr[1024] ;
   char*  locid ;
   int    ListSize;
   int    i,j;
   int    is = 0 ;

   RC = PM_Create_List( pList, pClass );

   for(i=0; attr_name[i]; i++)
      RC = PM_Set_Clause( pList, "Select", attr_name[i] );
/*
   strcpy(tmpstr,"&=(pgmid@program_id,'");
   strcat(tmpstr,data[0]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );
*/
   strcpy(tmpstr,"&=(lai_obj@ec@id,'");
   strcat(tmpstr,data[2]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   strcpy(tmpstr,"&=(logical_key@item_number,'");
   strcat(tmpstr,data[1]);
   strcat(tmpstr,"')");
   RC = PM_Set_Clause( pList, "Where", tmpstr );

   RC = PM_Set_Clause( pList, "Where", "&<=(insert_by, lai_obj@mfg_seq_num)" );
   RC = PM_Set_Clause( pList, "Where", "&OR(&=(extract_by, 2147483647), &>(extract_by, lai_obj@mfg_seq_num))" ) ;
   RC = PM_Set_Clause( pList, "Where", "&=(logical_key@loc_id, lai_obj@loc@loc_id)" );

   RC = PM_Open_List( pList );
   RC = PM_SizeOf_List( pList, ListSize );

   if ( ListSize == 0 ) {
      is = -3000 ;
   } else {
      RC = PM_Read_List( pList, 0, pObject );
      RC = PM_Get_Object_Attr( pObject, attr_name[3], locid );
      is = step5( data, locid );
   }

   RC = PM_Destroy_List( pList );
   return( is );
}

/*------------------------------------------------------------*/
/*  Function : check Heat treatment data exists?              */
/*------------------------------------------------------------*/
int step5(char* data[], char* locid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_HeatTreatment";
   char*  attr_name[]={
                   "db_object_id",
                   "logical_key@item_number",   /*  1  */
                   "il_ec_number",
                   "start_tmp",
                   "final_tmp",
                   "ht_par",      /*  5  */
                   "sub_not_ht",
                   "cmmt",
                   "req_heat",
                   "alumin",      /*  9  */
                   "sol_heat_tre",
                   "aging1",
                   "aging2",
                   "non_alumin",    /*  13  */
                   "hard_tmp",
                   "aust_con",
                   "pre_hard",
                   "normalize",
                   "stablize",
                   "stress_rel",    /*  19  */
                   "ec_id",
                   "not_req_heat",
                   "logical_key@loc_id",
                   "update_count",
                   "ec_id",
                   "rclrsn",
                   "chg_by@middle_name",
                   "chg_date",
                   0 } ;
   PMInstanceListRef* pIList;
   char*  subscribeList = NULL;
   int    i ;
   int    is = 0 ;
   char*  objid ;
   char*  updt_cnt ;
   char*  ec ;

   RC = PM_Create_Object(pObject, pClass) ;

   pIList = PMSDK_CreateInstanceList();

   subscribeList=(char *) malloc(4096);
   strcpy(subscribeList, NULL);
   strcat(subscribeList,"object@");
   for(i=0; attr_name[i]; i++) {
      strcat( subscribeList, attr_name[i]);
      strcat( subscribeList, ",");
   }
   strcat(subscribeList,"disabled_actions");

   PMSDK_AddInstance( pIList, "object", pObject );

   PMSDK_SetAttrValue( pObject, "db_object_id", "", pObject );
   PMSDK_SetAttrValue( pObject, "il_ec_number", data[2], pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@item_number", data[1], pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", locid, pObject );

   RC = PMSDK_MakeCall( pClass, "cl_open_object", pIList, subscribeList);

   if ( RC < 0 ) {
      is = -3200 ;
      is = step6( data, "", "0", locid );
   } else {
      ec = PMSDK_GetAttrValue( pObject, attr_name[20], pObject );
      if( strcmp(data[2],ec) == 0 ) {
         is = 3200 ;
      } else {
         is = 3199 ;
         printf("%d ", is );
         objid = PMSDK_GetAttrValue( pObject, attr_name[0], pObject );
         updt_cnt = PMSDK_GetAttrValue( pObject, attr_name[23], pObject );
         is = step6( data, objid, updt_cnt, locid );
      }
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject );

   return( is );
}

/*------------------------------------------------------------*/
/*  Function : Heat Treatment add                             */
/*------------------------------------------------------------*/
int step6(char* data[], char* objid, char* updt_cnt, char* locid )
{
   PM_Object_List* pList;
   PM_Object*      pObject;
   char*           pClass = "AIDC_HeatTreatment";
   char*  attr_name[]={
                   "db_object_id",
                   "logical_key@item_number",   /*  1  */
                   "il_ec_number",
                   "start_tmp",
                   "final_tmp",
                   "ht_par",      /*  5  */
                   "sub_not_ht",
                   "cmmt",
                   "req_heat",
                   "alumin",      /*  9  */
                   "sol_heat_tre",
                   "aging1",
                   "aging2",
                   "non_alumin",    /*  13  */
                   "hard_tmp",
                   "aust_con",
                   "pre_hard",
                   "normalize",
                   "stablize",
                   "stress_rel",    /*  19  */
                   "ec_id",
                   "not_req_heat",
                   "logical_key@loc_id",
                   "update_count",
                   "ec_id",
                   "rclrsn",
                   "chg_by@middle_name",
                   "chg_date",
                   "chg_time",
                   0 } ;
   PMInstanceListRef* pIList;
   PMInstanceRef*     ecINST;
   char*  subscribeList = NULL;
   int    ListSize;
   int    i ;
   int    is = 0 ;

   RC = PM_Create_Object(pObject, pClass);

   pIList = PMSDK_CreateInstanceList();
   ecINST = PMSDK_CreateInstance();

   PMSDK_SetAttrValue( pObject, "db_object_id", objid, pObject );
   PMSDK_SetAttrValue( pObject, "update_count", updt_cnt, pObject );
   PMSDK_SetAttrValue( pObject, "logical_key@loc_id", locid, pObject );
   PMSDK_SetAttrValue( pObject, "ec_id", data[2], pObject );

   for(i=1; i < 20; i++) {
      PMSDK_SetAttrValue( pObject, attr_name[i], data[i], pObject );
   }

   if( strcmp(data[5],"V") == 0 ) {
      PMSDK_SetAttrValue( pObject, "ht_par", "1", pObject );
      PMSDK_SetAttrValue( pObject, "sub_not_ht", "0", pObject );
   }
   if( strcmp(data[6],"V") == 0 ) {
      PMSDK_SetAttrValue( pObject, "ht_par", "0", pObject );
      PMSDK_SetAttrValue( pObject, "sub_not_ht", "1", pObject );
   }
   if( strcmp(data[8],"¬O")==0 || strcmp(data[8],"V")==0 || strcmp(data[8],"Y")==0 ) {
      PMSDK_SetAttrValue( pObject, "req_heat", "1", pObject );
      PMSDK_SetAttrValue( pObject, "not_req_heat", "0", pObject );
   }
   if( strcmp(data[8],"§_")==0 || strcmp(data[8],"X")==0 || strcmp(data[8],"N")==0 ) {
      PMSDK_SetAttrValue( pObject, "req_heat", "0", pObject );
      PMSDK_SetAttrValue( pObject, "not_req_heat", "1", pObject );
   }

   PMSDK_AddInstance( pIList, "object", pObject );
   PMSDK_AddInstance( pIList, "il_ec_number", ecINST );

   PMSDK_SetValue1( ecINST, data[2], NULL );

   RC = PMSDK_MakeCall( pClass, "cl_save", pIList, subscribeList );

   if( RC == 0 ) {
      is = 3210 ;
   } else {
      ErrorCheck( "PMSDK_MakeCall" );
      is = -3210 ;
   }

   if( subscribeList != NULL ) {
      free( subscribeList );
      subscribeList = NULL;
   }
   PMSDK_RemoveInstance( pIList, "il_ec_number" );
   PMSDK_RemoveInstance( pIList, "object" );
   PMSDK_DestroyInstance( ecINST );
   PMSDK_DestroyInstanceList( pIList );
   RC = PM_Destroy_Object( pObject ) ;

   return( is );
}
