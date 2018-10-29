// mod_files.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	return 0;
//}


#include <Windows.h>

#include <stdio.h>

#define BUFFER_SIZE (1024 * 1024)

HANDLE drive;
USN maxusn, lastusn;
SYSTEMTIME lt;
FILETIME ftime;
bool print_detail;

void print_reason(DWORD reason)
{
  if((reason & USN_REASON_DATA_OVERWRITE) == USN_REASON_DATA_OVERWRITE)
    printf("USN_REASON_DATA_OVERWRITE ");
  if(( reason & USN_REASON_DATA_EXTEND) == USN_REASON_DATA_EXTEND)
    printf("USN_REASON_DATA_EXTEND ");
  if(( reason & USN_REASON_DATA_TRUNCATION) == USN_REASON_DATA_TRUNCATION)
    printf("USN_REASON_DATA_TRUNCATION ");
  if(( reason & USN_REASON_NAMED_DATA_OVERWRITE) == USN_REASON_NAMED_DATA_OVERWRITE)
    printf("USN_REASON_NAMED_DATA_OVERWRITE ");
  if(( reason & USN_REASON_NAMED_DATA_EXTEND) == USN_REASON_NAMED_DATA_EXTEND)
    printf("USN_REASON_NAMED_DATA_EXTEND ");
  if(( reason & USN_REASON_NAMED_DATA_TRUNCATION) == USN_REASON_NAMED_DATA_TRUNCATION)
    printf("USN_REASON_NAMED_DATA_TRUNCATION ");
  if((reason & USN_REASON_FILE_CREATE) == USN_REASON_FILE_CREATE)
    printf("USN_REASON_FILE_CREATE ");
  if((reason & USN_REASON_FILE_DELETE) == USN_REASON_FILE_DELETE)
    printf("USN_REASON_FILE_DELETE ");
  if((reason & USN_REASON_EA_CHANGE) == USN_REASON_EA_CHANGE)
    printf("USN_REASON_EA_CHANGE ");
  if((reason & USN_REASON_SECURITY_CHANGE) == USN_REASON_SECURITY_CHANGE)
    printf("USN_REASON_SECURITY_CHANGE ");
  if((reason & USN_REASON_RENAME_OLD_NAME) == USN_REASON_RENAME_OLD_NAME)
    printf("USN_REASON_RENAME_OLD_NAME ");
  if((reason & USN_REASON_RENAME_NEW_NAME) == USN_REASON_RENAME_NEW_NAME)
    printf("USN_REASON_RENAME_NEW_NAME ");
  if((reason & USN_REASON_INDEXABLE_CHANGE) == USN_REASON_INDEXABLE_CHANGE)
    printf("USN_REASON_INDEXABLE_CHANGE ");
  if((reason & USN_REASON_BASIC_INFO_CHANGE) == USN_REASON_BASIC_INFO_CHANGE)
    printf("USN_REASON_BASIC_INFO_CHANGE ");
  if((reason & USN_REASON_HARD_LINK_CHANGE) == USN_REASON_HARD_LINK_CHANGE)
    printf("USN_REASON_HARD_LINK_CHANGE ");
  if((reason & USN_REASON_COMPRESSION_CHANGE) == USN_REASON_COMPRESSION_CHANGE)
    printf("USN_REASON_COMPRESSION_CHANGE ");
  if((reason & USN_REASON_ENCRYPTION_CHANGE) == USN_REASON_ENCRYPTION_CHANGE)
    printf("USN_REASON_ENCRYPTION_CHANGE ");
  if((reason & USN_REASON_OBJECT_ID_CHANGE) == USN_REASON_OBJECT_ID_CHANGE)
    printf("USN_REASON_OBJECT_ID_CHANGE ");
  if((reason & USN_REASON_REPARSE_POINT_CHANGE) == USN_REASON_REPARSE_POINT_CHANGE)
    printf("USN_REASON_REPARSE_POINT_CHANGE ");
  if((reason & USN_REASON_STREAM_CHANGE) == USN_REASON_STREAM_CHANGE)
    printf("USN_REASON_STREAM_CHANGE ");
  if((reason & USN_REASON_TRANSACTED_CHANGE) == USN_REASON_TRANSACTED_CHANGE)
    printf("USN_REASON_TRANSACTED_CHANGE ");
  if((reason & USN_REASON_CLOSE) == USN_REASON_CLOSE)
    printf("USN_REASON_CLOSE");

  printf("\n");
}

void print_list(int len, char ** list)
{
	printf("");
}

bool is_time_recent(LARGE_INTEGER timestamp){	
	ULARGE_INTEGER local_time;
	local_time.HighPart = ftime.dwHighDateTime;
	local_time.LowPart = ftime.dwLowDateTime;
	/*
	if(print_detail)
		print("Difference secs %llu\n", ((local_time.QuadPart - timestamp.QuadPart)/10000)/1000);
	*/

	if(((local_time.QuadPart - timestamp.QuadPart)/10000)/1000 > 300)
		return false;
	return true;
}

