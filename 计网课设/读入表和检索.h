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
#include "≤È—Ø”Ú√˚.h"

int DNS_TABLE_READIN(char* filename);
void URL_READIN(char* recvbuf, int recvnum);
int DOMAIN_MATCH(char* URL, int num);
unsigned short NewID_WRITEIN(unsigned short TempID,  BOOL TempFlag, SOCKADDR_IN TempAddr);
void PRINT_INFO(unsigned short NewID, int flag);
