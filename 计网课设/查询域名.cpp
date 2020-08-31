#include<stdio.h> //printf()
#include<winsock.h> 
#include<windows.h> //inet_addr()
#include<stdlib.h> //exit()
#include<string> //strlen()
#include<assert.h> //assert()
#include<conio.h>
#include<fstream>
#include<process.h>


extern char dns_servers[1][16];//���DNS��������IP
extern int dns_server_count ;

/*
**DNS�����в�ѯ����Ĳ�ѯ����
*/
#define A 1 //��ѯ���ͣ���ʾ���������IPv4��ַ 

void ngethostbyname(unsigned char*, int,int);
void ChangetoDnsNameFormat(unsigned char*, unsigned char*);

/*
**DNS�����ײ�
**����ʹ����λ��
*/
struct DNS_HEADER {
	unsigned short id; //�Ự��ʶ
	unsigned char rd : 1; // ��ʾ�����ݹ�
	unsigned char tc : 1; // ��ʾ�ɽضϵ� 
	unsigned char aa : 1; //  ��ʾ��Ȩ�ش�
	unsigned char opcode : 4;
	unsigned char qr : 1; //  ��ѯ/��Ӧ��־��0Ϊ��ѯ��1Ϊ��Ӧ
	unsigned char rcode : 4; //Ӧ����
	unsigned char cd : 1;
	unsigned char ad : 1;
	unsigned char z : 1; //����ֵ
	unsigned char ra : 1; // ��ʾ���õݹ�
	unsigned short q_count; // ��ʾ��ѯ��������ڵ����� 
	unsigned short ans_count; // ��ʾ�ش����������
	unsigned short auth_count; // ��ʾ��Ȩ���������
	unsigned short add_count; // ��ʾ�������������
};

/*
**DNS�����в�ѯ��������
*/
struct QUESTION {
	unsigned short qtype;//��ѯ����
	unsigned short qclass;//��ѯ��
};
typedef struct {
	unsigned char* name;
	struct QUESTION* ques;
} QUERY;

/*
**DNS�����лش�����ĳ����ֶ�
*/
//�����Ƶ�����
#pragma pack(push, 1)//�������״̬���趨Ϊ1�ֽڶ���
struct R_DATA {
	unsigned short type; //��ʾ��Դ��¼������
	unsigned short _class; //��
	unsigned int ttl; //��ʾ��Դ��¼���Ի����ʱ��
	unsigned short data_len; //���ݳ���
};
#pragma pack(pop) //�ָ�����״̬

/*
**DNS�����лش��������Դ�����ֶ�
*/
struct RES_RECORD {
	unsigned char* name;//��Դ��¼����������
	struct R_DATA* resource;//��Դ����
	unsigned char* rdata;
};

