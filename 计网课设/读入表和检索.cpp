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
using namespace std;
#pragma  comment(lib, "Ws2_32.lib") 
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

/*
**读入解析表
*/

int DNS_TABLE_READIN(char* filename)
{
	int BOUNDARY = 0;
	unsigned char TABLE[MAXSIZE];
	ifstream infile(filename, ios::in);//打开文件
	if (!infile)
	{
		cout << "文件打开错误！" << endl;
		return 0;
	}
	int END_SIGNAL = 0;
	while (getline(infile, TABLE[END_SIGNAL]))
		END_SIGNAL++;
	if (END_SIGNAL >= MAXSIZE - 1)
		cout << "内存已满！" << endl;
	END_SIGNAL--;
	for (int i = 0; i < END_SIGNAL; i++)
	{
		BOUNDARY = TABLE[i].flag(' ');
		if (BOUNDARY > TABLE[i].size())
			cout << "文件格式不符合。" << endl;
		else
		{
			DNS_TABLE[i].IP = TABLE[i].substr(0, BOUNDARY);
			DNS_TABLE[i].domain = TABLE[i].substr(BOUNDARY + 1);
		}
	}
	infile.close(); //关闭文件
	cout << "文件读入完成。" << endl;
	return END_SIGNAL;
}
/*
**获取域名
*/
void URL_READIN(char* recvbuf, int recvnum)
{
	char FORMER_URL[LENGTH];
	int i = 0, k = 0;
	memset(URL, 0, LENGTH);
	memcpy(FORMER_URL, &(recvbuf[sizeof(HEADER)]), recvnum - 16); //获取请求报文中的域名表示
	//域名转换
	while (i < strlen(FORMER_URL))
	{
		if (FORMER_URL[i] > 0 && FORMER_URL[i] <= 63)
			for (int j = FORMER_URL[i], i++; j > 0; j--, i++, k++)
				URL[k] = FORMER_URL[i];
		if (FORMER_URL[i] != 0)
		{
			URL[k] = '.';
			k++;
		}
	}
	URL[k] = '\0';
}
/*
**匹配域名，返回下表
*/
int DOMAIN_MATCH(char* URL, int num)
{
	int flag = -1;
	char* domain;

	for (int i = 0; i < num; i++)
	{
		domain = (char*)DNS_TABLE[i].domain.c_str();//domain的首地址
		if (strcmp(domain, URL) == 0)
		{
			flag = i;
			break;
		}
	}

	return flag;
}
/*
**转换为新的ID，并将信息写入ID转换表中
*/
unsigned short NewID_WRITEIN(unsigned short TempID, , BOOL TempFlag, SOCKADDR_IN TempAddr)
{
	TRANSLATOR[IDNum].FormerID = TempID;
	TRANSLATOR[IDNum].flag = TempFlag;
	TRANSLATOR[IDNum].client = TempAddr;
	IDNum++;
	return (unsigned short)(IDNum - 1);	//以表中下标作为新的ID
}
/*
**打印所有转换后信息
*/
void PRINT_INFO(unsigned short NewID, int flag)
{
	//打印转换后新的ID
	cout.setf(ios::left);
	cout << setiosflags(ios::left) << setw(4) << setfill(' ') << NewID;
	cout << "    ";
	//在表中没有找到DNS请求中的域名
	if (flag == -1)
	{
		//中继功能
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "中继";
		cout << "    ";
		//打印域名
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << URL;
		cout << "    ";
		//打印IP
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
	}

	//在表中找到DNS请求中的域名
	else {
		if (DNS_TABLE[flag].IP == "0.0.0.0")  //不良网站拦截
		{
			//屏蔽功能
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "屏蔽";
			cout << "    ";
			//打印域名(加*)
			cout.setf(ios::left);
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << URL;
			cout << "    ";
			//打印IP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
		}

		//检索结果为普通IP地址，则向客户返回这个地址
		else {
			//服务器功能
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "服务器";
			cout << "    ";
			//打印域名
			cout.setf(ios::left);
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << URL;
			cout << "    ";
			//打印IP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_TABLE[flag].IP << endl;
		}
	}
}