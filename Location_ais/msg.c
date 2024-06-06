void CloseSession()
{
   static int called = 0;
   if( called )
      return;
   called++;

   /* Log off of PM server */
   if( LoggedOn ) {
      RC = PM_Logoff();
      if( RC >= 0 )
         LoggedOn = 0;
      ErrorCheck( "PM_Logoff" );
   }

   /* Close PM environment */
   if( EnvOpen ) {
      RC = PM_Close_Env();
      if( RC >= 0 )
         EnvOpen = 0;
      ErrorCheck( "PM_Close_Env" );
   }

   fclose( TraceFile );
   fclose( OutputFile );
}

void ErrorCheck( char* FuncName )
{
   int i, size ;
   fprintf( TraceFile, "RC = %d ---- %s\n", RC, FuncName );
   size = PM_Error_Size();
   for( i = 0; i < size; i++ ) {
      const char* code = PM_Error_MsgCode( i );
      const char* text = PM_Error_MsgTxt( i );
      fprintf( TraceFile, "   Error[%d] :\n", i );
      fprintf( TraceFile, "      Code : [%s]\n", code );
      fprintf( TraceFile, "      Text : [%s]\n", text );
   }
   fprintf( TraceFile, "\n" );
}

int msg(int is, char* data[] )
{
   int j ;
   if( is==-1 && strlen(data[0])==0 ) return(0) ;

   fprintf( OutputFile, "RC = %6d : %s", is, data[0] );
   for ( j = 1; data[j]; j++ ) {
      fprintf( OutputFile, ",%s", data[j] );
   }
   fprintf( OutputFile, "\n" );
   return( 0 );
}

int maximum( int x, int y)
{
   if( x > y)
      return x ;
   else
      return y ;
}

int minimum( int x, int y)
{
   if( x < y)
      return x ;
   else
      return y ;
}

/*---------------------------------------------------*/
/* Locate position of string "needle" in "haystack"  */
/*---------------------------------------------------*/
int strind(char* haystack, char* needle)
{
  int i = 0;
  int d = 0;
  if ( strlen(haystack) >= strlen(needle))
  {
     for (i = strlen(haystack) - strlen(needle); i >= 0; i--)
     {
        int found = 1; /*assume we found (wanted to use boolean)*/
        for (d = 0; d < strlen(needle); d++)
        {
           if (haystack[i + d] != needle[d]) {
              found = 0;
              break;
           }
        }
        if (found == 1) {
            return i;
        }
     }
     return -1;
  } else {
     return -1;
  }
}
