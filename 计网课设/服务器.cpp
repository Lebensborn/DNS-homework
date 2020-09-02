#include<stdio.h> 
#include<winsock.h> 
#include<windows.h> 
#include<stdlib.h> 
#include<string> 
#include<assert.h> 
//#include<iostream>
#include<conio.h>
#include<fstream>
#include<thread>
#include<vector>
#include"查询域名.h"

//using namespace std;
#define SIZE 128 //定义用于接收数据的数组的大小
//int usernum = 0;//当前注册用户的人数
//extern dns_servers[];
//extern void ngethostbyname(unsigned char*, int, int);

void sendmsg(char* buf,int clifd) {
	send(clifd, buf, strlen(buf), 0);
}

void getmsg(char* a,int clifd) {
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
	int* a = (int*)lpParamter;
	int clifd = a[0];
	int answer;
	char question[SIZE] = { "请输入要查询的域名\n" };
	char domain[SIZE];
	while (1) {
		sendmsg(question,clifd);
		getmsg(domain,clifd);
		//由域名获得IPv4地址，A是查询类型
		ngethostbyname((unsigned char*)domain, 1, clifd);
	}
	return 0L;
}


int main(int argc, char* argv[])

{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) { perror("WSAStartup error"); }
	int listenfd; //tcp流程
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(-1 != listenfd);
	struct sockaddr_in ser;
	ser.sin_family = AF_INET;
	ser.sin_port = htons(6000);
	ser.sin_addr.s_addr = inet_addr("10.128.193.151");
	int res = 0;
	res = bind(listenfd, (struct sockaddr*) & ser, sizeof(ser));
	assert(-1 != res);
	res = listen(listenfd, 5);
	assert(-1 != res);
	char dns_servername[100];
	printf("请输入DNS服务器的IP：");
	scanf("%s", dns_servername);
	strcpy(dns_servers[0], dns_servername);
	while (1)
	{
		struct sockaddr_in cli;
		int len = sizeof(cli);
		int clifd = accept(listenfd, (struct sockaddr*) & cli, &len);
		//cout << "accept" << endl;
		if (clifd < 0)
		{
			printf("link error.\n");
			continue;
		}
		//alluser[usernum].clifd = clifd;
		//alluser[usernum].num = usernum;
		//thread my_thread(alluser[usernum]);
		//my_thread.detach();
		CreateThread(NULL, 0, comm_client, &clifd, 0, NULL);
		//usernum++;
	}
	closesocket(listenfd);
	return 0;
}