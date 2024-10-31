#include"CYuvCrop.h"

bool CYuvCrop::ProcessImage(uint16_t* Dst, uint16_t* Src)
{
	if (m_sCtrlParam.RegEnable)
	{
		int32_t i, j, indexs, indexd;
		uint16_t* pYSrc = Src;
		uint16_t* pUVSrc = Src + m_sCtrlParam.RegInputWidth * m_sCtrlParam.RegInputHeight;
		uint16_t* pYDst = Dst;
		uint16_t* pUVDst = Dst + m_sCtrlParam.RegCropWidth * m_sCtrlParam.RegCropHeight;
		for (i = 0; i < m_sCtrlParam.RegCropHeight; i++)
		{
			indexs = (i + m_sCtrlParam.RegStartH) * m_sCtrlParam.RegInputWidth + m_sCtrlParam.RegStartW;
			indexd = i * m_sCtrlParam.RegCropWidth;
			for (j = 0; j < m_sCtrlParam.RegCropWidth; j++)
			{
				pYDst[indexd + j] = pYSrc[indexs + j];
				pUVDst[indexd + j] = pUVSrc[indexs + j];
			}
		}
		
	}
	else
	{
		memcpy(Dst, Src, m_sCtrlParam.RegInputWidth * m_sCtrlParam.RegInputHeight * 2 * sizeof(uint16_t));
	}
	
	return 1;
}