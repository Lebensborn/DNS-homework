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
using namespace std;
#pragma  comment(lib, "Ws2_32.lib") 
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

/*
**���������
*/

int DNS_TABLE_READIN(char* filename)
{
	int BOUNDARY = 0;
	unsigned char TABLE[MAXSIZE];
	ifstream infile(filename, ios::in);//���ļ�
	if (!infile)
	{
		cout << "�ļ��򿪴���" << endl;
		return 0;
	}
	int END_SIGNAL = 0;
	while (getline(infile, TABLE[END_SIGNAL]))
		END_SIGNAL++;
	if (END_SIGNAL >= MAXSIZE - 1)
		cout << "�ڴ�������" << endl;
	END_SIGNAL--;
	for (int i = 0; i < END_SIGNAL; i++)
	{
		BOUNDARY = TABLE[i].flag(' ');
		if (BOUNDARY > TABLE[i].size())
			cout << "�ļ���ʽ�����ϡ�" << endl;
		else
		{
			DNS_TABLE[i].IP = TABLE[i].substr(0, BOUNDARY);
			DNS_TABLE[i].domain = TABLE[i].substr(BOUNDARY + 1);
		}
	}
	infile.close(); //�ر��ļ�
	cout << "�ļ�������ɡ�" << endl;
	return END_SIGNAL;
}
/*
**��ȡ����
*/
void URL_READIN(char* recvbuf, int recvnum)
{
	char FORMER_URL[LENGTH];
	int i = 0, k = 0;
	memset(URL, 0, LENGTH);
	memcpy(FORMER_URL, &(recvbuf[sizeof(HEADER)]), recvnum - 16); //��ȡ�������е�������ʾ
	//����ת��
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
**ƥ�������������±�
*/
int DOMAIN_MATCH(char* URL, int num)
{
	int flag = -1;
	char* domain;

	for (int i = 0; i < num; i++)
	{
		domain = (char*)DNS_TABLE[i].domain.c_str();//domain���׵�ַ
		if (strcmp(domain, URL) == 0)
		{
			flag = i;
			break;
		}
	}

	return flag;
}
/*
**ת��Ϊ�µ�ID��������Ϣд��IDת������
*/
unsigned short NewID_WRITEIN(unsigned short TempID, , BOOL TempFlag, SOCKADDR_IN TempAddr)
{
	TRANSLATOR[IDNum].FormerID = TempID;
	TRANSLATOR[IDNum].flag = TempFlag;
	TRANSLATOR[IDNum].client = TempAddr;
	IDNum++;
	return (unsigned short)(IDNum - 1);	//�Ա����±���Ϊ�µ�ID
}
/*
**��ӡ����ת������Ϣ
*/
void PRINT_INFO(unsigned short NewID, int flag)
{
	//��ӡת�����µ�ID
	cout.setf(ios::left);
	cout << setiosflags(ios::left) << setw(4) << setfill(' ') << NewID;
	cout << "    ";
	//�ڱ���û���ҵ�DNS�����е�����
	if (flag == -1)
	{
		//�м̹���
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "�м�";
		cout << "    ";
		//��ӡ����
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << URL;
		cout << "    ";
		//��ӡIP
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
	}

	//�ڱ����ҵ�DNS�����е�����
	else {
		if (DNS_TABLE[flag].IP == "0.0.0.0")  //������վ����
		{
			//���ι���
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "����";
			cout << "    ";
			//��ӡ����(��*)
			cout.setf(ios::left);
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << URL;
			cout << "    ";
			//��ӡIP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
		}

		//�������Ϊ��ͨIP��ַ������ͻ����������ַ
		else {
			//����������
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "������";
			cout << "    ";
			//��ӡ����
			cout.setf(ios::left);
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << URL;
			cout << "    ";
			//��ӡIP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_TABLE[flag].IP << endl;
		}
	}
}