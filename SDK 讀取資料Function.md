```
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
```

[LT104] function
`data[0] = (char *)malloc( strlen(temp) + 1 );`
strlen(temp) + 1 是因為需要額外的空間來儲存字串的結束符號 '\0'。
`memset( data[0], 0, strlen(temp) );`
我們只需要設定 temp 中的實際內容長度的記憶體區域即可
不需要設定結束字元 '\0'
這行程式碼將 data[0] 的內容全部設為 0，也就是清空 data[0]。
`strncpy( data[0], temp, strlen(temp)-1 );`
strlen(temp) - 1 的目的是為了只複製 temp 中的內容，不包括結尾的 '\n' 字元
這行程式碼將 temp 陣列的內容複製到 data[0] 中。注意這裡只複製了 strlen(temp)-1 個字元，也就是除了 '\n' 之外的所有字元。

[實際運作]
 fgets每讀取一列會自動加入換行字元\n
 同時也會在字串的結尾加上\0
 假設Inputfile中的一列是"511"
 fgets讀進來的一列是"511\n\0"

 假設InputFile是
 511
 450001

 temp= 511

 strlen(temp) = 511\n = 4

 data[0] 記憶體配置 = 5 (4+1) [][][][][]
 data[0] = memset = [0][0][0][0][]
 strncpy( data[0], temp, strlen(temp)-1 ); >> data[0]=[5][1][1][0][?]

假設在file中有一筆資料是511,在這段程式碼中，fgets 函數會從 file 讀取一行，並將其存儲在 temp 中。fgets 會讀取換行符（如果存在），並在讀取的字串的結尾添加一個 null 字元（\0）。

因此，如果 file 中的一行是 "511"，那麼 fgets 將讀取 "511\n"，並在其後添加一個 null 字元，所以 temp 將包含 "511\n\0"。

然後，strlen(temp) 將返回 temp 中的字元數，不包括結尾的 null 字元。所以，對於 "511\n\0"，strlen(temp) 將返回 4。

最後，malloc 將分配 strlen(temp) + 1 個字元的空間。這裡的 "+1" 是為了結尾的 null 字元。所以，對於 "511\n\0"，malloc 將分配 5 個字元的空間。

當data[0]被分配五個字元的空間後以[][][][][]來表示
接著再進行memset( data[0], 0, strlen(temp) );
所以data[0]=[0][0][0][0][]
最後 strncpy( data[0], temp, strlen(temp)-1 ); >> data[0]=[5][1][1][0][?]

[print出來]
temp: 511

strlen(temp): 4
data[0]: 511
size of data[0]: 3

![image](https://raw.githubusercontent.com/sogood0425/PDM/main/images/fgets.png)

***
fgets 函數確實會讀取一行，並且如果存在換行符，會包括換行符，最後添加 null 字元 \0。所以如果 input.txt 的一行是 "511"，fgets 會讀取 "511\n"，並且 temp 將包含 "511\n\0"。

strlen(temp) 返回字符串的長度，不包括結尾的 null 字元。所以對於 "511\n\0"，strlen(temp) 返回 4。

malloc 應該分配 strlen(temp) + 1 個字元的空間，以確保有足夠的空間存放字符串和結尾的 null 字元。

memset(data[0], 0, strlen(temp)); 這裡有一點需要注意，strlen(temp) 是 4，這樣 memset(data[0], 0, 4); 只會設置 data[0] 的前 4 個字元為 0，但應該設置 strlen(temp) + 1 個字元，這樣 data[0] 會初始化為 [0][0][0][0][0]。

strncpy(data[0], temp, strlen(temp) - 1); 這裡確實有個問題，它將複製 strlen(temp) - 1 個字元，即 3 個字元 "511"，然後 data[0] 將變為 [5][1][1][0][?]。最後一個 ? 是未初始化的記憶體。因此，應該手動在 data[0][strlen(temp) - 1] 位置添加 null 字元以確保字符串正確終止。
***
