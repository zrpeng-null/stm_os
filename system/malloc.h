/*************************************************************************************************
                Copyright (C) 2012-2022, Ray Chou, Innovation Laboratory
                              All rights reserved

                                �ڴ���� ��������
--�ļ���    : malloc.h
--�汾      : V1.0
--����      : �����ܲ�
--��������  : 2013/5/15
--�޸�����  : 2013/5/15
--ע��      : 1) stm32f10x��RAMΪ20kb ~ 64kb
              2) �ڴ��������һ��ȫ�ֱ�����ֻҪ��Ӧ���ڴ�������1���ͱ�ʾ��Ӧ������ڴ�鱻
                 ռ����, ��Ϊ��ȫ�ֱ�����������������ͨ������ڴ�����Ϳ���֪���жγ���ʹ����
                 ��һ���ڴ档
                 �ڴ��ͷţ�һ���Ϊ���������ڴ�������0��������ָ��
**************************************************************************************************
--�޸���ʷ��
*************************************************************************************************/
#ifndef _MALLOC_H_
#define _MALLOC_H_

#if 0
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;
#else
#include "stm32f10x.h"
#endif

#ifndef NULL
#define NULL 0
#endif

#define SRAMIN  0   //�ڲ��ڴ��
#define SRAMEX  1   //�ⲿ�ڴ��


//mem1�ڴ�����趨.mem1��ȫ�����ڲ�SRAM����
#define MEM1_BLOCK_SIZE         32                              //�ڴ���СΪ32�ֽ�
#define MEM1_MAX_SIZE           40*1024                         //�������ڴ� 40K
#define MEM1_ALLOC_TABLE_SIZE   MEM1_MAX_SIZE/MEM1_BLOCK_SIZE   //�ڴ���С 1280���ڴ�

//mem2�ڴ�����趨.mem2���ڴ�ش����ⲿSRAM����,�����Ĵ����ڲ�SRAM����
#define MEM2_BLOCK_SIZE         32                              //�ڴ���СΪ32�ֽ�
#define MEM2_MAX_SIZE           960*1024                        //�ڴ�ش�С
#define MEM2_ALLOC_TABLE_SIZE   MEM2_MAX_SIZE/MEM2_BLOCK_SIZE   //�ڴ���С ���ٿ��ڴ��


//�ڴ���������
struct _m_mallco_dev
{
    void (*init)(u8);                   //��ʼ��
    u8 (*perused)(u8);                  //�ڴ�ʹ����
    u8  *membase[2];                    //�ڴ�� ����2��������ڴ�
    u16 *memmap[2];                     //�ڴ����״̬��
    u8  memrdy[2];                      //�ڴ�����Ƿ����
};
extern struct _m_mallco_dev mallco_dev;  //��mallco.c���涨��

void mymemset(void *s, u8 c, u32 count); //�����ڴ�
void mymemcpy(void *des, void *src, u32 n); //�����ڴ�
void mem_init(u8 memx);                  //�ڴ�����ʼ������(��/�ڲ�����)
u32 mem_malloc(u8 memx, u32 size);       //�ڴ����(�ڲ�����)
u8 mem_free(u8 memx, u32 offset);        //�ڴ��ͷ�(�ڲ�����)
u8 mem_perused(u8 memx);                 //����ڴ�ʹ����(��/�ڲ�����)
////////////////////////////////////////////////////////////////////////////////
//�û����ú���
void myfree(u8 memx, void *ptr);            //�ڴ��ͷ�(�ⲿ����)
void *mymalloc(u8 memx, u32 size);          //�ڴ����(�ⲿ����)
void *myrealloc(u8 memx, void *ptr, u32 size); //���·����ڴ�(�ⲿ����)

#endif
