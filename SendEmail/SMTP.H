#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <io.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>  
#include <commctrl.h>
#pragma comment(lib, "comctl32") 
#pragma  comment(lib,"ws2_32.lib")

bool InitSock() ;
//���ӷ�����
bool ConnectServer(SOCKET *socket,const char *domain,const int port);
//��ʾ���Ӵ���
void ShowError();
//���ӵ�ָ��������
bool connectUserMail(SOCKET socket,char *username,char *password );
//�жϷ�����Ϣ�Ƿ���OK
bool IsOK(char * message);