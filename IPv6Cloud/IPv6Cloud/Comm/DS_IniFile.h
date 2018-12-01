/****************************************************************
Copyright (C), 2012-2014,  *** Tech. Co., Ltd.
File name:   IniFile.h
Author:      Feb
Version:     DS1.0
Date:        2012.01
Description: ini�ļ���ش���
******************************************************************/
#ifndef _READ_WRITE_CONFIG_FILE_H
#define _READ_WRITE_CONFIG_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SuccessRet 1;
#define FailedRet  0;

#define MAX_CFG_BUF                              512 

#define CFG_OK                                     0 
#define CFG_SECTION_NOT_FOUND                     -1 
#define CFG_KEY_NOT_FOUND                         -2 
#define CFG_ERR                                  -10 
#define CFG_ERR_FILE                             -10 
#define CFG_ERR_OPEN_FILE                        -10 
#define CFG_ERR_CREATE_FILE                      -11 
#define CFG_ERR_READ_FILE                        -12 
#define CFG_ERR_WRITE_FILE                       -13 
#define CFG_ERR_FILE_FORMAT                      -14 
#define CFG_ERR_SYSTEM                           -20 
#define CFG_ERR_SYSTEM_CALL                      -20 
#define CFG_ERR_INTERNAL                         -21 
#define CFG_ERR_EXCEED_BUF_SIZE                  -22 

#define COPYF_OK                                   0 
#define COPYF_ERR_OPEN_FILE                      -10 
#define COPYF_ERR_CREATE_FILE                    -11 
#define COPYF_ERR_READ_FILE                      -12 
#define COPYF_ERR_WRITE_FILE                     -13 

#define TXTF_OK                                    0 
#define TXTF_ERR_OPEN_FILE                        -1 
#define TXTF_ERR_READ_FILE                        -2 
#define TXTF_ERR_WRITE_FILE                       -3 
#define TXTF_ERR_DELETE_FILE                      -4 
#define TXTF_ERR_NOT_FOUND                        -5 

#define CFG_ssl  '['
#define CFG_ssr  ']'                 /*���־��Section Symbol --�ɸ���������Ҫ���ж�����ģ��� { }��*/
#define CFG_nis  ':'                 /*name �� index ֮��ķָ��� */
#define CFG_nts  ';'                 /*ע�ͷ�,ֻ�������׽���ע��*/

class CIniFile  
{
    
public:
    CIniFile();
    virtual ~CIniFile();

    //ȥ���ַ����ұߵĿ��ַ�
    char * strtrimr(char * buf);
	
	//ȥ���ַ�����ߵĿ��ַ�
    char * strtriml(char * buf);

	//���ļ��ж�ȡһ��
    int  FileGetLine(FILE *fp, char *buffer, int maxlen);

	//����key��value
    int  SplitKeyValue(char *buf, char **key, char **val);

	//�ļ�����
    int  FileCopy(void *source_file, void *dest_file);

	//����sectionΪname��index
    int  SplitSectionToNameIndex(char *section, char **name, char **index);

	//�ϳ�name��indexsectionΪsection
    int  JoinNameIndexToSection(char **section, char *name, char *index);

	//���key��ֵ
    int  ConfigGetKey(void *CFG_file, void *section, void *key, void *buf);

	//����key��ֵ
    int  ConfigSetKey(void *CFG_file, void *section, void *key, void *buf);

	//�������section
    int  ConfigGetSections(void *CFG_file, char *sections[]);

	//�������key������
    int  ConfigGetKeys(void *CFG_file, void *section, char *keys[]);

    int  CFG_section_line_no, CFG_key_line_no, CFG_key_lines;     
};

#endif // !defined(AFX_INIFILE_H__998CAFDF_E1B8_4DD5_B29A_1877455FE305__INCLUDED_)
