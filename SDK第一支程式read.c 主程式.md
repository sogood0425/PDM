fprintf、fscanf、strcpy這些很常看到的屬於內部程式
但有些function並不屬於C內建程式
如PM_Logon、ErrorCheck、CloseSession
PM_Logon是屬於PDM系統內建的程式，可以在SDK Reader Reference Guide.PDF找到
而ErrorCheck、CloseSession則是另外刻進去的功能，可以在msg.c找到

> int main(int argc, char **argv)
> {
> 
>    /* Open the input file */
>    InputFile = NULL;
>    InputFile = fopen(InputFilename, "r");
>    if (!InputFile)
>    {
>       fprintf(stderr, " Could not open input file [%s]\n", InputFilename);
>       return (FAIL);
>    }
> 
>    /* Open the output file */
>    OutputFile = NULL;
>    OutputFile = fopen(OutputFilename, "w");
>    if (!OutputFile)
>    {
>       fprintf(stderr, " Could not open output file [%s]\n", OutputFilename);
>       return (FAIL);
>    }
> 
>    /* Open the trace file */
>    TraceFile = NULL;
>    TraceFile = fopen(TraceFilename, "w");
>    if (!TraceFile)
>    {
>       fprintf(stderr, " Could not open trace file [%s]\n", TraceFilename);
>       return (FAIL);
>    }
> 
>    /* Open the PM environment */
>    RC = PM_Open_Env();
>    if (RC >= 0)
>       EnvOpen = 1;
> 
>    fscanf(InputFile, "%s %s %s %s",
>           User, Password, Server, Config);
> 
>    RC = PM_Logon(User, Password, Server, Config);
> 
>    if (RC >= 0)
>       LoggedOn = 1;
>    else
>       ErrorCheck("PM_Logon");
> 
>    while (fscanf(InputFile, "%s %s", pid_cond, dept) != EOF)
>       RC = LT104(pid_cond,dept);
> 
>    CloseSession();
> 
>    return (0);
> }

* main主程式裡
* argc = argv的長度 = 幾行
* char** argv = char*+\0結尾 所以必須寫成char**
* InputFilename = 讀取sdk.in檔案
* OutputFilename = 得到的資料會寫在person.data裡面
* TraceFilename = 有錯誤時可以去sdk.log檔案錯誤

#### sdk.in檔案裡面要寫四個欄位的值,且必須以空白做區隔
User, Password, Server, Config
User, Password = 員工帳號 + 密碼 或 超級帳號 plmadm01 + tel3259
Server可以選擇pmserver or pmdev
Config 則是看需要什麼 如S76 F16....

#### ErrorCheck這支function主要是來寫sdk.log看哪裡出錯

舉例 正常情況下,sdk.log會顯示
RC = 0 LoggedOn = 1
RC = 0 ---- PM_Logoff

舉例 有錯誤訊息的話,sdk.log會顯示
RC = 0 LoggedOn = -1
RC = -1 ---- PM_Logon
   Error[0] :
      Code : [apgd001]
      Text : [Datalayer Error Message 1000 --- An invalid password was entered.  Passwords are case sensitive.  Check your Caps Lock key before attempting to logon again..]
      
這裡會記錄RC = ?
在翻閱SDK Reader Reference Guide這本書裡面有提到 Error Handling

![image](https://raw.githubusercontent.com/sogood0425/PDM/main/images/Error_handling.png)

```
0 The function executed without error.
-1 An error occurred.
+1 Informational and warning messages
而PM_Error_Size則是記錄了幾種錯誤,然後相對應在PM_Error_MsgCode、PM_Error_MsgTxt會顯示
PM_Error_MsgCode代表是錯誤代碼
PM_Error_MsgTxt則是代表錯誤代碼的內容
C:\PM320BR1\mrienu\dat\nt 記載了所有有關code的錯誤代碼 這裡在記事本打開是會看不到的
因為編碼問題,直接打開skd.log看就可以知道錯誤代碼的內容
```