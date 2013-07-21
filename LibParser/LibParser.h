#pragma once
#include <stdio.h>
#include <windows.h>
#include <io.h>
#include <vector>
using std::vector;

//flib �ļ���ʽ
//ǩ��
#define IMAGE_FLIB_START_SIZE             8
#define IMAGE_FLIB_START                  "!<flib>\n"
//FileHeader�ļ�ͷ��һ��FuncHead��
//��һ����ȫΪ�յ�fileheader��Ϊ�ļ�ͷ�Ľ�β
//NamSection���ƶΣ�����洢�����к���������null��β��C����ַ���
//DatSection���ݶΣ�����洢�����к�������

typedef struct _FuncHeader
{
	DWORD NameOff;//��������ƫ��
	DWORD NameSize;//��Ҫ����������ƫ��
	DWORD DataOff;//��������ƫ��
	DWORD DataSize;//�������ݴ�С
}FuncHeader,*PFuncHeader;
typedef struct _FlibFuncHeader//flib�ļ��еĺ���ͷ�ṹ
{
	DWORD NameOff;
	//DWORD NameSize;//����������NULL��β������Ҫsize��
	DWORD DataOff;
	DWORD DataSize;
}FlibFuncHeader,*PFlibFuncHeader;
typedef vector<FuncHeader> FuncHeaderTable;

class CLibParser
{
public:
	CLibParser(void);
	~CLibParser(void);
protected:
	PBYTE m_pLibImage;
	long  m_fsize;
	FILE* m_pFlibFile;//������ɵĺ������ļ�
	FILE* m_pNameFile;//�м����ɵĺ��������ļ�
	FILE* m_pDataFile;//�м����ɵĺ��������ļ�
	CHAR  m_FlibFileName[MAX_PATH];
	CHAR  m_NameFileName[MAX_PATH];
	CHAR  m_DataFileName[MAX_PATH];
	FuncHeaderTable m_FuncTable;//����ͷ��
protected:
	BOOL LoadLib(PCSTR szLib);
	PBYTE GetFirstObjSection();
	BOOL InitOutPutFile(PCSTR szLib);
	BOOL ParseObjs(PBYTE pObjSect);
	void LinkFile();

	void ModifySuffix(PCHAR filename,PCHAR pSuffix);
	BOOL fopen_S(FILE ** _File, PCSTR _Filename,PCSTR _Mode);
	BOOL bImportlibraryFormat(PBYTE pSect);
public:
	BOOL Parse(PCSTR szLib);
};
