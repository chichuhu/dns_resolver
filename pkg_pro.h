#ifndef PKG_PRO_H__
#define PKG_PRO_H__

#include<winsock2.h>
#include<string>

#include"sqlite3.h"

#define IDTABLE_SIZE 256

#define BUFFER_SIZE 1024
#define QNAME_MAX_LENTH 256
#define SUPERIOR_SERVER_ADDRESS 10.3.9.6

using namespace std;

typedef struct
{
	unsigned short ID;
	unsigned short FLAGS;
	unsigned short QDCOUNT;
	unsigned short ANCOUNT;
	unsigned short NSCOUNT;
	unsigned short ARCOUNT;
} dns_header;

//********************************
extern int it_length;//��ǰ�����ID��Ŀ
extern int last;//�������ݵĳ���
extern short int old_id_table[IDTABLE_SIZE];//ԭʼID��
extern short int new_id_table[IDTABLE_SIZE];//���ĺ��ID��
extern SOCKADDR_IN client_ip[IDTABLE_SIZE];//��ſͻ�����ip��ַ�����Է��𸴰��Լ���������
//******************************

extern SOCKET serverSocket;

void init_table(short int t[], short int q);//�����ʼ��

void query_pro(dns_header *header, char *receiveBuffer, SOCKADDR_IN cli_ip);//���������

void query_for_superior_server(char *receiveBuffer);//ת������һ������������

void resp_pro(dns_header *header, char *receiveBuffer);//��Ӧ������

void query_record(sqlite3 *db, char *zErrMsg, string domainName);

void query_cnamerecord(sqlite3 *db, char *zErrMsg, string domainName);

void query_mxrecord(sqlite3 *db, char *zErrMsg, string domainName);

void query_nsrecord(sqlite3 *db, char *zErrMsg, string domainName);

//*******************************************************
int do_name_reso(int clength, int addlength, int c_byte, char doname[], char *receiveBuffer);//��������
//********************************************************



#endif // PKG_PRO_H__

