// LibScanner.cpp : �������̨Ӧ�ó������ڵ㡣
//���ã�LibParser�Ĳ��Գ�����Ҫ����LibParser���ɵ�flb�ļ��Ƿ�׼ȷ
//����:
//1.PEInfo����pe�ļ�����ô���Σ�
//2.PVDasm��������棬��������Σ������callָ��(0xe8)���Ӷ���ø�������
//3.LibScanner����flb�������ļ��������еĺ��������������õ��ĺ������ݽ��бȽϣ��Ӷ��ж��Ƿ��ǿ⺯��

//���ߣ�leeeryan	leeeryan@gmail.com

#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include "PEInfo.h"
#include "LibScanner.h"
#include "PVdasm\\Disasm.h"

void ModifySuffix(PCHAR filename,PCHAR pSuffix)
{
	PCHAR pDest=strrchr(filename,'.');
	do 
	{
		*pDest++=*pSuffix++;
	} while (*pDest&&*pSuffix);
	*pDest=0;
}
FILE* pLogFile=NULL;
int _tmain(int argc, _TCHAR* argv[])
{
	FILE* pFile=NULL;
	PBYTE PEImage=NULL;
	PBYTE pCodeData=NULL;
	DWORD baseAddress=0;
	unsigned int CodeDatSize=0;
	BYTE MajorLinkerVersion=0;
	PCHAR PEFileNam="..\\test.exe";
	CHAR  LogFileNam[MAX_PATH];

#pragma region ����PE�ļ� 
	fopen_s(&pFile,PEFileNam,"rb");
	if(!pFile)
	{
		printf("Error:Can't open %s",PEFileNam);
		return 0;
	}
	const unsigned int fileLen=_filelength(_fileno(pFile));
	PEImage=new BYTE[fileLen];
	fread_s(PEImage,fileLen,fileLen,1,pFile);
	fclose(pFile);
#pragma endregion 
	
#pragma region ����PE�ļ�
	CPEInfo PEInfo;
	if(!PEInfo.Parse(PEImage))
		return 0;
	pCodeData=PEInfo.GetCodeData();
	CodeDatSize=PEInfo.GetCodeDataSize();
	MajorLinkerVersion=PEInfo.GetMajorLinkerVersion();
	baseAddress=PEInfo.GetBaseAddress();
#pragma endregion 
	//��ʼ��LibScanner�����м��ض��ڵ�flb�ļ�
	if(!InitLibScanner(MajorLinkerVersion))
		return 0;
	
#pragma region ���÷��������
	strcpy_s(LogFileNam,MAX_PATH,PEFileNam);
	ModifySuffix(LogFileNam,".log");
	fopen_s(&pLogFile,LogFileNam,"wb");
	if(!pFile)
	{
		printf("Error:Can't open %s\n",PEFileNam);
		return 0;
	}
	PVDasm(pCodeData,CodeDatSize,baseAddress,pLogFile);
	fclose(pLogFile);
#pragma endregion 
	
	printf("PEFile %s Analysis Succeed!\n",PEFileNam);
	delete[] PEImage;
	return 0;
}


extern void ShowDecoded(DISASSEMBLY* pDisasm,FILE* pfile);
//call(0xe8)ָ����������ĺ���
void CallHandle(PBYTE pCallData,DISASSEMBLY* pDisasm)
{
	PCHAR pLibFuncNam=NULL;
	while(*pCallData==0xe9)//����JMP
	{
		int jumpOffset=*(int*)(pCallData+1);
		pCallData=pCallData+jumpOffset+5;
	}
	//�жϴ˺����Ƿ��ǿ⺯��
	pLibFuncNam=CheckIfLibFunc(pCallData);
	if(pLibFuncNam)
	{
		ShowDecoded(pDisasm,pLogFile);
		fprintf_s(pLogFile,"Call LibFunc:%s\n",pLibFuncNam);
	}
}