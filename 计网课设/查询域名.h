#include<stdio.h> //printf()
#include<winsock.h> 
#include<windows.h> //inet_addr()
#include<stdlib.h> //exit()
#include<string> //strlen()
#include<assert.h> //assert()
#include<conio.h>
#include<fstream>
#include<process.h>

char dns_servers[1][16];//���DNS��������IP
int dns_server_count = 0;
#define A 1 //��ѯ���ͣ���ʾ���������IPv4��ַ 

void ngethostbyname(unsigned char*, int,int);