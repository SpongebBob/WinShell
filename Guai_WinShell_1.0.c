#define _WIN32_WINNT 0x0501

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include <Wincon.h>
#include <tlhelp32.h>
#include <malloc.h>
#include <string.h>
#include <direct.h>
#include "Guai_headfile.h"

int main()
{
	void cd_cmd(char *dir);
	void dir_cmd(char *dir);
	void ftime(FILETIME filetime);
	void GetProcessList();
	void history_cmd();
	void add_history(char *inputcmd);
	HANDLE process(int bg,char appName[]);
	BOOL killProcess(char *pid);
	BOOL WINAPI ConsoleHandler(DWORD CEvent);
	void help();

	char c,*input,*arg[2],path[BUFSIZE];
	int input_len=0,is_bg=0,i,j,k;
	HANDLE hprocess;
	DWORD dwRet;

	while (1)
	{
		for (i=0;i<2;i++)
			arg[i]=NULL;
		dwRet=GetCurrentDirectory(BUFSIZE,path);
		if (dwRet==0)
		{
			printf("GetCurrentDirectory failed (%d)\n",GetLastError() );
		}
		else if (dwRet>BUFSIZE)
		{
			printf("GetCurrentDirectory failed (buffer too small; need %d chars)\n",dwRet);
		}
		else printf("%s>", path);

		input_len=0;
		while ((c=getchar())==' '||c=='\t'||c==EOF);
		if (c=='\n')
				continue;
		while(c!='\n')
		{
			buf[input_len++]=c;
			c=getchar();
		}
		buf[input_len++]='\0';

		input= (char*) malloc ( sizeof (char) * (input_len) );
		strcpy(input,buf);

		for (i=0,j=0,k=0;i<input_len;i++)
		{
			if (input[i]==' '||input[i]=='\0')
			{
				if (j==0)
					continue;
				else 
				{
					buf[j++]='\0';
					arg[k]=(char*)malloc (sizeof(char*)j);
					strcpy(arg[k++],buf);
					j=0;
				}
			}
			else 
			{
				buf[j++]=input[i];
			}
		}

		if (strcmp(arg[0],"cd")==0)
		{
			add_history(input);
			for (i=3,j=0;i<=input_len;i++)
				buf[j++]=input[i];
			buf[j]='\0';
			arg[1]=(char*)malloc(sizeof(char)* j);
			strcpy(arg[1],buf);
			cd_cmd(arg[1]);
			free(input);
			continue;
		}

		if (strcmp(arg[0],"dir")==0)
		{
			char *route;
			add_history(input);
			if (arg[1]==NULL)
			{
				route=path;
				dir_cmd(route);
			}
			else dir_cmd(arg[1]);
			free(input);
			continue;
		}

		if (strcmp(arg[0],"tasklist")==0)
		{
			add_history(input);
			GetProcessList();
			free(input);
			continue;
		}

		if (strcmp(arg[0],"fp")==0)
		{
			add_history(input);
			if (arg[1]==NULL)
			{
				printf("没有可执行文件\n");
				free(input);
				continue;
			}
			is_bg=0;
			hprocess=process(is_bg,input);
			if (WaitForSingleObject(hprocess,INFINITE)==WAIT_OBJECT_0)
				free(input);
			continue;
		}

		if (strcmp(arg[0],"bg&")==0)
		{
			add_history(input);
			if (arg[1]==NULL)
			{
				printf("没有可执行文件\n");
				free(input);
				continue;
			}
			is_bg=1;
			process(is_bg,input);
			free(input);
			continue;
		}

		if (strcmp(arg[0],"taskkill")==0)
		{
			BOOL success;
			add_history(input);
			success=killProcess(arg[1]);
			if (!success)
				printf("kill process failed\n");
			free(input);
			continue;
		}

		if (strcmp(arg[0],"history")==0)
		{
			add_history(input);
			history_cmd();
			free(input);
			continue;
		}

		if (strcmp(arg[0],"exit")==0)
		{
			add_history(input);
			printf("Bye bye!\n");
			free(input);
			break;
		}

		if (strcmp(arg[0],"help")==0)
		{
			add_history(input);
			help();
			free(input);
			continue;
		}
		else
		{
			printf("please type in correct command!\n");
			continue;
		}
	}
}

void cd_cmd(char *route)
{
	if (!SetCurrentDirectory(route))
	{
		printf(TEXT("SetCurrentDirectory failed (%d)\n") , GetLastError());
	}
}

