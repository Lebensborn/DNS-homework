#include <stdio.h>
#include <winsock.h>
#include <winsock2.h> 
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <iostream>
#include <conio.h>
#include <fstream>
#include <thread>
#include <vector>
#include <iomanip>
#include "查询域名.h"
#include "读入表和检索.h"
using namespace std;
#define SIZE 128 //定义用于接收数据的数组的大小
//int usernum = 0;//当前注册用户的人数
//extern dns_servers[];
//extern void ngethostbyname(unsigned char*, int, int);
#define MAXSIZE 1000
#define LENGTH 65
STRUCTURE DNS_TABLE[MAXSIZE];	 //DNS域名解析表
TRANSLATION TRANSLATOR[MAXSIZE]; //ID转换表
int IDNum = 0;					 //转换表中的条目个数
char URL[LENGTH];				 //域名
void sendmsg(char *buf, int clifd)
{
	send(clifd, buf, strlen(buf), 0);
}

void getmsg(char *a, int clifd)
{
	//char buff[SIZE] = { '\0' };
	int n = recv(clifd, a, SIZE - 1, 0);
	if (n <= 0)
	{
		closesocket(clifd);
		printf("%d is over.\n", clifd);
		return;
	}
	printf("%d : %s\n", clifd, a);
	send(clifd, "\nOK\n", strlen("\nOK\n"), 0);
}

DWORD WINAPI comm_client(LPVOID lpParamter) //子进程用于处理和客户端交互

{
	int *a = (int *)lpParamter;
	int clifd = a[0];
	int answer;
	char question[SIZE] = {"请输入要查询的域名\n"};
	char domain[SIZE];
	while (1)
	{
		sendmsg(question, clifd);
		getmsg(domain, clifd);
		//由域名获得IPv4地址，A是查询类型
		ngethostbyname((unsigned char *)domain, 1, clifd);
	}
	return 0L;
}

int main(int argc, char *argv[])

