#include<stdlib.h>
#include<stdio.h>

#include"pkg_pro.h"


using namespace std;

SOCKADDR_IN client_ip[IDTABLE_SIZE];//��ſͻ�����ip��ַ�����Է��𸴰��Լ���������

//************************
void init_table(short int t[], short int q)
{
	for (int i = 0; i < IDTABLE_SIZE; i++)
	{
		t[i] = q;
	}
}
//*****************************

void query_for_superior_server(char *receiveBuffer)
{
	SOCKADDR_IN to_address;
	int addr_len = sizeof(SOCKADDR_IN);
	to_address.sin_family = AF_INET;
	to_address.sin_port = htons(53);
	to_address.sin_addr.S_un.S_addr = inet_addr("10.3.9.6");
	if (sendto(serverSocket, receiveBuffer, last, 0, (const struct sockaddr *)&to_address, addr_len))
	{
		printf("ת������һ�������������ɹ���\n");
	}
	else
	{
		printf("ת������һ������������ʧ�ܣ�\n");
	}
}

void query_pro(dns_header *header, char *receiveBuffer, SOCKADDR_IN cli_ip)
{
	char *question_sec = receiveBuffer + sizeof(dns_header);
	char QNAME[QNAME_MAX_LENTH + 1];
	int index = 0, now_pos = 0;
	while (question_sec[index] != 0x00)//��ȡ����
	{
		int temp = question_sec[index], i;
		for (i = 0; i < temp; i++)
		{
			QNAME[i + now_pos] = question_sec[index + i + 1];
		}
		index += temp + 1;
		now_pos += temp;
		if (question_sec[index] != 0x00)
			QNAME[now_pos++] = '.';
	}
	QNAME[now_pos] = '\0';
	//	printf("%s\n", QNAME);

	int result = 0;
	/*
	  ��ѯ�������ݿ�

	*/
	if (result)//�������ݿ��л���,���ͻ��˷���response��
	{

	}
	else//���һ��������������ѯ
	{
		//ת��DNS���ݰ�ͷID,��ͻ���ip��ַ
		int k = 0;//�ж��Ƿ�����ͬID
		short int hid = header->ID;
		short int nhid;//���ĺ��ID
		for (int i = 0; i < it_length; i++)
		{
			if (hid == new_id_table[i]) {
				k = 1;
				break;
			}
		}
		if (k == 0) {//û����ͬ��ID
			nhid = hid;//���ø���ID
			int i = 0;
			for (i = 0; i < it_length; i++) {
				if (nhid < new_id_table[i]) {//����ID������ĺ�ı�
					for (int j = it_length; j > i; j--)
					{//����
						new_id_table[j] = new_id_table[j - 1];
						old_id_table[j] = old_id_table[j - 1];
						client_ip[j] = client_ip[j - 1];
					}
					old_id_table[i] = hid;
					new_id_table[i] = nhid;
					client_ip[i] = cli_ip;
					it_length++;
					break;
				}
			}
			if (i == it_length) {//���±�������ID��С�ڴ�ID
				old_id_table[i] = hid;
				new_id_table[it_length] = nhid;
				client_ip[i] = cli_ip;
				it_length++;
			}
		}
		else {//����ͬ��id
			nhid = 0;//��0��ʼ����ID
			int i = 0;
			for (int i = 0; i < it_length; i++) {
				if (nhid == new_id_table[i])  nhid++;//�����ID�ѱ�ʹ��
				else {//�����IDδ��ʹ�ã�����������ģ���ֱ���ڴ˲���
					for (int j = it_length; j > i; j--)
					{//����
						new_id_table[j] = new_id_table[j - 1];
						old_id_table[j] = old_id_table[j - 1];
						client_ip[j] = client_ip[j - 1];
					}
					old_id_table[i] = hid;
					new_id_table[i] = nhid;
					client_ip[i] = cli_ip;
					it_length++;
					break;
				}
			}
			if (i == it_length) {//���±�������ID��С�ڴ�ID
				old_id_table[i] = hid;
				new_id_table[it_length] = nhid;
				client_ip[i] = cli_ip;
				it_length++;
			}
		}


		header->ID = nhid;//����ID�ֶθ�����ͷ

		//���յ�header���ֽ����Ϊ�����ֽ���
		header->ID = htons(header->ID);
		header->FLAGS = htons(header->FLAGS);
		header->QDCOUNT = htons(header->QDCOUNT);
		header->ANCOUNT = htons(header->ANCOUNT);
		header->NSCOUNT = htons(header->NSCOUNT);
		header->ARCOUNT = htons(header->ARCOUNT);

		query_for_superior_server(receiveBuffer);//���һ���������������Ͳ�ѯ

	}
}

void resp_pro(dns_header *header, char *receiveBuffer)
{
	//��ԭDNS���ݰ�ͷID
	SOCKADDR_IN q_ip;//�˰�Ӧ�ظ����Ŀͻ���ip
	short int hid = header->ID;
	short int rehid;//��ԭ����ID
	int i = 0;
	for (i = 0; i < it_length; i++)
	{//�ҵ�ID���±��ж�Ӧ���±�
		if (hid == new_id_table[i])  break;
	}
	rehid = old_id_table[i];//ȡԭʼID
	q_ip = client_ip[i];

	for (int j = i; j < it_length - 1; j++)
	{//ɾ����ID���ݣ�����ID����ǰ��
		old_id_table[i] = old_id_table[i + 1];
		new_id_table[i] = new_id_table[i + 1];
		client_ip[i] = client_ip[i + 1];
	}
	it_length--;//���ȼ�1
	header->ID = rehid;//��ԭID

	//ȡӦ�����ݣ��������ݿ�

	//ȡAA
	if ((header->FLAGS & 0x0400) == 0x0400) {//������Ȩ�����������������ڴ�Ϊ���ط�������
		printf("�����Ա��ط�����\n");
	}
	else {
		printf("�������ϼ�������\n");
	}

	//�ж�RCODE
	if ((header->FLAGS & 0x000F) == 0x0003) {//RCODEΪ3��ʾ��������
		printf("δ��ѯ��������\n\n");
	}
	else {//������Ӧ��
		printf("\n");
		//�ж������������Դ��¼����
		int ques = header->QDCOUNT;//query�ֶθ���
		int requ = header->ANCOUNT;//anwser�ֶθ���
		int aure = header->NSCOUNT;//authority�ֶθ���
		int adre = header->ARCOUNT;//additional�ֶθ���
		int reso = requ + aure + adre;//�ܵ���Դ��¼����

		int c_byte = sizeof(dns_header);//��ǰ��ֵ����ֽ�λ��
//		printf("�������Դ��¼����%d   %d\n", ques, reso);
		//���Question Section����
		for (int i = 0; i < ques; i++)
		{
			if (i == 0)  printf("Question Section��%d������\n\n", ques);

			char doname[QNAME_MAX_LENTH];//�������ʼ���ַ��׺��
			int length = 0;//�������ʼ���ַ��׺��ռ�õ��ֽ���
			unsigned short *type;//��ѯ����
			unsigned short *Class;//��ѯ��


			length = do_name_reso(0, 0, c_byte, doname, receiveBuffer);//�����������ʼ���ַ��׺��
			c_byte += length;
			type = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;
			Class = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;

			*type = ntohs(*type);
			*Class = ntohs(*Class);
			printf("    qname��%s\n", doname);
			printf("    qtype��%d%\n", *type);
			printf("    qclass��%d%\n\n", *Class);
			*type = htons(*type);
			*Class = htons(*Class);
		}

		//��ֺ�������Դ��¼���֣���Ϊ��ʽ��ͬ����ͬʱ����
		for (int i = 0; i < reso; i++)
		{
			if (i == 0)  printf("Anwser Section��%d������\n\n", requ);
			if (i == requ)  printf("Authority Records Section��%d������\n\n", aure);
			if (i == requ + aure)  printf("Additional Records Section��%d������\n\n", adre);

			char doname[QNAME_MAX_LENTH];//�������ʼ���ַ��׺��
			int length = 0;//�������ʼ���ַ��׺��ռ�õ��ֽ���
			unsigned short *type;//��ѯ����
			unsigned short *Class;//��ѯ��
			unsigned long *ttl;//����ʱ��
			unsigned short *relength;//��Դ���ݳ���

			length = do_name_reso(0, 0, c_byte, doname, receiveBuffer);//�����������ʼ���ַ��׺��
//			printf("��������%d\n", length);
			c_byte += length;
			type = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;
			Class = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;
			ttl = (unsigned long*)(receiveBuffer + c_byte);
			c_byte += 4;
			relength = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;

			*type = ntohs(*type);
			*Class = ntohs(*Class);
			*ttl = ntohl(*ttl);
			*relength = ntohs(*relength);
			printf("    name��%s\n", doname);
			printf("    type��%d%\n", *type);
			printf("    class��%d%\n", *Class);
			printf("    time to live��%ld%\n", *ttl);

			if (*type == 1)
			{//IP��ַ����
				unsigned char ip_address[4];
				ip_address[0] = receiveBuffer[c_byte];
				ip_address[1] = receiveBuffer[c_byte + 1];
				ip_address[2] = receiveBuffer[c_byte + 2];
				ip_address[3] = receiveBuffer[c_byte + 3];
				printf("    ip��%d.%d.%d.%d\n\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
			}
			else if (*type == 2)
			{//NS����
				char dname[QNAME_MAX_LENTH];//�洢����������������
				length = do_name_reso(0, 0, c_byte, dname, receiveBuffer);//��������������������
				printf("    name server��%s\n\n", dname);
			}
			else if (*type == 5)
			{//CNAME����
				char cname[QNAME_MAX_LENTH];//�洢�淶��
				length = do_name_reso(0, 0, c_byte, cname, receiveBuffer);//�����淶��
				printf("    cname��%s\n\n", cname);
			}
			else if (*type == 15)
			{//MX����
				char mname[QNAME_MAX_LENTH];//�洢�ʼ���������
				unsigned short *preference;//�ʼ������������ȼ�
				preference = (unsigned short*)(receiveBuffer + c_byte);
				length = do_name_reso(0, 0, c_byte + 2, mname, receiveBuffer);//�����ʼ���������
				*preference = ntohs(*preference);
				printf("    preference��%d\n", *preference);
				printf("    mail exchange��%s\n\n", mname);
				*preference = htons(*preference);
			}
			c_byte += *relength;

			*type = htons(*type);
			*Class = htons(*Class);
			*ttl = htonl(*ttl);
			*relength = htons(*relength);
		}
	}

	header->ID = htons(header->ID);
	header->FLAGS = htons(header->FLAGS);
	header->QDCOUNT = htons(header->QDCOUNT);
	header->ANCOUNT = htons(header->ANCOUNT);
	header->NSCOUNT = htons(header->NSCOUNT);
	header->ARCOUNT = htons(header->ARCOUNT);

	//ת�����ͻ���
	sendto(serverSocket, receiveBuffer, last, 0, (SOCKADDR*)&q_ip, sizeof(SOCKADDR));
}

int do_name_reso(int clength, int addlength, int c_byte, char doname[], char *receivebuffer)
{
	int length = clength;//��¼����ռ�ó���
	int alength = addlength;//��¼�����ĳ���
	int cu_byte = c_byte;
		printf("��ǰ�ֽ�����%d\n", cu_byte);
	unsigned char  c;

	c = receivebuffer[cu_byte];//ȡ��һ���������ֽ���
	printf("��ǰ�����ֽ�����%d\n", c);
	while (c != 0)
	{//δ������������
		if ((c & 0xc0) == 0xc0)
		{
			unsigned short *x = (unsigned short *)(receivebuffer + cu_byte);
			*x = ntohs(*x);//ת��Ϊ�����ֽ���
			*x = (*x) & 0x3fff;//ǰ��bit����
			int offset = *x;
			int k = do_name_reso(length, alength, offset, doname, receivebuffer);//�ݹ����������������ռ�ó���
			*x = (*x) | 0xc000;//ǰ��bit��ԭ
			*x = htons(*x);//��ԭΪ�����ֽ���
			return length + 2;
		}
		else
		{
			cu_byte++;
			length++;
						printf("��ǰ�ֽ�����%d\n", cu_byte);
			int le = c;//ת��Ϊ����
			printf("��ǰ�����ֽ�����%d\n", le);
			for (int i = 0; i < le; i++)
			{
				doname[alength++] = receivebuffer[cu_byte++];
				length++;
			}
			c = receivebuffer[cu_byte];//ȡ��һ���������ֽ���
			printf("��ǰ�����ֽ�����%d\n", c);
			if (c != 0)  doname[alength++] = '.';
		}
	}
	cu_byte++;
	length++;//����������Ҳ����ռ�ó�����
	doname[alength] = '\0';
	return length;

}

void queryARecord(sqlite3 *db, char *zErrMsg, string domainName)
{
	int ret = 0;
	sqlite3_stmt *statement;
	string sql = "SELECT * from A_RECORD where domainName = '" + domainName + "'";
	sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return;
	}
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* ARecord = (char *)sqlite3_column_text(statement, 1);
		int TTL = sqlite3_column_int(statement, 2);

		printf("domainName = %s\nARecord = %s\nTTL = %d\n\n", domainName1, ARecord, TTL);

	}
}

void queryCNAMERecord(sqlite3 *db, char *zErrMsg, string domainName)
{
	int ret = 0;
	sqlite3_stmt *statement;
	string sql = "SELECT * from CNAME_RECORD where domainName = '" + domainName + "'";
	sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return;
	}
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* CNAMERecord = (char *)sqlite3_column_text(statement, 1);
		int TTL = sqlite3_column_int(statement, 2);

		printf("domainName = %s\nCNAMERecord = %s\nTTL = %d\n\n", domainName1, CNAMERecord, TTL);

	}
}

void queryMXRecord(sqlite3 *db, char *zErrMsg, string domainName)
{
	int ret = 0;
	sqlite3_stmt *statement;
	string sql = "SELECT * from MX_RECORD where domainName = '" + domainName + "'";
	sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return;
	}
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* MXRecord = (char *)sqlite3_column_text(statement, 1);
		int MXPreference = sqlite3_column_int(statement, 2);
		int TTL = sqlite3_column_int(statement, 3);

		printf("domainName = %s\nMXRecord = %s\nMXPreference = %d\nTTL = %d\n\n", domainName1, MXRecord, MXPreference, TTL);

	}
}

void queryNSRecord(sqlite3 *db, char *zErrMsg, string domainName)
{
	int ret = 0;
	sqlite3_stmt *statement;
	string sql = "SELECT * from NS_RECORD where domainName = '" + domainName + "'";
	sqlite3_prepare(db, sql.c_str(), -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return;
	}
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* NSRecord = (char *)sqlite3_column_text(statement, 1);
		int TTL = sqlite3_column_int(statement, 2);

		printf("domainName = %s\nNSRecord = %s\nTTL = %d\n\n", domainName1, NSRecord, TTL);

	}
}