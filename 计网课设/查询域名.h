#include<stdio.h> //printf()
#include<winsock.h> 
#include<windows.h> //inet_addr()
#include<stdlib.h> //exit()
#include<string> //strlen()
#include<assert.h> //assert()
#include<conio.h>
#include<fstream>
#include<process.h>

char dns_servers[1][16];//存放DNS服务器的IP
int dns_server_count = 0;
#define A 1 //查询类型，表示由域名获得IPv4地址 

void ngethostbyname(unsigned char*, int,int);