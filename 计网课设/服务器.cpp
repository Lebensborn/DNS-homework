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
#include "��ѯ����.h"
#include "�����ͼ���.h"
using namespace std;
#define SIZE 128 //�������ڽ������ݵ�����Ĵ�С
//int usernum = 0;//��ǰע���û�������
//extern dns_servers[];
//extern void ngethostbyname(unsigned char*, int, int);
#define MAXSIZE 1000
#define LENGTH 65
STRUCTURE DNS_TABLE[MAXSIZE];	 //DNS����������
TRANSLATION TRANSLATOR[MAXSIZE]; //IDת����
int IDNum = 0;					 //ת�����е���Ŀ����
char URL[LENGTH];				 //����
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

DWORD WINAPI comm_client(LPVOID lpParamter) //�ӽ������ڴ���Ϳͻ��˽���

{
	int *a = (int *)lpParamter;
	int clifd = a[0];
	int answer;
	char question[SIZE] = {"������Ҫ��ѯ������\n"};
	char domain[SIZE];
	while (1)
	{
		sendmsg(question, clifd);
		getmsg(domain, clifd);
		//���������IPv4��ַ��A�ǲ�ѯ����
		ngethostbyname((unsigned char *)domain, 1, clifd);
	}
	return 0L;
}

int main(int argc, char *argv[])

{
	WSADATA wsaData;
	SOCKET socketServer, socketLocal;			   //����DNS���ⲿDNS�����׽���
	SOCKADDR_IN serverName, clientName, localName; //����DNS���ⲿDNS����������������׽��ֵ�ַ
	char sendbuf[512];
	char recvbuf[512];
	char tablePath[100];
	char outerDns[16];
	int iLen_cli, iSend, iRecv;
	int num;

	if (argc == 1)
	{
		strcpy(outerDns, "192.168.146.2");
		strcpy(tablePath, "�˴�Ϊdnsrelay.txt·��");
	}

	else if (argc == 2)
	{
		strcpy(outerDns, argv[1]);
		strcpy(tablePath, "�˴�Ϊdnsrelay.txt·��");
	}

	else if (argc == 3)
	{
		strcpy(outerDns, argv[1]);
		strcpy(tablePath, argv[2]);
	}

	num = DNS_TABLE_READIN(tablePath); //��ȡ����������

	for (int i = 0; i < 300; i++)
	{ //��ʼ��IDת����
		TRANSLATOR[i].FormerID = 0;
		TRANSLATOR[i].flag = FALSE;
		memset(&(TRANSLATOR[i].client), 0, sizeof(SOCKADDR_IN));
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData); //��ʼ��ws2_32.dll��̬���ӿ�

	//��������DNS���ⲿDNS�׽���
	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);

	//���ñ���DNS���ⲿDNS�����׽���
	localName.sin_family = AF_INET;
	localName.sin_port = htons(53);
	localName.sin_addr.s_addr = inet_addr("127.0.0.1");

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(53);
	serverName.sin_addr.s_addr = inet_addr(outerDns);

	//�󶨱���DNS��������ַ
	if (bind(socketLocal, (SOCKADDR *)&localName, sizeof(localName)))
	{
		cout << "Binding Port 53 failed." << endl;
		exit(1);
	}
	else
		cout << "Binding Port 53 succeed." << endl;

	//����DNS�м̷������ľ������
	while (1)
	{
		iLen_cli = sizeof(clientName);
		memset(recvbuf, 0, 512);

		//����DNS����
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
			URL_READIN(recvbuf, iRecv);		   //��ȡ����
			int find = DOMAIN_MATCH(URL, num); //�������������в���

			cout << URL << endl;

			//��������������û���ҵ�
			if (find == -1)
			{
				//IDת��
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(NewID_WRITEIN(ntohs(*pID), clientName, FALSE));
				memcpy(recvbuf, &nID, sizeof(unsigned short));

				//��ӡ ʱ�� newID ���� ���� IP
				PRINT_INFO(ntohs(nID), find);

				//��recvbufת����ָ�����ⲿDNS������
				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR *)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR)
				{
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //�ͷŶ�̬������ڴ�

				//���������ⲿDNS����������Ӧ����
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR *)&clientName, &iLen_cli);

				//IDת��
				pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				int m = ntohs(*pID);
				unsigned short oID = htons(TRANSLATOR[m].FormerID);
				memcpy(recvbuf, &oID, sizeof(unsigned short));
				TRANSLATOR[m].flag = TRUE;

				//��IDת�����л�ȡ����DNS�����ߵ���Ϣ
				clientName = TRANSLATOR[m].client;

				//��recvbufת���������ߴ�
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR *)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR)
				{
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //�ͷŶ�̬������ڴ�
			}

			//���������������ҵ�
			else
			{
				//��ȡ�����ĵ�ID
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));

				//ת��ID
				unsigned short nID = NewID_WRITEIN(ntohs(*pID), clientName, FALSE);

				//��ӡ ʱ�� newID ���� ���� IP
				PRINT_INFO(nID, find);

				//������Ӧ���ķ���
				memcpy(sendbuf, recvbuf, iRecv); //����������
				unsigned short a = htons(0x8180);
				memcpy(&sendbuf[2], &a, sizeof(unsigned short)); //�޸ı�־��

				//�޸Ļش�����
				if (strcmp(DNS_TABLE[find].IP.c_str(), "0.0.0.0") == 0)
					a = htons(0x0000); //���ι��ܣ��ش���Ϊ0
				else
					a = htons(0x0001); //���������ܣ��ش���Ϊ1
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				int curLen = 0;

				//����DNS��Ӧ����
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

				//�����ĺ���Ӧ���ֹ�ͬ���DNS��Ӧ���Ĵ���sendbuf
				memcpy(sendbuf + iRecv, answer, curLen);

				//����DNS��Ӧ����
				iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR *)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR)
				{
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //�ͷŶ�̬������ڴ�
			}
		}
	}

	closesocket(socketServer); //�ر��׽���
	closesocket(socketLocal);
	WSACleanup(); //�ͷ�ws2_32.dll��̬���ӿ��ʼ��ʱ�������Դ

	// WORD wVersionRequested;
	// WSADATA wsaData;
	// int err;
	// wVersionRequested = MAKEWORD(1, 1);
	// err = WSAStartup(wVersionRequested, &wsaData);
	// if (err != 0)
	// {
	// 	perror("WSAStartup error");
	// }
	// int listenfd; //tcp����
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
	// printf("������DNS��������IP��");
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