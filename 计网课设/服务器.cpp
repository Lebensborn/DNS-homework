#include<stdio.h> 
#include<winsock.h> 
#include<windows.h> 
#include<stdlib.h> 
#include<string> 
#include<assert.h> 
#include<iostream>
#include<conio.h>
#include<fstream>
#include<thread>
#include<vector>
#include"��ѯ����.h"

using namespace std;
#define SIZE 128 //�������ڽ������ݵ�����Ĵ�С
int usernum = 0;//��ǰע���û�������

class user {
public:
	int num;//�û����
	int clifd;//socket��ʶ
	void operator()();//���ڴ�����Ӧ�û����߳�
	void getmsg(char* a);//�ӿͻ��˽�����Ϣ�������ַ�����
	void sendmsg(char* buf);//������Ϣ
};
user alluser[20];

void user::operator()() {
	int answer;
	char question[SIZE] = { "������Ҫ��ѯ������\n" };	
	char domain[SIZE];
	while (1) {
		sendmsg(question);		
		getmsg(domain);
		//���������IPv4��ַ��A�ǲ�ѯ����
		ngethostbyname((unsigned char*)domain, A,clifd);
	}	
}
void user::sendmsg(char* buf) {
	send(clifd, buf, strlen(buf), 0);
}

void user::getmsg(char* a) {
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

int main(int argc, char* argv[])

{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) { perror("WSAStartup error"); }
	int listenfd; //tcp����
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(-1 != listenfd);
	struct sockaddr_in ser;
	ser.sin_family = AF_INET;
	ser.sin_port = htons(6000);
	ser.sin_addr.s_addr = inet_addr("192.168.3.8");
	int res = 0;
	res = bind(listenfd, (struct sockaddr*) & ser, sizeof(ser));
	assert(-1 != res);
	res = listen(listenfd, 5);
	assert(-1 != res);
	char dns_servername[100];
	printf("������DNS��������IP��");
	scanf("%s", dns_servername);
	strcpy(dns_servers[0], dns_servername);
	while (1)
	{
		struct sockaddr_in cli;
		int len = sizeof(cli);
		int clifd = accept(listenfd, (struct sockaddr*) & cli, &len);
		cout << "accept" << endl;
		if (clifd < 0)
		{
			printf("link error.\n");
			continue;
		}
		alluser[usernum].clifd = clifd;
		alluser[usernum].num = usernum;
		thread my_thread(alluser[usernum]);
		my_thread.detach();
		usernum++;
	}
	closesocket(listenfd);
	return 0;
}