int mod_files(wchar_t * driveletter_w, USN  startusn)
{
   SYSTEMTIME stUTC, stLocal;
   //GetLocalTime(&lt);
   GetSystemTime(&lt);
   SystemTimeToFileTime(&lt,&ftime);
   DWORD bytecount = 1;
   void * buffer;
   USN_RECORD * recordend;
   USN_JOURNAL_DATA * journal;
   DWORDLONG filecount = 0;
   DWORD starttick, endtick;
   
   READ_USN_JOURNAL_DATA ReadData = {0, 0xFFFFFFFF, FALSE, 0, 0};
   USN_RECORD * UsnRecord;
   DWORD dwRetBytes;
   int I;   
   starttick = GetTickCount();

   //printf("Allocating memory.\n");
   buffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   //printf("Opening volume.\n");
   drive = CreateFile( driveletter_w,
               GENERIC_READ | GENERIC_WRITE,
               FILE_SHARE_READ | FILE_SHARE_WRITE,
               NULL, OPEN_EXISTING, 0, NULL);
   if( drive == INVALID_HANDLE_VALUE )
   {
      printf("CreateFile failed (%d)\n", GetLastError());
      return 0;
   }

   //printf("Calling FSCTL_QUERY_USN_JOURNAL\n");
   if (!DeviceIoControl(drive, FSCTL_QUERY_USN_JOURNAL, NULL, 0, buffer, BUFFER_SIZE, &bytecount, NULL))
   {
      printf("FSCTL_QUERY_USN_JOURNAL: %u\n", GetLastError());
      return 0;
   }

   journal = (USN_JOURNAL_DATA *)buffer;
   /* */
   if(print_detail)
   {
	   printf("UsnJournalID: %llu\n", journal->UsnJournalID);
	   printf("FirstUsn: %llu\n", journal->FirstUsn);
	   printf("NextUsn: %llu\n", journal->NextUsn);
	   printf("LowestValidUsn: %llu\n", journal->LowestValidUsn);
	   printf("MaxUsn: %llu\n", journal->MaxUsn);
	   printf("MaximumSize: %llu\n", journal->MaximumSize);
	   printf("AllocationDelta: %llu\n", journal->AllocationDelta);	   
   }
  

   ReadData.UsnJournalID = journal->UsnJournalID;
   if (startusn < journal->NextUsn)
	   ReadData.StartUsn = startusn; //1249479120; //1249433168; 
   else
	   ReadData.StartUsn =0;
   /*
   printf( "****************************************\n");
   
   */
   if(print_detail)
   {
	   printf( "****************************************\n");
	   printf( "Journal ID: %llu\n", journal->UsnJournalID );
	   printf( "FirstUsn: %llu\n\n", journal->FirstUsn );
   }
   

   I=0;
   for(;;) // for(I=0; I<=5; I++)
   {
	  //I=0;
	  //printf("Allocating memory again.\n");
	  buffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	  if (buffer == NULL)
	  {
		  printf("VirtualAlloc: %u\n", GetLastError());
		  CloseHandle(drive);
		  return 0;
	  }

      if( !DeviceIoControl( drive, FSCTL_READ_USN_JOURNAL, 
		    &ReadData, sizeof(ReadData),
            buffer, BUFFER_SIZE, &bytecount, NULL) )
      {
         
		 printf( "Read journal failed (%d)\n", GetLastError());
		 if(print_detail)
		 {
			 printf("File count: %lu\n", filecount);
			 endtick = GetTickCount();
			 printf("Ticks: %u\n", endtick - starttick);
			 printf( "Last: %llu\n", lastusn );
		 }
		 CloseHandle(drive);
         return 0;
      }

      dwRetBytes = bytecount - sizeof(USN);
	  if( dwRetBytes < sizeof(USN)){
		  if(print_detail)
		  {
			  printf("File count: %lu\n", filecount);
			  endtick = GetTickCount();
			  printf("Ticks: %u\n", endtick - starttick);
			  printf( "Last: %llu\n", lastusn );
		  }
		  CloseHandle(drive);
		  return 0;
	  }
	  if(print_detail)
	  {
		  printf("Bytes returned: %u\n", bytecount);
		  printf("dwRetBytes : %u\n", dwRetBytes);
		  printf( "****************************************\n");
	  }

      // Find the first record
      UsnRecord = (USN_RECORD *)((USN *)buffer + 1);
      recordend = (USN_RECORD *)(((BYTE *)buffer) + bytecount);

      while( dwRetBytes > 0 )      //while (record < recordend)
      {
		 if(((UsnRecord->Reason & USN_REASON_FILE_CREATE) == USN_REASON_FILE_CREATE) ||
			((UsnRecord->Reason & USN_REASON_DATA_OVERWRITE) == USN_REASON_DATA_OVERWRITE)
		   )
		 {		
			 if(is_time_recent(UsnRecord->TimeStamp)) // If time is less than 10 mins
			 {
				 //Timestamp convert
				 if(FileTimeToSystemTime((FILETIME*) &UsnRecord->TimeStamp, &stUTC))
					 SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
				 lastusn = UsnRecord->Usn;				 
				 FILE_ID_DESCRIPTOR fid;
				 ZeroMemory(&fid, sizeof(fid));	
				 fid.dwSize = sizeof(fid);
				 fid.Type = FileIdType;
				 fid.FileId.QuadPart = UsnRecord->FileReferenceNumber;
				 HANDLE handle = OpenFileById(drive, &fid, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, FILE_FLAG_BACKUP_SEMANTICS);
				 if (handle == INVALID_HANDLE_VALUE)
				 {
					 if(print_detail)
					 {
						 printf("");
						 printf("OpenFileById failed (%d)\n", GetLastError());
						 printf("Cant open file: %.*S\n", UsnRecord->FileNameLength/2, UsnRecord->FileName);
					 }
				 }
				 else
				 {	
					 char  tmp_path[1024];
					 char * path;					 
					 memset(tmp_path, 0, sizeof(tmp_path));					 
					 GetFinalPathNameByHandleA(handle, tmp_path, 1024, 0);
					 if(strlen(tmp_path)>7)
					 {
						 path = tmp_path;
						 path +=4;
						 printf("%s\n",path);						 
					 }
					 /*	*/
					 if(print_detail)
					 {
						 printf("File name: %.*S\n", UsnRecord->FileNameLength/2, UsnRecord->FileName);
						 printf("Path: %s\n",path);
						 printf("USN: %llu\n", UsnRecord->Usn);
						 printf("Filename length: %d\n", UsnRecord->FileNameLength);
						 printf("Reason: %x ::  ", UsnRecord->Reason);
						 print_reason(UsnRecord->Reason);
						 //printf("Timestamp: %llx  :: ", UsnRecord->TimeStamp); //nothing
						 //UTC Time
						 //printf("%02d:%02d:%02d %02d:%02d:%02d\t\t", stUTC.wDay, stUTC.wMonth, stUTC.wYear, stUTC.wHour, stUTC.wMinute, stUTC.wSecond);
					 
						 //Local Time
						 printf("Timestamp: %02d:%02d:%02d %02d:%02d:%02d\n", stLocal.wDay, stLocal.wMonth, stLocal.wYear, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
						 //is_time_recent(UsnRecord->TimeStamp);
						 printf("FileAttributes: %x  :: ", UsnRecord->FileAttributes);
						 if((UsnRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) 
							 printf("This is file.\n");
						 else
							 printf("This is folder.\n");
						 printf("\n");
					 }					 
				 }				 
			 } //if USNREASON is USN_REASON_DATA_OVERWRITE
		 } //if TIME less than 10mins

         dwRetBytes -= UsnRecord->RecordLength;
         filecount++;

         // Find the next record
         UsnRecord = (USN_RECORD *)(((BYTE *)UsnRecord) + UsnRecord->RecordLength);
      }
      // Update starting USN for next call
      ReadData.StartUsn = *(USN *)buffer;
   }
   endtick = GetTickCount();
   if(print_detail)
   {
	   printf("File count: %lu\n", filecount);
	   printf("Ticks: %u\n", endtick - starttick);
	   printf("Last: %llu\n", lastusn );
   }   
   CloseHandle(drive);
   return 0;
}

int main(int argc, char ** argv)
{
   USN  startusn;   
   wchar_t driveletter_w[9];
   char driveletter[9];
   startusn = 0;
   print_detail = false;

   
   --argc; ++argv;  // remove first argument i.e. program name
   if(argc < 4 || argc > 5){
	   printf("Usage:\n");
	   printf("program name -drive <Drive Letter> -startusn <USN number>\n");
	   printf("OR\n");
	   printf("program name -drive <Drive Letter> -startusn <USN number>\n");
	   return 0;
   }

   while(argc>0)
   {
	   if (strcmp(argv[0],"-drive")==0){
		   //mbstowcs(&driveletter_w, argv[1], 1);
		   strcpy(driveletter,"\\\\.\\");
		   strcat(driveletter,argv[1]);
		   strcat(driveletter,":");
		   mbstowcs(driveletter_w, driveletter, 9);
	   }
	   if (strcmp(argv[0],"-startusn")==0)
		   sscanf_s(argv[1],"%llu",&startusn);
	   if (strcmp(argv[0],"-debug")==0)
		   print_detail = true;
	   --argc; ++argv;
   }
   if(print_detail)
   {
      printf("args: %d\n",argc);
	  printf("argv: %s\n",argv[0]);
   }
   
   /*   
   printf("drive: U+%x\n", driveletter_w);
   wprintf(L"drive: %ls\n", driveletter_w);
   printf("startusn: %llu\n\n\n", startusn);
   */
   mod_files(driveletter_w, startusn);
}