void dir_cmd(char *route)
{
	WIN32_FIND_DATA FindFileData;
	files_Content head, *p, *q;
	HANDLE hFind=INVALID_HANDLE_VALUE;
	DWORD dwError;
	char volume_name[256];
	int file=0,dir=0;
	_int64 sum_file=0;

	_int64 l_user,l_sum,l_idle;
	unsigned long volume_number;
	char *DirSpec[3];

	DirSpec[0]=(char*)malloc(sizeof(char)*2);
	strncpy (DirSpec[0],route,1);
	*(DirSpec[0]+1)='\0';
	DirSpec[1]=(char*)malloc(sizeof(char)*4);
	strcpy(DirSpec[1],DirSpec[0]);
	strncat(DirSpec[1],":\\",3);
	DirSpec[2]=(char*)malloc(sizeof(char)*(strlen(route)+2));
	DirSpec[3]=(char*)malloc(sizeof(char)*(strlen(route)+5));
	strcpy(DirSpec[2],route);
	strcpy(DirSpec[3],route);
	strncat(DirSpec[2],"\\",2);
	strncat(DirSpec[3],"\\*.*",5);
	hFind=FindFirstFile(DirSpec[3],&FindFileData);
	if (hFind==INVALID_HANDLE_VALUE)
	{
		printf("Invalid file handle. Error is %u\n",GetLastError());
	}
	else 
	{
		GetVolumeInformation(DirSpec[1],volume_name,50,&volume_number,NULL,NULL,NULL,10);
		if (strlen(volume_name)==0)
			printf("驱动器 %s 中的卷没有标签。\n",DirSpec[0]);
		else 
			printf("驱动器 %s 中的卷是 %s \n",DirSpec[0],volume_name);
		printf(" 卷的序列号是 %X \n\n", volume_number);
		printf("卷的序列号是 %X \n\n", DirSpec[2]);
		head.time=FindFileData.ftCreationTime;
		strcpy(head.name,FindFileData.cFileName);
		if (FindFileData.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
		{
			head.type=0;
			dir++;
		}
		else
		{
			head.type=1;
			head.size=FindFileData.nFileSizeLow;
			file++;
			sum_file += FindFileData.nFileSizeLow;
		}
		p=&head;
		while(FindNextFile(hFind,&FindFileData)!=0)
		{
			q=(files_Content*)malloc(sizeof(files_Content));
			q->time=FindFileData.ftCreationTime;
			strcpy(q->name,FindFileData.cFileName);
			if (FindFileData.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
			{
				q->type=0;
				dir++;
			}
			else
			{
				q->type=1;
				q->size=FindFileData.nFileSizeLow;
				file++;
				sum_file += FindFileData.nFileSizeLow;
			}
			p->next=q;
			p=q;
		}
		p->next=NULL;
		p=&head;
		while (p!=NULL)
		{
			ftime(p->time);
			if (p->type==0)
				printf("\t<DIR>\t\t");
			else
				printf("\t\t%9lu", p->size);
			printf("\t%s\n",p->name);
			p=p->next;
		}
		free(p);
		printf("%15d 个文件\t\t%I64d 字节 \n",file,sum_file);
		GetDiskFreeSpaceEx(DirSpec[1],(PULARGE_INTEGER)&l_user,(PULARGE_INTEGER)&l_sum,(PULARGE_INTEGER)&l_idle);
		printf("%15d 个目录\t\t\t%I64d 可用字节 \n",dir,l_idle);
		dwError=GetLastError();
		FindClose(hFind);
		if (dwError!=ERROR_NO_MORE_FILES)
		{
			printf("FindNextFile error. Error is %u\n",dwError);
		}
	}
}

void ftime(FILETIME filetime)
{
	SYSTEMTIME systemtime;
	if (filetime.dwLowDateTime==-1)
	{
		wprintf(L"Never Expires ");
	}
	else 
	{
		if (FileTimeToLocalFileTime(&filetime,&filetime)!=0)
		{
			if (FileTimeToSystemTime(&filetime,&systemtime)!=0)
			{
				char str[50];
				wsprintf(str, "%d-%02d-%02d %02d:%02d",systemtime.wYear,systemtime.wMonth,systemtime.wDay,systemtime.wHour,systemtime.wMinute);
				printf("%s",str);
			}
			else 
			{
				wprintf(L"FileTimeToSystemTime failed ");
			}
		}
		else
		{
			wprintf(L"FileTimeToLocalFileTime failed ");
		}
	}
}

void GetProcessList()
{
	HANDLE hProcessSnap=NULL;
	PROCESSENTRY32 pe32={0};
	hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hProcessSnap==INVALID_HANDLE_VALUE)
		printf("\nCreateToolhelp32Snapshot() failed:%d",GetLastError());
	pe32.dwSize=sizeof(PROCESSENTRY32);
	if (Process32First(hProcessSnap,&pe32))
	{
		DWORD dwPriorityClass;
		printf("\n优先级\t\t进程\t\t线程\t\t进程名\n");
		do
		{
			HANDLE hProcess;
			hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pe32.th32ProcessID);
			dwPriorityClass=GetPriorityClass (hProcess);
			CloseHandle(hProcess);
			printf("%d\t",pe32.pcPriClassBase);
			printf("\t%d\t",pe32.th32ProcessID);
			printf("\t%d\t",pe32.cntThreads);
			printf("\t%s\n",pe32.szExeFile);
		}
		while (Process32Next (hProcessSnap,&pe32));
	}
	else 
		printf("\nProcess32First() failed:%d",GetLastError());
	CloseHandle(hProcessSnap);
}

