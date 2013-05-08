//Download by http://www.NewXing.com
#pragma once
#ifndef __DEBUG
#define __DEBUG
#include <fstream>
#include <iostream>
using namespace std;
#endif
#include <WINDOWS.H>
#include <IO.H>
#include <fcntl.h>
#include <sys/stat.h>
#include <shellapi.h>

typedef struct _PE_HEADER
{
	DWORD signature;
    IMAGE_FILE_HEADER _head;
    IMAGE_OPTIONAL_HEADER opt_head;
    IMAGE_SECTION_HEADER section_header[6];
}PE_HERDER;

class CPE
{
public:
	CPE(void);
public:
	~CPE(void);
private:
	bool GetPEAddress(const void* Basepoint);					//��ȡPE��ַ	
	bool WriteNewEntry(int ret,long offset,DWORD dwAddress);	//д�������
	bool WriteCode(int ret,long offset,char* strCap,char* strText,char* openFlag,char* Address);//д�����
	void DWORDTOSTR(DWORD dwAddress,unsigned char* chStr);		//DWORDת��Ϊ�ַ���
	bool WriteFile(char* FileName);		
private:
	DWORD dwSpace;				//���ÿռ�
	DWORD dwRAV;				//����װ�ص�ַ һ��Ϊ0x400000
	DWORD dwVirSize;			//��ʵ��С
	DWORD dwEntryAddress;		//��ڵ�ַ
	DWORD dwWriteEntry;			//д���ַ���
	DWORD dwOldEntry;			//old���
	DWORD dwNewEntry;			//�����
	DWORD dwCodeOffset;			//����ƫ��
	DWORD dwPeAddress;			//PE ��ַ
	DWORD dwFlagAddress;		//��־��ַ
	DWORD dwVirtualSize;		//�����С
	DWORD dwPhyAddress;			//������ַ
	DWORD dwPhySize;			//������С
	DWORD dwCodeAddress;		//д���MessageBox�ĺ�����ַ
	DWORD dwShellAddress;		//�˳�����
public:
	bool ModifyPE(char* strFileName);	
};