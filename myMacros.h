#pragma once
#include <stdio.h>
#include <stdlib.h>

#define CHECK_RETURN_0(p)	if(p==NULL)return 0;

#define CHECK_MSG_RETURN_0(p,msg)	{if(p==NULL){printf("%s\n",msg);return 0;}}

#define FREE_CLOSE_FILE_RETURN_0(p,fp)	{free(p);fclose(fp);return 0;}

#define CLOSE_RETURN_0(fp)	{fclose(fp);return 0;}