void add_history(char *inputcmd)
{
	envhis.end=(envhis.end+1) % HISNUM;
	if (envhis.end==envhis.start)
	{
		envhis.start=(envhis.start+1) % HISNUM;
	}
	strcpy(envhis.his_cmd[envhis.end],inputcmd);
}

void history_cmd()
{
	int i,j=1;
	if (envhis.start==envhis.end);
	else if (envhis.start<envhis.end)
	{
		for (i=envhis.start+1;i<=envhis.end;i++)
		{
			printf("%d\t%s\n",j,envhis.his_cmd[i]);
			j++;
		}
	}
	else 
	{
		for (i=envhis.start+1;i<HISNUM;i++)
		{
			printf("%d\t%s\n",j,envhis.his_cmd[i]);
			j++;
		}
		for (i=0;i<=envhis.end+1;i++)
		{
			printf("%d\t%s\n",j,envhis.his_cmd[i]);
			j++;
		}
	}
}

HANDLE process(int bg,char appName[])
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb=sizeof(si);
	GetStartupInfo(&si);
	ZeroMemory(&pi,sizeof(pi));
	if (bg==0)
	{
		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
		{
			printf("Unable to install handler!\n");
			return NULL;
		}
		CreateProcess(NULL,appName,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		return pi.hProcess;
	}
	else 
	{
		si.dwFlags=STARTF_USESHOWWINDOW;
		si.wShowWindow=SW_HIDE;
		CreateProcess(NULL,appName,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
		return NULL;
	}	
}

BOOL killProcess(char *pid)
{
	int id,i;
	DWORD dwExitStatus;
	HANDLE hprocess;
	id=atoi(pid);
	hprocess=OpenProcess(PROCESS_TERMINATE,FALSE,id);
	GetExitCodeProcess(hprocess,&dwExitStatus);
	if (i=TerminateProcess(hprocess,dwExitStatus))
		return TRUE;
	else 
		return FALSE;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch (CEvent)
	{
		case CTRL_C_EVENT:
			break;
		case CTRL_BREAK_EVENT:
			break;
		case CTRL_CLOSE_EVENT:
			break;
		case CTRL_LOGOFF_EVENT:
			break;
		case CTRL_SHUTDOWN_EVENT:
			break;
	}
	return TRUE;
}

void help()
{
	printf("cd:切换当前目录。\n输入形式:cd ..\n\t  cd [drive:][path](cd c:\\temp)  \n注：cd命令以空格为分隔符，区分命令和参数。\n\n");
	printf("dir:显示目录中的文件和子文件列表。\n输入形式: dir \n\t dir [drive:][path](dir c:\\temp) \n注:cd命令以空格为分隔符，区分命令和参数。\n\n" );
	printf("tasklist:显示系统中当前的进程信息。\n输入形式: tasklist\n\n");
	printf("fp:创建进程并在前台执行。\n输入形式: fp\n\n");
	printf("bg&:创建进程并在后台执行。\n输入形式: bg&\n\n");
	printf("taskkill:终止进程。\n输入形式: taskkill [pid]\n注:taskkill命令以空格为分隔符，pid为进程id。\n\n");
	printf("history:显示历史命令。\n输入形式:history\n\n");
	printf("exit:退出。\n输入形式:exit\n\n");
}
