#include <stdio.h> //printf()
#include <winsock2.h>
#include <windows.h> //inet_addr()
#include <stdlib.h>	 //exit()
#include <string>	 //strlen()
#include <assert.h>	 //assert()
#include <conio.h>
#include <fstream>
#include <process.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "查询域名.h"

int DNS_TABLE_READIN(char* filename);
void URL_READIN(char* recvbuf, int recvnum);
int DOMAIN_MATCH(char* URL, int num);
unsigned short NewID_WRITEIN(unsigned short TempID,  BOOL TempFlag, SOCKADDR_IN TempAddr);
void PRINT_INFO(unsigned short NewID, int flag);
/*
**DNS解析表结构
*/
typedef struct TABLE_STRUCTURE
{
	unsigned char IP;	  //IP地址
	unsigned char domain; //域名
}STRUCTURE;

/*
**DNS转换表结构
*/
typedef struct TABLE_TRANSLATION
{
	unsigned short FormerID; //原ID
	BOOL flag;				 //完成转换否？
	SOCKADDR_IN client;		 //客户套接字地址
}TRANSLATION;
#define MAXSIZE 1000
#define LENGTH 65
STRUCTURE DNS_TABLE[MAXSIZE];//DNS域名解析表
TRANSLATION TRANSLATOR[MAXSIZE]; //ID转换表
int IDNum = 0;//转换表中的条目个数
char URL[LENGTH];//域名
