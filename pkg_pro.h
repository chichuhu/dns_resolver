#ifndef PKG_PRO_H__
#define PKG_PRO_H__

#include<winsock2.h>
#include<string>
#include<iostream>

#include"sqlite3.h"

#define IDTABLE_SIZE 256

#define BUFFER_SIZE 1024
#define QNAME_MAX_LENTH 256
#define SUPERIOR_SERVER_ADDRESS "10.3.9.5"

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


extern int it_length;//��ǰ�����ID��Ŀ
extern int last;//�������ݵĳ���
extern short int old_id_table[IDTABLE_SIZE];//ԭʼID��
extern short int new_id_table[IDTABLE_SIZE];//���ĺ��ID��
extern SOCKADDR_IN client_ip[IDTABLE_SIZE];//��ſͻ�����ip��ַ�����Է��𸴰��Լ���������
extern sqlite3 *db;//sqlite3���ݿ��ʼ����Ϣ

extern SOCKET serverSocket;

void init_table(short int t[], short int q);//�����ʼ��
void query_pro(dns_header *header, char *receiveBuffer, SOCKADDR_IN cli_ip);//���������
void query_for_superior_server(char *receiveBuffer, dns_header *header, SOCKADDR_IN cli_ip);//ת������һ������������
void resp_pro(dns_header *header, char *receiveBuffer);//��Ӧ������
void connect_string(char *a, char *b);
void connect_const_string(char *a, const char *b);

int query_A_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength);
int query_A_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength, const char *Address, int addLength);
void insert_A_record(sqlite3 *db, char *zErrMsg, char *Name, char *Alias, char *Type, char *Class, int TTL, int DataLength,const char *Address, int *length);

void insert_CNAME_record(sqlite3 *db, char *zErrMsg, char *Name, char *Alias, char *Type, char *Class, int TTL, int DataLength, char *CNAME, int *length);
int query_CNAME_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength);
int query_CNAME_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength, char *CNAME, int CNLength);

void connect_string(char *a, const char *b, int aLength, int bLength);
void connect_string(char *a, char *b, int aLength, int bLength);

string translate_IP(unsigned char* ip);
int do_name_reso(int clength, int addlength, int c_byte, char doname[], char *receiveBuffer);//��������

#endif // PKG_PRO_H__

