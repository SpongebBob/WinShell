#define BUFSIZE MAX_PATH
#define HISNUM 12
char buf[BUFSIZE];

typedef struct ENV_HISTORY
{	
	int start;
	int end;
	char his_cmd[HISNUM][100];
}ENV_HISTORY;
ENV_HISTORY envhis;

typedef struct files_Content
{
	FILETIME time;
	char name[200];
	int type;
	int size;
	struct files_Content *next;
}files_Content;
