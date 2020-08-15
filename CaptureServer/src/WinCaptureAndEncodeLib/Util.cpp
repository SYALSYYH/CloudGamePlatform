#include "StdAfx.h"
#include "Util.h"
#include <windows.h>

/************************************************************************/
//��������TransRGB32ToRGB24                              
//����:RGB565ͼ���ʽת����RGBͼ���ʽ
//�������
//@ pcDes			unsigned char*	Ŀ�����ݵ�ַָ��
//@ nDesSize		int				Ŀ�����ݴ�С
//@ pcSrc			unsigned char*	Դ���ݵ�ַָ��
//@ nSrcSize		int				Դ���ݴ�С
//@ nSrcWidth		int				Դͼ���
//@ nSrcHeight		int				Դͼ���
//�������
//true:�ɹ�;false:ʧ��
/************************************************************************/
bool TransRGB32ToRGB24(unsigned char* pcDes, int nDesSize, unsigned char* pcSrc, int nSrcSize, int nSrcWidth, int nSrcHeight)
{
	unsigned char* pcRgb24 = pcDes;
	unsigned char* psRgb32 = pcSrc;
	for (int i = 0; i < nSrcWidth*nSrcHeight; i++)
	{
		memcpy(pcRgb24, psRgb32, sizeof(char) * 3);
		pcRgb24 += 3;
		psRgb32 += 4;
	}

	return true;
}

/************************************************************************/
//��������TransRGB24ToYUV420                              
//����:RGB24ͼ���ʽת����YUV420ͼ���ʽ
//�������
//@ pcDes			unsigned char*	Ŀ�����ݵ�ַָ��
//@ nDesSize		int				Ŀ�����ݴ�С
//@ pcSrc			unsigned char*	Դ���ݵ�ַָ��
//@ nSrcSize		int				Դ���ݴ�С
//@ nSrcWidth		int				Դͼ���
//@ nSrcHeight		int				Դͼ���
//�������
//true:�ɹ�;false:ʧ��
/************************************************************************/
bool TransRGB24ToYUV420(unsigned char * pcDes, int nDesSize, unsigned char* pcSrc, int nSrcSize, int nSrcWidth,int nSrcHeight)
{
	bool bReturn = true;

	int nIndex_W = 0;
	int nIndex_H = 0;

	unsigned char * pBuffer_Y = NULL;
	unsigned char * pBuffer_U = NULL;
	unsigned char * pBuffer_V = NULL;

	unsigned char * pBuffer_RGB = NULL;

	memset(pcDes,0,nDesSize);

	int nSize = nSrcWidth * nSrcHeight + (nSrcWidth * nSrcHeight)/2;
	if(nSize != nDesSize || NULL == pcDes || NULL == pcSrc)
	{
		bReturn = false;
	}
	else
	{
		pBuffer_Y = pcDes;
		pBuffer_V = pcDes + nSrcWidth * nSrcHeight;
		pBuffer_U = pBuffer_V + (nSrcWidth * nSrcHeight * 1/4);

		unsigned char Y, U, V, R, G, B;

		for (nIndex_H = 0; nIndex_H< nSrcHeight; nIndex_H ++)
		{
			pBuffer_RGB = pcSrc + nSrcWidth * (nSrcHeight - 1 - nIndex_H) * 3;

			for (nIndex_W = 0;nIndex_W < nSrcWidth; nIndex_W ++)
			{
				R = *(pBuffer_RGB++);
				G = *(pBuffer_RGB++);
				B = *(pBuffer_RGB++);

				Y = (unsigned char)( ( 66 * R + 129 * G +  25 * B + 128) >> 8) + 16  ;          
				U = (unsigned char)( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128 ;          
				V = (unsigned char)( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128 ;

				*(pBuffer_Y ++) = max( 0, min(Y, 255 ));

				if (nIndex_H % 2 == 0 && nIndex_W % 2 ==0)
				{
					if (255 < U)
					{
						U = 255;
					}

					if (0 > U)
					{
						U = 0;
					}

					*(pBuffer_U ++) = U;
					//��u����
				}
				else
				{
					//��v����
					if (nIndex_W % 2==0)
					{
						if (255 < V)
						{
							V = 255;
						}
						if (0 > V)
						{
							V = 0;
						}
						*(pBuffer_V ++) = V;
					}
				}
			}
		}
	}

	return bReturn;
}

bool TransRGB32ToYUV420(unsigned char * pcDes, int nDesSize, unsigned char* pcSrc, int nSrcSize, int nSrcWidth, int nSrcHeight)
{
	bool bReturn = true;

	int nIndex_W = 0;
	int nIndex_H = 0;

	unsigned char * pBuffer_Y = NULL;
	unsigned char * pBuffer_U = NULL;
	unsigned char * pBuffer_V = NULL;

	unsigned char * pBuffer_RGB = NULL;

	memset(pcDes, 0, nDesSize);

	int nSize = nSrcWidth * nSrcHeight + (nSrcWidth * nSrcHeight) / 2;
	if (nSize != nDesSize || NULL == pcDes || NULL == pcSrc)
	{
		bReturn = false;
	}
	else
	{
		pBuffer_Y = pcDes;
		pBuffer_V = pcDes + nSrcWidth * nSrcHeight;
		pBuffer_U = pBuffer_V + (nSrcWidth * nSrcHeight * 1 / 4);

		unsigned char Y, U, V, R, G, B, A;

		for (nIndex_H = 0; nIndex_H< nSrcHeight; nIndex_H++)
		{
			//pBuffer_RGB = pcSrc + nSrcWidth * (nSrcHeight - 1 - nIndex_H) * 4;
			pBuffer_RGB = pcSrc + nSrcWidth * nIndex_H * 4;

			for (nIndex_W = 0; nIndex_W < nSrcWidth; nIndex_W++)
			{
				R = *(pBuffer_RGB++);
				G = *(pBuffer_RGB++);
				B = *(pBuffer_RGB++);
				A = *(pBuffer_RGB++);

				Y = (unsigned char)((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
				U = (unsigned char)((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
				V = (unsigned char)((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;

				*(pBuffer_Y++) = max(0, min(Y, 255));

				if (nIndex_H % 2 == 0 && nIndex_W % 2 == 0)
				{
					if (255 < U)
					{
						U = 255;
					}

					if (0 > U)
					{
						U = 0;
					}

					*(pBuffer_U++) = U;
					//��u����
				}
				else
				{
					//��v����
					if (nIndex_W % 2 == 0)
					{
						if (255 < V)
						{
							V = 255;
						}
						if (0 > V)
						{
							V = 0;
						}
						*(pBuffer_V++) = V;
					}
				}
			}
		}
	}

	return bReturn;
}
