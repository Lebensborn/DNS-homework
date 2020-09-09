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
char dns_servers[1][16]; //存放DNS服务器的IP
int dns_server_count = 0;
/*
**DNS报文首部
**这里使用了位域
*/
typedef struct DNS_HEADER
{
	unsigned short id;	  //会话标识
	unsigned char rd : 1; // 表示期望递归
	unsigned char tc : 1; // 表示可截断的
	unsigned char aa : 1; //  表示授权回答
	unsigned char opcode : 4;
	unsigned char qr : 1;	 //  查询/响应标志，0为查询，1为响应
	unsigned char rcode : 4; //应答码
	unsigned char cd : 1;
	unsigned char ad : 1;
	unsigned char z : 1;	   //保留值
	unsigned char ra : 1;	   // 表示可用递归
	unsigned short q_count;	   // 表示查询问题区域节的数量
	unsigned short ans_count;  // 表示回答区域的数量
	unsigned short auth_count; // 表示授权区域的数量
	unsigned short add_count;  // 表示附加区域的数量
} HEADER;

/*
**DNS报文中查询问题区域
*/
struct QUESTION
{
	unsigned short qtype;  //查询类型
	unsigned short qclass; //查询类
};
typedef struct
{
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;

/*
**DNS报文中回答区域的常量字段
*/
//编译制导命令
#pragma pack(push, 1) //保存对齐状态，设定为1字节对齐
struct R_DATA
{
	unsigned short type;	 //表示资源记录的类型
	unsigned short _class;	 //类
	unsigned int ttl;		 //表示资源记录可以缓存的时间
	unsigned short data_len; //数据长度
};
#pragma pack(pop) //恢复对齐状态

/*
**DNS报文中回答区域的资源数据字段
*/
struct RES_RECORD
{
	unsigned char *name;	 //资源记录包含的域名
	struct R_DATA *resource; //资源数据
	unsigned char *rdata;
};

/*
**DNS报文中查询区域的查询类型
*/
#define A 1 //查询类型，表示由域名获得IPv4地址

void ngethostbyname(unsigned char *, int);
void ChangetoDnsNameFormat(unsigned char *, unsigned char *);

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

int DNS_TABLE_READIN(char *filename)
{
	int BOUNDARY=0;
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
void URL_READIN(char *recvbuf, int recvnum)
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
int DOMAIN_MATCH(char *URL, int num)
{
	int flag = -1;
	char *domain;

	for (int i = 0; i < num; i++)
	{
		domain = (char *)DNS_TABLE[i].domain.c_str();//domain的首地址
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
unsigned short NewID_WRITEIN (unsigned short TempID, ,BOOL TempFlag,SOCKADDR_IN TempAddr)
{
	TRANSLATOR[IDNum].FormerID = TempID;
	TRANSLATOR[IDNum].flag = TempFlag;
	TRANSLATOR[IDNum].client  = TempAddr;
	IDNum++;
	return (unsigned short)(IDNum-1);	//以表中下标作为新的ID
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
	    if(DNS_TABLE[flag].IP == "0.0.0.0")  //不良网站拦截
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
		    cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
		    cout << "    ";
			//打印IP
			cout.setf(ios::left);
		    cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_TABLE[flag].IP << endl;
		}
	}
}

int main(int argc, char *argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		perror("WSAStartup error");
	}
	unsigned char hostname[100];
	char dns_servername[100];
	printf("请输入DNS服务器的IP：");
	scanf("%s", dns_servername);
	strcpy(dns_servers[0], dns_servername);
	printf("请输入要查询IP的主机名：");
	scanf("%s", hostname);

	//由域名获得IPv4地址，A是查询类型
	ngethostbyname(hostname, A);

	return 0;
}

/*
**实现DNS查询功能
*/
void ngethostbyname(unsigned char *host, int query_type)
{
	unsigned char buf[14776], *qname, *reader;
	int i, j, stop, s;

	struct sockaddr_in a; //地址

	struct RES_RECORD answers[20], auth[20], addit[20]; //回答区域、授权区域、附加区域中的资源数据字段
	struct sockaddr_in dest;							//地址

	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;

	printf("\n所需解析域名：%s", host);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //建立分配UDP套结字

	dest.sin_family = AF_INET;						  //IPv4
	dest.sin_port = htons(53);						  //53号端口
	dest.sin_addr.s_addr = inet_addr(dns_servers[0]); //DNS服务器IP

	dns = (struct DNS_HEADER *)&buf;
	/*设置DNS报文首部*/
	dns->id = (unsigned short)htons(getpid()); //id设为进程标识符
	dns->qr = 0;							   //查询
	dns->opcode = 0;						   //标准查询
	dns->aa = 0;							   //不授权回答
	dns->tc = 0;							   //不可截断
	dns->rd = 1;							   //期望递归
	dns->ra = 0;							   //不可用递归
	dns->z = 0;								   //必须为0
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;			 //没有差错
	dns->q_count = htons(1); //1个问题
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	//qname指向查询问题区域的查询名字段
	qname = (unsigned char *)&buf[sizeof(struct DNS_HEADER)];

	ChangetoDnsNameFormat(qname, host);																//修改域名格式
	qinfo = (struct QUESTION *)&buf[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

	qinfo->qtype = htons(query_type); //查询类型为A
	qinfo->qclass = htons(1);		  //查询类为1

	//向DNS服务器发送DNS请求报文
	printf("\n\n发送报文中...");
	if (sendto(s, (char *)buf, sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
	{
		perror("发送失败！");
	}
	printf("发送成功！");

	//从DNS服务器接受DNS响应报文
	i = sizeof dest;
	printf("\n接收报文中...");
	if (recvfrom(s, (char *)buf, 14776, 0, (struct sockaddr *)&dest, (int *)&i) < 0)
	{
		perror("接收失败！");
	}
	else
		printf("接收成功！");

	dns = (struct DNS_HEADER *)buf;

	//将reader指向接收报文的回答区域
	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION)];

	printf("\n\n响应报文包含: ");
	printf("\n %d个问题", ntohs(dns->q_count));
	printf("\n %d个回答", ntohs(dns->ans_count));
	printf("\n %d个授权服务", ntohs(dns->auth_count));
	printf("\n 应答码%d", ntohs(dns->rcode));
	printf("\n %d个附加记录\n\n", ntohs(dns->add_count));

	/*
	**解析接收报文
	*/
	reader = reader + sizeof(short); //short类型长度为32为，相当于域名字段长度，这时reader指向回答区域的查询类型字段
	answers[i].resource = (struct R_DATA *)(reader);
	//reader = reader + sizeof(struct R_DATA);//指向回答问题区域的资源数据字段
	printf("type:%d\n", ntohs(answers[i].resource->type));
	printf("长度%u\n", ntohs(answers[i].resource->data_len));
	if (ntohs(answers[i].resource->type) != A)
	{
		printf("7\n");
		reader = reader + sizeof(struct R_DATA) + ntohs(answers[i].resource->data_len) + sizeof(short);
		answers[i].resource = (struct R_DATA *)(reader);
		printf("type:%d\n", ntohs(answers[i].resource->type));
		printf("长度%u\n", ntohs(answers[i].resource->data_len));
	}
	reader = reader + sizeof(struct R_DATA);
	if (ntohs(answers[i].resource->type) == A) //判断资源类型是否为IPv4地址
	{
		printf("长度%u\n", ntohs(answers[i].resource->data_len));
		answers[i].rdata = (unsigned char *)malloc(ntohs(answers[i].resource->data_len)); //资源数据
		for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
		{
			answers[i].rdata[j] = reader[j];
		}
		answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
		reader = reader + ntohs(answers[i].resource->data_len);
	}

	//显示查询结果
	if (ntohs(answers[i].resource->type) == A) //判断查询类型IPv4地址
	{
		long *p;
		p = (long *)answers[i].rdata;
		a.sin_addr.s_addr = *p;
		printf("IPv4地址:%s\n", inet_ntoa(a.sin_addr));
	}

	return;
}

/*
**从www.baidu.com转换到3www5baidu3com
*/
void ChangetoDnsNameFormat(unsigned char *dns, unsigned char *host)
{
	int lock = 0, i;
	strcat((char *)host, ".");

	for (i = 0; i < strlen((char *)host); i++)
	{
		if (host[i] == '.')
		{
			*dns++ = i - lock;
			for (; lock < i; lock++)
			{
				*dns++ = host[lock];
			}
			lock++;
		}
	}
	*dns++ = '\0';
}