/*
**ʵ��DNS��ѯ����
*/
void ngethostbyname(unsigned char* host, int query_type,int clifd) {
	unsigned char buf[14776], * qname, * reader;
	int i, j, stop, s;

	struct sockaddr_in a;//��ַ

	struct RES_RECORD answers[20], auth[20], addit[20];//�ش�������Ȩ���򡢸��������е���Դ�����ֶ�
	struct sockaddr_in dest;//��ַ

	struct DNS_HEADER* dns = NULL;
	struct QUESTION* qinfo = NULL;

	printf("\n�������������%s", host);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //��������UDP�׽���

	dest.sin_family = AF_INET;//IPv4
	dest.sin_port = htons(53);//53�Ŷ˿�
	dest.sin_addr.s_addr = inet_addr(dns_servers[0]);//DNS������IP

	dns = (struct DNS_HEADER*) & buf;
	/*����DNS�����ײ�*/
	dns->id = (unsigned short)htons(getpid());//id��Ϊ���̱�ʶ��
	dns->qr = 0; //��ѯ
	dns->opcode = 0; //��׼��ѯ
	dns->aa = 0; //����Ȩ�ش�
	dns->tc = 0; //���ɽض�
	dns->rd = 1; //�����ݹ�
	dns->ra = 0; //�����õݹ�
	dns->z = 0; //����Ϊ0
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;//û�в��
	dns->q_count = htons(1); //1������
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	//qnameָ���ѯ��������Ĳ�ѯ���ֶ�
	qname = (unsigned char*)&buf[sizeof(struct DNS_HEADER)];

	ChangetoDnsNameFormat(qname, host);//�޸�������ʽ 
	qinfo = (struct QUESTION*) & buf[sizeof(struct DNS_HEADER)
		+ (strlen((const char*)qname) + 1)]; //qinfoָ�������ѯ����Ĳ�ѯ�����ֶ�

	qinfo->qtype = htons(query_type); //��ѯ����ΪA
	qinfo->qclass = htons(1); //��ѯ��Ϊ1

	//��DNS����������DNS������
	printf("\n\n���ͱ�����...");
	if (sendto(s, (char*)buf, sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr*) & dest, sizeof(dest)) < 0)
	{
		perror("����ʧ�ܣ�");
	}
	printf("���ͳɹ���");

	//��DNS����������DNS��Ӧ����
	i = sizeof dest;
	printf("\n���ձ�����...");
	if (recvfrom(s, (char*)buf, 14776, 0, (struct sockaddr*) & dest, (int*)&i) < 0) {
		perror("����ʧ�ܣ�");
	}
	else printf("���ճɹ���");

	dns = (struct DNS_HEADER*) buf;

	//��readerָ����ձ��ĵĻش����� 
	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)
		+ sizeof(struct QUESTION)];

	printf("\n\n��Ӧ���İ���: ");
	printf("\n %d������", ntohs(dns->q_count));
	printf("\n %d���ش�", ntohs(dns->ans_count));
	printf("\n %d����Ȩ����", ntohs(dns->auth_count));
	printf("\n Ӧ����%d", ntohs(dns->rcode));
	printf("\n %d�����Ӽ�¼\n\n", ntohs(dns->add_count));
	
	
	/*
	**�������ձ���
	*/
	reader = reader + sizeof(short);//short���ͳ���Ϊ32Ϊ���൱�������ֶγ��ȣ���ʱreaderָ��ش�����Ĳ�ѯ�����ֶ�
	answers[i].resource = (struct R_DATA*) (reader);
	//reader = reader + sizeof(struct R_DATA);//ָ��ش������������Դ�����ֶ�
	printf("type:%d\n", ntohs(answers[i].resource->type));
	printf("����%u\n", ntohs(answers[i].resource->data_len));
	if (ntohs(answers[i].resource->type) != A) {
		printf("7\n");
		reader = reader + sizeof(struct R_DATA)+ntohs(answers[i].resource->data_len)+sizeof(short);
		answers[i].resource = (struct R_DATA*) (reader);
		printf("type:%d\n", ntohs(answers[i].resource->type));
		printf("����%u\n", ntohs(answers[i].resource->data_len));
	}
	reader = reader + sizeof(struct R_DATA);
	if (ntohs(answers[i].resource->type) ==A) //�ж���Դ�����Ƿ�ΪIPv4��ַ
	{
		printf("����%u\n", ntohs(answers[i].resource->data_len));
		answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));//��Դ����
		for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
		{
			answers[i].rdata[j] = reader[j];
		}
		answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
		reader = reader + ntohs(answers[i].resource->data_len);
	}
	

	//��ʾ��ѯ���
	if (ntohs(answers[i].resource->type) == A) //�жϲ�ѯ����IPv4��ַ
	{
		long* p;
		p = (long*)answers[i].rdata;
		a.sin_addr.s_addr = *p;
		printf("IPv4��ַ:%s\n", inet_ntoa(a.sin_addr));
		send(clifd, inet_ntoa(a.sin_addr), strlen(inet_ntoa(a.sin_addr)), 0);
	}

	return;
}

/*
**��www.baidu.comת����3www5baidu3com
*/
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) {
	int lock = 0, i;
	strcat((char*)host, ".");

	for (i = 0; i < strlen((char*)host); i++) {
		if (host[i] == '.') {
			*dns++ = i - lock;
			for (; lock < i; lock++) {
				*dns++ = host[lock];
			}
			lock++;
		}
	}
	*dns++ = '\0';
}