# modfiles

The modfiles is small windows application that will show modified files in last 5 minutes.
Note that a single file name can displayed multiple times.
Use powershell script/cmdlet to get unique file names.
``` powershell
$tmp_list = mod_files.exe -drive c -startusn 0 | Select-Object -Unique | ForEach-Object {[System.IO.FileInfo] $_}
```

#### Command
``` powershell
mod_files.exe -drive <Drive_Letter> -startusn <0 or USN number>
```

#### Requirement
[Microsoft Visual C++ 2010 Redistributable Package](https://www.microsoft.com/en-in/download/details.aspx?id=5555)

[Microsoft Visual C++ 2010 Redistributable Package x64](https://www.microsoft.com/en-in/download/details.aspx?id=13523) For x64

