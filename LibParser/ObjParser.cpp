//���ã�������lib�ļ�����ȡ����Ŀ���ļ�(obj)��Ա(COFF�ļ���ʽ)
//������ȡ������������������ �ֱ�д��nam dat�ļ�
//����������ƫ�ƣ���������С����������ƫ�ƣ��������ݴ�С ���� FuncHeaderTable

//���ߣ�leeeryan	leeeryan@gmail.com


#include "ObjParser.h"

CObjParser::CObjParser(void):
m_pObjImage(NULL),m_pFileHeader(NULL),m_pSectionHeader(NULL),m_pRelocation(NULL),
m_pStrings(NULL),m_pNamFile(NULL),m_pDatFile(NULL),m_pFuncTable(NULL)
{
}

CObjParser::~CObjParser(void)
{
	//�����ͷţ���Ϊ����֮ǰm_pLibImageָ���һ�����ڴ��һ���֣��ᵼ���ظ��ͷ��ڴ�
	//if(m_pObjImage)delete m_pObjImage;
}
//��PE COFF �ļ���ʽ�� ���ᵽ���������ű��¼��ʽ֮һ���������壻
//�����к�����С���Լ�ָ���¸��������ű��������������Ĺ��ߣ�
//��ʵ��֤�����������ã��������ű�������û�и������ű�
BOOL CObjParser::Parse(PBYTE objImage,FILE* pNamFile,FILE* pDatFile,FuncHeaderTable* funcTable)
{
	m_pObjImage=objImage;
	m_pNamFile=pNamFile;
	m_pDatFile=pDatFile;
	m_pFuncTable=funcTable;
	
	m_pFileHeader=(PIMAGE_FILE_HEADER)objImage;
	m_pSectionHeader=(PIMAGE_SECTION_HEADER)(objImage+sizeof(IMAGE_FILE_HEADER));
	m_pSymbol=(PIMAGE_SYMBOL)(objImage+m_pFileHeader->PointerToSymbolTable);
	m_pStrings=(PCHAR)(m_pSymbol+m_pFileHeader->NumberOfSymbols);
	
	//ֻ����������
	PIMAGE_SYMBOL pSymbol;
	//�½�һ������ͷ
	FuncHeader funcHeader;
	//�������ű�
	for (DWORD i=0;i<m_pFileHeader->NumberOfSymbols;i++)
	{
		pSymbol=m_pSymbol+i;
// 		�洢���ΪEXTERNAL��2����Type ���ֵ��������һ��������0x20��
//		�Լ�SectionNumber ���ֵ����0�����ͱ�־�ź����Ŀ�ͷ
		if(ISFCN(pSymbol->Type)&&pSymbol->SectionNumber>0
			&&pSymbol->StorageClass==IMAGE_SYM_CLASS_EXTERNAL)
		{
			memset(&funcHeader,0,sizeof(funcHeader));

			GetNameofSymb(pSymbol,funcHeader);	
			GetDataofSymb(pSymbol,funcHeader);	

			m_pFuncTable->push_back(funcHeader);
		}
		//ֱ�������������ű�
		i+=pSymbol->NumberOfAuxSymbols;
	}

	return TRUE;
}
void CObjParser::GetNameofSymb(PIMAGE_SYMBOL pSymbol,FuncHeader& funcHeader)
{
	PCHAR pName=NULL;
	CHAR shortNam[9]={0};
	//����������Ƴ��Ȳ�����8 ���ֽڣ���ô���ű��ShortName ��
	//���ǰ��������������һ��8 �ֽڳ������飻
	if (pSymbol->N.Name.Short)
	{
		//pName= (PCHAR)pSymbol->N.ShortName;
		//ע�⣺��������������ռ��8���ֽڣ��Ǿ�û��NULL�������ˣ�
		//���Բ��ܼ򵥵�������ķ���
		
		memcpy_s(shortNam,9,pSymbol->N.ShortName,8);

		pName=shortNam;
	} 
	// ����Ļ������������ַ������е�һ��ƫ�Ƶ�ַ
	else
	{
		pName= m_pStrings+pSymbol->N.Name.Long;
	}

	//��¼ƫ��
	if (m_pFuncTable->size()==0)
	{
		funcHeader.NameOff=0;
	} 
	else
	{
		FuncHeader& funcHeadPrev=m_pFuncTable->at(m_pFuncTable->size()-1);
		funcHeader.NameOff=funcHeadPrev.NameOff+funcHeadPrev.NameSize;
	}
	//��¼��С
	funcHeader.NameSize=strlen(pName)+1;
	//д��nam�ļ�
	fwrite(pName,funcHeader.NameSize,1,m_pNamFile);
	fflush(m_pNamFile);
}
//��ú�������
void CObjParser::GetDataofSymb(PIMAGE_SYMBOL pSymbol,FuncHeader& funcHeader)
{
	PIMAGE_SECTION_HEADER pISH = m_pSectionHeader+(pSymbol->SectionNumber-1);//SectionNumber��1 ��ʼ������
	if(!pISH)
	{
		MessageBox(NULL,"Get SectionHeader Error!","Error",MB_ICONWARNING);
		return;
	}
	//��¼ƫ��
	if (m_pFuncTable->size()==0)
	{
		funcHeader.DataOff=0;
	} 
	else
	{
		FuncHeader& funcHeadPrev=m_pFuncTable->at(m_pFuncTable->size()-1);
		funcHeader.DataOff=funcHeadPrev.DataOff+funcHeadPrev.DataSize;
	}	
	//��¼��С
	//���ּ��㺯����С�ķ�����׼ȷ����������Ĭ�����pSymbol���ڽڴ�pSymbol->Valueƫ�ƴ���ʼ��
	//�ڽ�������pSymbol����Ӧ�ĺ������ݣ����п��ܴ˽ڻ�����������������
	//�����󲿷����ǳ��õĺ������ǵ�һ���������ȷ��������С�ֱȽϸ��ӣ���ռ�����׷���
	//�ýڴ�СSizeOfRawData-�����ڴ˽ڵ�ƫ��Value
	funcHeader.DataSize=pISH->SizeOfRawData-pSymbol->Value;
	//��־�ض�λ λ��
	MarkRelocatePos(pISH);
	//��ȡ��������
	PBYTE funData=m_pObjImage+pISH->PointerToRawData+pSymbol->Value;
	//д��dat�ļ�
	fwrite(funData,funcHeader.DataSize,1,m_pDatFile);
	fflush(m_pDatFile);
}
//��־�ض�λ��Ϣ
void CObjParser::MarkRelocatePos(PIMAGE_SECTION_HEADER pISH)
{
	//���ĸ��ֽ�0��־�ض�λ��Ϣλ��
	DWORD pReloMark=0;
	DWORD modifyOff=0;
	//����ض�λ��
	PIMAGE_RELOCATION pIR = (PIMAGE_RELOCATION)(m_pObjImage + pISH->PointerToRelocations);
	//�ض�λ���С
	DWORD RefCount = pISH->NumberOfRelocations;
	for(DWORD i =0;i<RefCount;i++)
	{
		//���ض�λƫ��
		modifyOff=pISH->PointerToRawData+pIR[i].VirtualAddress;
		//�޶�
		memcpy_s(m_pObjImage+modifyOff,4,&pReloMark,4);		
	}
}	