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
#include "��ѯ����.h"

int DNS_TABLE_READIN(char* filename);
void URL_READIN(char* recvbuf, int recvnum);
int DOMAIN_MATCH(char* URL, int num);
unsigned short NewID_WRITEIN(unsigned short TempID,  BOOL TempFlag, SOCKADDR_IN TempAddr);
void PRINT_INFO(unsigned short NewID, int flag);
/*
**DNS������ṹ
*/
typedef struct TABLE_STRUCTURE
{
	unsigned char IP;	  //IP��ַ
	unsigned char domain; //����
}STRUCTURE;

/*
**DNSת����ṹ
*/
typedef struct TABLE_TRANSLATION
{
	unsigned short FormerID; //ԭID
	BOOL flag;				 //���ת����
	SOCKADDR_IN client;		 //�ͻ��׽��ֵ�ַ
}TRANSLATION;
#define MAXSIZE 1000
#define LENGTH 65
STRUCTURE DNS_TABLE[MAXSIZE];//DNS����������
TRANSLATION TRANSLATOR[MAXSIZE]; //IDת����
int IDNum = 0;//ת�����е���Ŀ����
char URL[LENGTH];//����
