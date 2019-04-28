#include<stdlib.h>
#include<stdio.h>

#include"pkg_pro.h"



SOCKADDR_IN client_ip[IDTABLE_SIZE];//��ſͻ�����ip��ַ�����Է��𸴰��Լ���������



static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int str_len(char *str)
{
	int i = 0;
	while (str[i])
	{
		i++;
	}
	return i;
}

string translate_IP(unsigned char* ip)//����unsigned char���ʹ洢��ip��ַת�����ַ���
{
	string result = "";
	for (int i = 0; i < 4; i++)
	{
		result = result + to_string(ip[i]) + ".";
	}
	return result.substr(0, result.length() - 1);
}

void init_table(short int t[], short int q)
{
	for (int i = 0; i < IDTABLE_SIZE; i++)
	{
		t[i] = q;
	}
}

void query_for_superior_server(char *receiveBuffer, dns_header *header, SOCKADDR_IN cli_ip)
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
		for (i = 0; i < it_length; i++)
		{
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
		if (i == it_length)
		{//���±�������ID��С�ڴ�ID
			old_id_table[i] = hid;
			new_id_table[it_length] = nhid;
			client_ip[i] = cli_ip;
			it_length++;
		}
	}
	else
	{//����ͬ��id
		nhid = 0;//��0��ʼ����ID
		int i = 0;
		for (int i = 0; i < it_length; i++)
		{
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
		if (i == it_length)
		{//���±�������ID��С�ڴ�ID
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

	SOCKADDR_IN to_address;
	int addr_len = sizeof(SOCKADDR_IN);
	to_address.sin_family = AF_INET;
	to_address.sin_port = htons(53);
	to_address.sin_addr.S_un.S_addr = inet_addr(SUPERIOR_SERVER_ADDRESS);
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
	
	/*�жϱ������ݿ��Ƿ���и�������¼*/
	char doName[QNAME_MAX_LENTH];//��ѯ�������ʼ���ַ��׺��
	int length = 0;//�������ʼ���ַ��׺��ռ�õ��ֽ���
	int c_byte = sizeof(dns_header);//��ǰ��ֵ����ֽ�λ��
	unsigned short *type;//��ѯ����

	length = do_name_reso(0, 0, c_byte, doName, receiveBuffer);//�����������ʼ���ַ��׺��
	c_byte += length;
	type = (unsigned short*)(receiveBuffer + c_byte);
	c_byte += 2;

	int tp = ntohs(*type);
	printf("�����ѯ����: %d\n",tp);

	int doNameLen = str_len(doName);


	char *zErrMsg = 0;
	if(tp == 1)//ΪA���Ͳ�ѯ����
	{
		int queryResult = query_A_record(db, zErrMsg, doName, doNameLen);
		if (queryResult)//�������ݿ��л���,���ͻ��˷���response��
		{
			printf("���ش���%s������A���ͼ�¼!\n\n", doName);
		}
		else
		{
			query_for_superior_server(receiveBuffer, header, cli_ip);//���һ���������������Ͳ�ѯ
		}
		return;
	}
	//else if (*type == 5)//ΪCNAME���Ͳ�ѯ����
	//{

	//}

	//�����û�м�¼�����һ��������������ѯ
	query_for_superior_server(receiveBuffer, header, cli_ip);//���һ���������������Ͳ�ѯ
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

		
		char doName[QNAME_MAX_LENTH];//��ѯ�������ʼ���ַ��׺��

		for (int i = 0; i < ques; i++)
		{
			if (i == 0)  printf("Question Section��%d������\n\n", ques);

			
			int length = 0;//�������ʼ���ַ��׺��ռ�õ��ֽ���
			unsigned short *type;//��ѯ����
			unsigned short *Class;//��ѯ��


			length = do_name_reso(0, 0, c_byte, doName, receiveBuffer);//�����������ʼ���ַ��׺��
			c_byte += length;
			type = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;
			Class = (unsigned short*)(receiveBuffer + c_byte);
			c_byte += 2;

			*type = ntohs(*type);
			*Class = ntohs(*Class);
			printf("    qname��%s\n", doName);
			printf("    qtype��%d\n", *type);
			printf("    qclass��%d\n\n", *Class);
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
			//printf("��������%d\n", *relength);
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
			printf("    type��%d\n", *type);
			printf("    class��%d\n", *Class);
			printf("    time to live��%ld\n", *ttl);
			//printf("    dataLenth: %d\n", *relength);

			char storeData[BUFFER_SIZE];
			char *zErrMsg = 0;

			int TTL = (int)*ttl;
			int ttlLen = (std::to_string(TTL)).length();
			int doNameLen = str_len(doName);
			int aliasLen = str_len(doname);
			int lenth[7];//������Դ��������	

			lenth[0] = doNameLen;//��������
			lenth[1] = aliasLen;//��������
			lenth[3] = 2;//class����
			lenth[4] = ttlLen;

			printf("    dataLenth: %d\n", doNameLen);
			printf("    TTL:%d\n", TTL);
			if (*type == 1)
			{//IP��ַ����
				unsigned char ip_address[4];
				storeData[0] = 'A';
				storeData[1] = 'I';
				storeData[2] = 'N';
				storeData[4] = ip_address[0] = receiveBuffer[c_byte];
				storeData[5] = ip_address[1] = receiveBuffer[c_byte + 1];
				storeData[6] = ip_address[2] = receiveBuffer[c_byte + 2];
				storeData[7] = ip_address[3] = receiveBuffer[c_byte + 3];
				storeData[8] = '\0';
				printf("    ip��%d.%d.%d.%d\n\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
				
				string ip = translate_IP(ip_address);	
				int ipLen = ip.length();//ip��ַ����
				lenth[2] = 1;//type����
				lenth[5] = 1;//DataLength�ֶγ���
				lenth[6] = ipLen;
				const char *ipRes = ip.data();

				if(!query_A_record(db, zErrMsg,  doName, doNameLen, ipRes, ipLen))
				    insert_A_record(db, zErrMsg, doName, doname, storeData, storeData + 1, TTL, 4, ipRes, lenth);
			}
			else if (*type == 2)
			{//NS����
				char dname[QNAME_MAX_LENTH];//�洢����������������
				length = do_name_reso(0, 0, c_byte, dname, receiveBuffer);//��������������������
				printf("    name server��%s\n\n", dname);
			}
			else if (*type == 5)
			{//CNAME����
				lenth[2] = 2;//type���ݳ���Ϊ2
				storeData[0] = 'C';//CN����
				storeData[1] = 'N';
				storeData[2] = 'I';//INclass
				storeData[3] = 'N';

				char cname[QNAME_MAX_LENTH];//�洢�淶��
				length = do_name_reso(0, 0, c_byte, cname, receiveBuffer);//�����淶��
				lenth[5] = (std::to_string(length)).length();
				lenth[6] = str_len(cname);
				if(!query_CNAME_record(db, zErrMsg, doName, doNameLen, cname, lenth[6]))//������ݿ����޸�CN��¼��洢
				  insert_CNAME_record(db, zErrMsg, doName, doname, storeData, storeData + 2, TTL, length, cname, lenth);
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
	//printf("��ǰ�ֽ�����%d\n", cu_byte);
	unsigned char  c;

	c = receivebuffer[cu_byte];//ȡ��һ���������ֽ���
	//printf("��ǰ�����ֽ�����%d\n", c);
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
			//printf("��ǰ�ֽ�����%d\n", cu_byte);
			int le = c;//ת��Ϊ����
			//printf("��ǰ�����ֽ�����%d\n", le);
			for (int i = 0; i < le; i++)
			{
				doname[alength++] = receivebuffer[cu_byte++];
				length++;
			}
			c = receivebuffer[cu_byte];//ȡ��һ���������ֽ���
			//printf("��ǰ�����ֽ�����%d\n", c);
			if (c != 0)  doname[alength++] = '.';
		}
	}
	cu_byte++;
	length++;//����������Ҳ����ռ�ó�����
	doname[alength] = '\0';
	return length;

}

void connect_string(char *a, char *b, int aLength, int bLength)
{
	int i;
	for (i = 0; i < bLength; i++)
	{
		a[aLength + i] = b[i];
	}
	a[aLength + i] = 0;
}

void connect_string(char *a, const char *b, int aLength, int bLength)
{
	int i;
	for (i = 0; i < bLength; i++)
	{
		a[aLength + i] = b[i];
	}
	a[aLength + i] = 0;
}

void insert_A_record(sqlite3 *db, char *zErrMsg, char *Name, char *Alias, char *Type, char *Class, int TTL, int DataLength, const char *Address, int *length)
{
	int sqlLength;
	char temSql[4096] = "INSERT INTO A_RECORD (Name, Alias, Type, Class, Time_to_live, Data_length, Address) VALUES (";
	sqlLength = strlen(temSql);
	//temSql = temSql + "'" + domainName + "'" + ", " + "'" + ARecord + "'" + ", " + std::to_string(TTL) + ");";
	connect_string(temSql, "'", sqlLength, 1);
	sqlLength += 1;
	connect_string(temSql, Name, sqlLength, length[0]);
	sqlLength += length[0];
	connect_string(temSql, "', '", sqlLength, 4);
	sqlLength += 4;
	connect_string(temSql, Alias, sqlLength, length[1]);
	sqlLength += length[1];
	connect_string(temSql, "', '", sqlLength, 4);
	sqlLength += 4;
	connect_string(temSql, Type, sqlLength, length[2]);
	sqlLength += length[2];
	connect_string(temSql, "', '", sqlLength, 4);
	sqlLength += 4;
	connect_string(temSql, Class, sqlLength, length[3]);
	sqlLength += length[3];
	connect_string(temSql, "', ", sqlLength, 3);
	sqlLength += 3;
	connect_string(temSql, std::to_string(TTL).c_str(), sqlLength, length[4]);
	sqlLength += length[4];
	connect_string(temSql, ", ", sqlLength, 2);
	sqlLength += 2;
	connect_string(temSql, std::to_string(DataLength).c_str(), sqlLength, length[5]);
	sqlLength += length[5];
	connect_string(temSql, ", '", sqlLength, 3);
	sqlLength += 3;
	connect_string(temSql, Address, sqlLength, length[6]);
	sqlLength += length[6];
	connect_string(temSql, "');", sqlLength, 3);
	//printf("%s", temSql);
	//return;
	int rc = sqlite3_exec(db, temSql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}

}

int query_A_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength, const char *Address, int addLength)
{
	int ret = 0;
	sqlite3_stmt *statement;
	int sqlLength;
	char sql[4096] = "SELECT * from A_RECORD where Name = '";
	sqlLength = strlen(sql);
	connect_string(sql, Name, sqlLength, nameLength);
	sqlLength += nameLength;
	connect_string(sql, "' and Address = '", sqlLength, 17);
	sqlLength += 17;
	connect_string(sql, Address, sqlLength, addLength);
	sqlLength += addLength;
	connect_string(sql, "';", sqlLength, 2);
	sqlite3_prepare(db, sql, -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return 0;
	}
	int res = 0;
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* ARecord = (char *)sqlite3_column_text(statement, 6);

		printf("domainName = %s\nARecord = %s\n\n", domainName1, ARecord);
		res++;
	}
	return res;
} 

int query_A_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength)
{
	int ret = 0;
	sqlite3_stmt *statement;
	int sqlLength;
	char sql[4096] = "SELECT * from A_RECORD where Name = '";
	sqlLength = strlen(sql);
	connect_string(sql, Name, sqlLength, nameLength);
	sqlLength += nameLength;
	connect_string(sql, "';", sqlLength, 2);
	sqlite3_prepare(db, sql, -1, &statement, NULL);
	//printf("%s-------sql------\n", sql);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return 0;
	}
	int res = 0;
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* alias = (char *)sqlite3_column_text(statement, 1);
		int TTL = sqlite3_column_int(statement, 4);
		char *address = (char *)sqlite3_column_text(statement, 6);

		printf("domainName = %s\nARecord = %s\nTTL = %d\nadress = %s\n\n", domainName1, alias, TTL, address);
		res++;
	}
	//printf("%d--------query-------\n", res);
	return res;
}

void insert_CNAME_record(sqlite3 *db, char *zErrMsg, char *Name, char *Alias, char *Type, char *Class, int TTL, int DataLength, char *CNAME, int *length)
{
	int sqlLength;
	char temSql[4096] = "INSERT INTO CNAME_RECORD (Name, Alias, Type, Class, Time_to_live, Data_length, CNAME) VALUES (";
	sqlLength = strlen(temSql);
	//temSql = temSql + "'" + domainName + "'" + ", " + "'" + ARecord + "'" + ", " + std::to_string(TTL) + ");";
	connect_string(temSql, "'", sqlLength, 1);
	sqlLength += 1;
	connect_string(temSql, Name, sqlLength, length[0]);
	sqlLength += length[0];
	connect_string(temSql, "', '", sqlLength, 4);
	sqlLength += 4;
	connect_string(temSql, Alias, sqlLength, length[1]);
	sqlLength += length[1];
	connect_string(temSql, "', '", sqlLength, 4);
	sqlLength += 4;
	connect_string(temSql, Type, sqlLength, length[2]);
	sqlLength += length[2];
	connect_string(temSql, "', '", sqlLength, 4);
	sqlLength += 4;
	connect_string(temSql, Class, sqlLength, length[3]);
	sqlLength += length[3];
	connect_string(temSql, "', ", sqlLength, 3);
	sqlLength += 3;
	connect_string(temSql, std::to_string(TTL).c_str(), sqlLength, length[4]);
	sqlLength += length[4];
	connect_string(temSql, ", ", sqlLength, 2);
	sqlLength += 2;
	connect_string(temSql, std::to_string(DataLength).c_str(), sqlLength, length[5]);
	sqlLength += length[5];
	connect_string(temSql, ", '", sqlLength, 3);
	sqlLength += 3;
	connect_string(temSql, CNAME, sqlLength, length[6]);
	sqlLength += length[6];
	connect_string(temSql, "');", sqlLength, 3);
	//printf("%s", temSql);
	//return;
	int rc = sqlite3_exec(db, temSql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}

}

int query_CNAME_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength)
{
	int ret = 0;
	sqlite3_stmt *statement;
	int sqlLength;
	char sql[4096] = "SELECT * from CNAME_RECORD where Name = '";
	sqlLength = strlen(sql);
	connect_string(sql, Name, sqlLength, nameLength);
	sqlLength += nameLength;
	connect_string(sql, "';", sqlLength, 2);
	sqlite3_prepare(db, sql, -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return 0;
	}
	int res = 0;
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* CNAME_Record = (char *)sqlite3_column_text(statement, 6);


		//printf("domainName = %s\nCNAME = %s\n\n", domainName1, CNAME_Record);
		res++;
	}
	return res;
}

int query_CNAME_record(sqlite3 *db, char *zErrMsg, char *Name, int nameLength, char *CNAME, int CNLength)
{
	int ret = 0;
	sqlite3_stmt *statement;
	int sqlLength;
	char sql[4096] = "SELECT * from CNAME_RECORD where Name = '";
	sqlLength = strlen(sql);
	connect_string(sql, Name, sqlLength, nameLength);
	sqlLength += nameLength;
	connect_string(sql, "' and CNAME = '", sqlLength, 15);
	sqlLength += 15;
	connect_string(sql, CNAME, sqlLength, CNLength);
	sqlLength += CNLength;
	connect_string(sql, "';", sqlLength, 2);
	sqlite3_prepare(db, sql, -1, &statement, NULL);
	if (ret != SQLITE_OK)
	{
		printf("prepare error ret : %d\n", ret);
		return 0;
	}
	int res = 0;
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		char* domainName1 = (char *)sqlite3_column_text(statement, 0);
		char* CNAME_Record = (char *)sqlite3_column_text(statement, 6);

		printf("domainName = %s\nCNAME = %s\n\n", domainName1, CNAME_Record);
		res++;
	}
	return res;
}
