/*---------------------------------------------------*/
/* Include files                                     */
/*---------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <apghdr.h>
#include <sdk_tst.h>
int LT104(char *,char *);

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

char User[10];
char Config[10];
char Server[10];
char Password[10];
char pid_cond[40];
char dept[10];
char *TraceFilename = "sdk.log";
char *InputFilename = "sdk.in";
char *OutputFilename = "person.data";
FILE *TraceFile;
FILE *InputFile;
FILE *OutputFile;
int RC;
int LoggedOn = 0;
int EnvOpen = 0;

/*---------------------------------------------------*/
/* Main                                              */
/*---------------------------------------------------*/
int main(int argc, char **argv)
{

   /* Open the input file */
   InputFile = NULL;
   InputFile = fopen(InputFilename, "r");
   if (!InputFile)
   {
      fprintf(stderr, " Could not open input file [%s]\n", InputFilename);
      return (FAIL);
   }

   /* Open the output file */
   OutputFile = NULL;
   OutputFile = fopen(OutputFilename, "w");
   if (!OutputFile)
   {
      fprintf(stderr, " Could not open output file [%s]\n", OutputFilename);
      return (FAIL);
   }

   /* Open the trace file */
   TraceFile = NULL;
   TraceFile = fopen(TraceFilename, "w");
   if (!TraceFile)
   {
      fprintf(stderr, " Could not open trace file [%s]\n", TraceFilename);
      return (FAIL);
   }

   /* Open the PM environment */
   RC = PM_Open_Env();
   if (RC >= 0)
      EnvOpen = 1;

   fscanf(InputFile, "%s %s %s %s",
          User, Password, Server, Config);

   RC = PM_Logon(User, Password, Server, Config);

   if (RC >= 0)
      LoggedOn = 1;
   else
      ErrorCheck("PM_Logon");

   while (fscanf(InputFile, "%s %s", pid_cond, dept) != EOF)
      RC = LT104(pid_cond,dept);

   CloseSession();

   return (0);
}

/*--------------------------------------------------------------------------*/
/*  Function                                                                */
/*--------------------------------------------------------------------------*/
int LT104(char *pid_cond, char *dept)
{

   PM_Object_List *pList;
   PM_Object *pObject;
   char *PersonClass = "AIDC_Person";
   char *attr_name[] = {
       "db_object_id",
       "dept",
       "person_num",
       "middle_name",
       "first_name",
       "last_name",
       "per_dpt_phone",
       "user_id@system_id",
       "pdi_network",
       "pdi_node",
       "active_flag",
       "resp_type",
       "email_id",
       "email_type",
       0};
   char *attr_value[20];
   char tmp[8];
   char tmpin[80];
   char tmpstr[80];
   int ListSize;
   int i, j;

   RC = PM_Create_List(pList, PersonClass);

   for (i = 0; attr_name[i]; i++)
      RC = PM_Set_Clause(pList, "Select", attr_name[i]);

   strcpy(tmpin, pid_cond);
   if (tmpin[strlen(pid_cond) - 1] == '%')
      strcpy(tmpstr, "&LIKE(person_num,'");
   else
      strcpy(tmpstr, "&=(person_num,'");
   strcat(tmpstr, pid_cond);
   strcat(tmpstr, "')");
   RC = PM_Set_Clause(pList, "Where", tmpstr);

   //20240315
   strcpy(tmpin, dept);
   if (tmpin[strlen(dept) - 1] == '%')
      strcpy(tmpstr, "&LIKE(dept,'");
   else
      strcpy(tmpstr, "&=(dept,'");
   strcat(tmpstr, dept);
   strcat(tmpstr, "')");
   RC = PM_Set_Clause(pList, "Where", tmpstr);

   RC = PM_Set_Clause(pList, "OrderBy", "person_num");

   RC = PM_Open_List(pList);
   RC = PM_SizeOf_List(pList, ListSize);

   for (i = 0; i < ListSize; i++)
   {
      RC = PM_Read_List(pList, i, pObject);

      for (j = 0; attr_name[j]; j++)
         RC = PM_Get_Object_Attr(pObject, attr_name[j], attr_value[j]);

      for (j = 1; attr_name[j]; j++)
         fprintf(OutputFile, "%s\n", attr_value[j]);

      RC = PM_Destroy_Object(pObject);
   }

   RC = PM_Destroy_List(pList);
   return (OK);
}