{
	WSADATA wsaData;
	SOCKET socketServer, socketLocal;			   //本地DNS和外部DNS两个套接字
	SOCKADDR_IN serverName, clientName, localName; //本地DNS、外部DNS和请求端三个网络套接字地址
	char sendbuf[512];
	char recvbuf[512];
	char tablePath[100];
	char outerDns[16];
	int iLen_cli, iSend, iRecv;
	int num;

	if (argc == 1)
	{
		strcpy(outerDns, "192.168.146.2");
		strcpy(tablePath, "此处为dnsrelay.txt路径");
	}

	else if (argc == 2)
	{
		strcpy(outerDns, argv[1]);
		strcpy(tablePath, "此处为dnsrelay.txt路径");
	}

	else if (argc == 3)
	{
		strcpy(outerDns, argv[1]);
		strcpy(tablePath, argv[2]);
	}

	num = DNS_TABLE_READIN(tablePath); //获取域名解析表

	for (int i = 0; i < 300; i++)
	{ //初始化ID转换表
		TRANSLATOR[i].FormerID = 0;
		TRANSLATOR[i].flag = FALSE;
		memset(&(TRANSLATOR[i].client), 0, sizeof(SOCKADDR_IN));
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData); //初始化ws2_32.dll动态链接库

	//创建本地DNS和外部DNS套接字
	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);

	//设置本地DNS和外部DNS两个套接字
	localName.sin_family = AF_INET;
	localName.sin_port = htons(53);
	localName.sin_addr.s_addr = inet_addr("127.0.0.1");

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(53);
	serverName.sin_addr.s_addr = inet_addr(outerDns);

	//绑定本地DNS服务器地址
	if (bind(socketLocal, (SOCKADDR *)&localName, sizeof(localName)))
	{
		cout << "Binding Port 53 failed." << endl;
		exit(1);
	}
	else
		cout << "Binding Port 53 succeed." << endl;

	//本地DNS中继服务器的具体操作
	while (1)
	{
		iLen_cli = sizeof(clientName);
		memset(recvbuf, 0, 512);

		//接收DNS请求
		iRecv = recvfrom(socketLocal, recvbuf, sizeof(recvbuf), 0, (SOCKADDR *)&clientName, &iLen_cli);

		if (iRecv == SOCKET_ERROR)
		{
			cout << "Recvfrom Failed: " << WSAGetLastError() << endl;
			continue;
		}
		else if (iRecv == 0)
		{
			break;
		}
		else
		{
			URL_READIN(recvbuf, iRecv);		   //获取域名
			int find = DOMAIN_MATCH(URL, num); //在域名解析表中查找

			cout << URL << endl;

			//在域名解析表中没有找到
			if (find == -1)
			{
				//ID转换
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(NewID_WRITEIN(ntohs(*pID), clientName, FALSE));
				memcpy(recvbuf, &nID, sizeof(unsigned short));

				//打印 时间 newID 功能 域名 IP
				PRINT_INFO(ntohs(nID), find);

				//把recvbuf转发至指定的外部DNS服务器
				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR *)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR)
				{
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //释放动态分配的内存

				//接收来自外部DNS服务器的响应报文
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR *)&clientName, &iLen_cli);

				//ID转换
				pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				int m = ntohs(*pID);
				unsigned short oID = htons(TRANSLATOR[m].FormerID);
				memcpy(recvbuf, &oID, sizeof(unsigned short));
				TRANSLATOR[m].flag = TRUE;

				//从ID转换表中获取发出DNS请求者的信息
				clientName = TRANSLATOR[m].client;

				//把recvbuf转发至请求者处
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR *)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR)
				{
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //释放动态分配的内存
			}

			//在域名解析表中找到
			else
			{
				//获取请求报文的ID
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));

				//转换ID
				unsigned short nID = NewID_WRITEIN(ntohs(*pID), clientName, FALSE);

				//打印 时间 newID 功能 域名 IP
				PRINT_INFO(nID, find);

				//构造响应报文返回
				memcpy(sendbuf, recvbuf, iRecv); //拷贝请求报文
				unsigned short a = htons(0x8180);
				memcpy(&sendbuf[2], &a, sizeof(unsigned short)); //修改标志域

				//修改回答数域
				if (strcmp(DNS_TABLE[find].IP.c_str(), "0.0.0.0") == 0)
					a = htons(0x0000); //屏蔽功能：回答数为0
				else
					a = htons(0x0001); //服务器功能：回答数为1
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				int curLen = 0;

				//构造DNS响应部分
				char answer[16];
				unsigned short Name = htons(0xc00c);
				memcpy(answer, &Name, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short TypeA = htons(0x0001);
				memcpy(answer + curLen, &TypeA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short ClassA = htons(0x0001);
				memcpy(answer + curLen, &ClassA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long timeLive = htonl(0x7b);
				memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
				curLen += sizeof(unsigned long);

				unsigned short IPLen = htons(0x0004);
				memcpy(answer + curLen, &IPLen, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long IP = (unsigned long)inet_addr(DNS_TABLE[find].IP.c_str());
				memcpy(answer + curLen, &IP, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += iRecv;

				//请求报文和响应部分共同组成DNS响应报文存入sendbuf
				memcpy(sendbuf + iRecv, answer, curLen);

				//发送DNS响应报文
				iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR *)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR)
				{
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //释放动态分配的内存
			}
		}
	}

	closesocket(socketServer); //关闭套接字
	closesocket(socketLocal);
	WSACleanup(); //释放ws2_32.dll动态链接库初始化时分配的资源

	// WORD wVersionRequested;
	// WSADATA wsaData;
	// int err;
	// wVersionRequested = MAKEWORD(1, 1);
	// err = WSAStartup(wVersionRequested, &wsaData);
	// if (err != 0)
	// {
	// 	perror("WSAStartup error");
	// }
	// int listenfd; //tcp流程
	// listenfd = socket(AF_INET, SOCK_STREAM, 0);
	// assert(-1 != listenfd);
	// struct sockaddr_in ser;
	// ser.sin_family = AF_INET;
	// ser.sin_port = htons(6000);
	// ser.sin_addr.s_addr = inet_addr("10.128.193.151");
	// int res = 0;
	// res = bind(listenfd, (struct sockaddr *)&ser, sizeof(ser));
	// assert(-1 != res);
	// res = listen(listenfd, 5);
	// assert(-1 != res);
	// char dns_servername[100];
	// printf("请输入DNS服务器的IP：");
	// scanf("%s", dns_servername);
	// strcpy(dns_servers[0], dns_servername);
	// while (1)
	// {
	// 	struct sockaddr_in cli;
	// 	int len = sizeof(cli);
	// 	int clifd = accept(listenfd, (struct sockaddr *)&cli, &len);
	// 	//cout << "accept" << endl;
	// 	if (clifd < 0)
	// 	{
	// 		printf("link error.\n");
	// 		continue;
	// 	}
	// 	//alluser[usernum].clifd = clifd;
	// 	//alluser[usernum].num = usernum;
	// 	//thread my_thread(alluser[usernum]);
	// 	//my_thread.detach();
	// 	CreateThread(NULL, 0, comm_client, &clifd, 0, NULL);
	// 	//usernum++;
	// }
	// closesocket(listenfd);
	return 0;
}