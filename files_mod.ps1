#files_mod.ps1 -drive driveletter -needmail $false
#Set-ExecutionPolicy Unrestricted

# Pass drive letter

Param
(
  [string]$drive,
  [bool]$needmail,
)



$MAILREUQIRED = $needmail


if($MAILREUQIRED){
  $PCName = $env:COMPUTERNAME
  $EmailFrom = "$PCName@mailID"
  $CC = ""
  $EmailTo = "" 
  $mailuser = ""  # For authentication
  $mailpass = ""  # For authentication
  $mailpass = $mailpass | ConvertTo-SecureString -AsPlainText -Force
  $mailcred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $mailuser, $mailpass
  $SMTPServer = ""
}



# Add extension list which you want to exclude from showing  example ".al"
# Example:
# $extlist = @(
# ".txt", ".ps1", ".cpp"
# )

$extlist = @(
  ".txt", ".ps1", ".cpp"
)


if([IntPtr]::size -eq 8) {
  $tmp_list=mod_files_x64.exe -drive $drive -startusn 0 | Select-Object -Unique | ForEach-Object {[System.IO.FileInfo]$_} 
}
else {
  $tmp_list=mod_files.exe -drive $drive -startusn 0 | Select-Object -Unique | ForEach-Object {[System.IO.FileInfo] $_} 
}

# Exclude folders, files which dont have extension and files whose extension is added in extlist.
$files_mod = $tmp_list | Where-Object {!$_.PSIsContainer -and $extlist -notcontains $_.Extension -and  $_.Extension -ne ''}
$grouped_ext = $files_mod | Group-Object extension | Where-Object {$_.Count -gt 20}
$total_count = 0

# For checking files are text or binary
$nonPrintable = [char[]] (0..8 + 10..31 + 127 + 129 + 141 + 143 + 144 + 157)


# Add modified files list in registry for comparing in next run.
if ( -not (Test-Path "HKLM:\SOFTWARE\MODFILES\FilesMod$drive"))
{
  try { New-Item -Path "HKLM:\SOFTWARE\MODFILES" -Name "FilesMod$drive" -Force -ErrorAction Continue | Out-Null}
  catch { }
}
else {
  $lastmodifiedexts = (Get-ItemProperty "HKLM:\SOFTWARE\MODFILES\FilesMod$drive").PSObject.Properties | Where-Object { $_.name -like '.*'} | Select-Object Name, Value
  try { Remove-Item -Path "HKLM:\SOFTWARE\MODFILES\FilesMod$drive" -ErrorAction Continue | Out-Null }
  catch { }
  try { New-Item -Path "HKLM:\SOFTWARE\MODFILES" -Name "FilesMod$drive" -Force -ErrorAction Continue | Out-Null }
  catch { }  
}

foreach($grp_element in $grouped_ext) { $total_count += $grp_element.Count}

if ($total_count -gt 20){
  $filelist = ""
  $cnt = 0
  foreach($grp_element in $grouped_ext) {
    try {
      New-ItemProperty -Path "HKLM:\SOFTWARE\MODFILES\FilesMod$drive" -Name $grp_element.name -Value $grp_element.Count -PropertyType "String" -Force | Out-Null
    }
    catch {}
    foreach($lastext in $lastmodifiedexts) { if ( $grp_element.name -eq $lastext.name){
      # Check 5 random modified files to check text files or binary files
      $random_files = Get-Random -InputObject $grp_element.group -count 5
      $bincount = 0
      foreach($fl in $random_files){
        $lines = Get-Content $fl -ErrorAction Ignore -TotalCount 5
        $result = @($lines | Where-Object { $_.IndexOfAny($nonPrintable) -ge 0 })
        if($result.Count -gt 0){ $bincount +=1}
      }
      $cnt += $lastext.Value
      if($bincount -ge 4){
        $binary = $True
        $grp_element.group | ForEach-Object { $cnt += 1; $filelist = $filelist + $_.Fullname + [Environment]::NewLine}}
      }
    }
  }
  if ($MAILREUQIRED) {
    $EmailSubject = "Multiple files are modified on $PCName"
    $EmailBody = "<pre>Please check around " + $cnt + " files are modified in last 10 minutes on computer <b>$PCName</b>.
  
    <b>Following are the list of files:
    $filelist </b>
    <br/><br/>This is auto generated mail for your information and necessary action.</pre>"
   
    if ( $cnt -ge 40 -and $binary){
      #IF COUNT OF FILES ARE DOUBLE THEN MAIL THE LIST.
      Write-Output "Sending Email"
      Send-MailMessage -From $EmailFrom -To $EmailTo -Subject $EmailSubject -body $EmailBody -SmtpServer $SMTPServer -BodyAsHtml -Cc $CC -Credential $mailcred
    }
  }
  else {
    $filelist
  }
    
  if ( $cnt -ge 2500 -and $binary){
    #SHUTDOWN IF COUNT OF FILES ARE MORE THAN 2500
    Write-Output "Shutting down machine"
	C:\Windows\System32\shutdown.exe -s -t 2 -f
  }
  
}

Write-Output $files_mod.Count
Write-Output $total_count

exit 0