#include"Dewarp.h"

int32_t CDewarp::StartFrame(uint16_t* Src, uint8_t* DewarpTable)
{
	int32_t i, j, index2;
	int32_t Offset = m_sCtrlParam.RegFullWidth * m_sCtrlParam.RegFullHeight;
	m_sInternalParam.pDataStartLine = m_sCtrlParam.RegStartH;
	m_sInternalParam.pDataValidLine = m_sCtrlParam.RegInitLinebuffHeight;
	
	m_sInternalParam.pRealMapWidth = (m_sCtrlParam.RegMapWidth << 2) + (m_sCtrlParam.RegMapWidth << 1) + (1 << m_sCtrlParam.RegBlockWidth);
	m_sInternalParam.pBlockWidth = 1 << m_sCtrlParam.RegBlockWidth;
	m_sInternalParam.pOutputLine = 0;
	for (i = 0; i < m_sCtrlParam.RegInitLinebuffHeight; i++)
	{
		index2 = (m_sInternalParam.pDataStartLine + i) * m_sCtrlParam.RegFullWidth + m_sCtrlParam.RegStartW;
		for (j = 0; j < m_sCtrlParam.RegInputWidth; j++)
		{
			m_sInternalParam.pLineBuffY[i][j] = Src[index2 + j];
			m_sInternalParam.pLineBuffUV[i][j] = Src[Offset +index2 + j];
		}
	}

	for (i = 0; i < 2; i++)
	{
		index2 = i * m_sInternalParam.pRealMapWidth;
		for (j = 0; j < m_sInternalParam.pRealMapWidth; j++)
		{
			m_sInternalParam.pTableLineBuff[i][j] = DewarpTable[index2 + j];
		}
	}
	m_sInternalParam.pMapStartLine = 2;
	return 1;
}
int32_t CDewarp::StartLine(int32_t RowIndex)
{
    int32_t x1, x2, y1, y2;
	m_sInternalParam.pOutputColumn = 0;
	m_sInternalParam.pCoorGainH = RowIndex & (m_sInternalParam.pBlockWidth - 1);
	
	x1 = m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pBlockWidth] + (m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pBlockWidth + 1] << 8) + (m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pBlockWidth + 2] << 16);
	x2 = m_sInternalParam.pTableLineBuff[1][m_sInternalParam.pBlockWidth] + (m_sInternalParam.pTableLineBuff[1][m_sInternalParam.pBlockWidth + 1] << 8) + (m_sInternalParam.pTableLineBuff[1][m_sInternalParam.pBlockWidth + 2] << 16);

	y1 = m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pBlockWidth + 3] + (m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pBlockWidth + 4] << 8) + (m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pBlockWidth + 5] << 16);
	y2 = m_sInternalParam.pTableLineBuff[1][m_sInternalParam.pBlockWidth + 3] + (m_sInternalParam.pTableLineBuff[1][m_sInternalParam.pBlockWidth + 4] << 8) + (m_sInternalParam.pTableLineBuff[1][m_sInternalParam.pBlockWidth + 5] << 16);


	m_sInternalParam.pCoorX[1] = (m_sInternalParam.pCoorGainH * (x2 - x1) >> m_sCtrlParam.RegBlockWidth) + x1;
	m_sInternalParam.pCoorY[1] = (m_sInternalParam.pCoorGainH * (y2 - y1) >> m_sCtrlParam.RegBlockWidth) + y1;

	return 1;
}
int32_t CDewarp::GetInputROllingNum(int32_t RowIndex)
{
	m_sInternalParam.pInputNum = m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pCoorGainH] & 0x0F;
	m_sInternalParam.pRollingNum = (m_sInternalParam.pTableLineBuff[0][m_sInternalParam.pCoorGainH] >> 4) & 0x0F;
	return 1;
}
int32_t CDewarp::UpdateCoor(int32_t ColumnIndex)
{
	int32_t x1, x2, y1, y2;
	m_sInternalParam.pCoorX[0] = m_sInternalParam.pCoorX[1];
	m_sInternalParam.pCoorY[0] = m_sInternalParam.pCoorY[1];
	int32_t Offset = m_sInternalParam.pBlockWidth + (ColumnIndex << 2) + (ColumnIndex << 1);
	
	x1 = m_sInternalParam.pTableLineBuff[0][Offset] + (m_sInternalParam.pTableLineBuff[0][Offset + 1] << 8) + (m_sInternalParam.pTableLineBuff[0][Offset + 2] << 16);
	x2 = m_sInternalParam.pTableLineBuff[1][Offset] + (m_sInternalParam.pTableLineBuff[1][Offset + 1] << 8) + (m_sInternalParam.pTableLineBuff[1][Offset + 2] << 16);

	y1 = m_sInternalParam.pTableLineBuff[0][Offset + 3] + (m_sInternalParam.pTableLineBuff[0][Offset + 4] << 8) + (m_sInternalParam.pTableLineBuff[0][Offset + 5] << 16);
	y2 = m_sInternalParam.pTableLineBuff[1][Offset + 3] + (m_sInternalParam.pTableLineBuff[1][Offset + 4] << 8) + (m_sInternalParam.pTableLineBuff[1][Offset + 5] << 16);

	m_sInternalParam.pCoorX[1] = (m_sInternalParam.pCoorGainH * (x2 - x1) >> m_sCtrlParam.RegBlockWidth) + x1;
	m_sInternalParam.pCoorY[1] = (m_sInternalParam.pCoorGainH * (y2 - y1) >> m_sCtrlParam.RegBlockWidth) + y1;
	return 1;
}

int32_t CDewarp::GetFinalCoor(int32_t ColumnIndex)
{
	int32_t gainY = ColumnIndex & (m_sInternalParam.pBlockWidth - 1);
	
	m_sInternalParam.pCoorFinalY[0] = (gainY * (m_sInternalParam.pCoorX[1] - m_sInternalParam.pCoorX[0]) >> m_sCtrlParam.RegBlockWidth) + m_sInternalParam.pCoorX[0];//X
	m_sInternalParam.pCoorFinalY[1] = (gainY * (m_sInternalParam.pCoorY[1] - m_sInternalParam.pCoorY[0]) >> m_sCtrlParam.RegBlockWidth) + m_sInternalParam.pCoorY[0];//Y
	
	if (0 == (ColumnIndex & 1))
	{
		int32_t gainUV = ((ColumnIndex << 1) + 1) & ((m_sInternalParam.pBlockWidth << 1) - 1);
		m_sInternalParam.pCoorFinalUV[0] = (gainUV * (m_sInternalParam.pCoorX[1] - m_sInternalParam.pCoorX[0]) >> (m_sCtrlParam.RegBlockWidth + 1)) + m_sInternalParam.pCoorX[0];//X
		m_sInternalParam.pCoorFinalUV[1] = (gainUV * (m_sInternalParam.pCoorY[1] - m_sInternalParam.pCoorY[0]) >> (m_sCtrlParam.RegBlockWidth + 1)) + m_sInternalParam.pCoorY[0];//Y
	}

	return 1;
}

int32_t CDewarp::GetRowColIndexGain()
{
	int32_t IntUV, FloatUV;
	m_sInternalParam.pYH[0] = clip(m_sInternalParam.pCoorFinalY[1] >> FLOATPRECISION, m_sCtrlParam.RegStartH, m_sCtrlParam.RegStartH + m_sCtrlParam.RegInputHeight - 1);
	m_sInternalParam.pYH[1] = clip(m_sInternalParam.pYH[0] + 1, m_sCtrlParam.RegStartH, m_sCtrlParam.RegStartH + m_sCtrlParam.RegInputHeight - 1);

	m_sInternalParam.pYW[0] = clip(m_sInternalParam.pCoorFinalY[0] >> FLOATPRECISION, m_sCtrlParam.RegStartW, m_sCtrlParam.RegStartW + m_sCtrlParam.RegInputWidth - 1);
	m_sInternalParam.pYW[1] = clip(m_sInternalParam.pYW[0] + 1, m_sCtrlParam.RegStartW, m_sCtrlParam.RegStartW + m_sCtrlParam.RegInputWidth - 1);

	m_sInternalParam.pYGain[0] = m_sInternalParam.pCoorFinalY[0] & PRECISIONMAX;
	m_sInternalParam.pYGain[1] = m_sInternalParam.pCoorFinalY[1] & PRECISIONMAX;

	m_sInternalParam.pUVH[0] = clip(m_sInternalParam.pCoorFinalUV[1] >> FLOATPRECISION, m_sCtrlParam.RegStartH, m_sCtrlParam.RegStartH + m_sCtrlParam.RegInputHeight - 1);
	m_sInternalParam.pUVH[1] = clip(m_sInternalParam.pUVH[0] + 1, m_sCtrlParam.RegStartH, m_sCtrlParam.RegStartH + m_sCtrlParam.RegInputHeight - 1);

	IntUV = m_sInternalParam.pCoorFinalUV[0] >> FLOATPRECISION;
	FloatUV = m_sInternalParam.pCoorFinalUV[0] & PRECISIONMAX;

	if (0==(IntUV & 1))
	{
		if (FloatUV < 2048)
		{
			m_sInternalParam.pUVW[1] = clip(IntUV >> 1, m_sCtrlParam.RegStartW >> 1, (m_sCtrlParam.RegStartW >> 1) + (m_sCtrlParam.RegInputWidth >> 1) - 1);
			m_sInternalParam.pUVW[0] = clip(m_sInternalParam.pUVW[1] - 1, m_sCtrlParam.RegStartW >> 1, (m_sCtrlParam.RegStartW >> 1) + (m_sCtrlParam.RegInputWidth >> 1) - 1);
			m_sInternalParam.pUVGain[0] = FloatUV + (PRECISIONHALF << 1) + PRECISIONHALF;
		}
		else
		{
			m_sInternalParam.pUVW[0] = clip(IntUV >> 1, m_sCtrlParam.RegStartW >> 1, (m_sCtrlParam.RegStartW >> 1) + (m_sCtrlParam.RegInputWidth >> 1) - 1);
			m_sInternalParam.pUVW[1] = clip(m_sInternalParam.pUVW[0] + 1, m_sCtrlParam.RegStartW >> 1, (m_sCtrlParam.RegStartW >> 1) + (m_sCtrlParam.RegInputWidth >> 1) - 1);
			m_sInternalParam.pUVGain[0] = FloatUV - PRECISIONHALF;
		}
	}
	else
	{
		m_sInternalParam.pUVW[0] = clip(IntUV >> 1, m_sCtrlParam.RegStartW >> 1, (m_sCtrlParam.RegStartW >> 1) + (m_sCtrlParam.RegInputWidth >> 1) - 1);
		m_sInternalParam.pUVW[1] = clip(m_sInternalParam.pUVW[0] + 1, m_sCtrlParam.RegStartW >> 1, (m_sCtrlParam.RegStartW >> 1) + (m_sCtrlParam.RegInputWidth >> 1) - 1);
		m_sInternalParam.pUVGain[0] = FloatUV + PRECISIONHALF;
	}
	
	m_sInternalParam.pUVGain[1] = m_sInternalParam.pCoorFinalUV[1] & PRECISIONMAX;

	return 1;
	
}

int32_t CDewarp::ProcessOnePixelY()
{
	int32_t temp1, temp2;
	m_sInternalParam.pPixelY[0][0] = m_sInternalParam.pLineBuffY[m_sInternalParam.pYH[0] - m_sInternalParam.pDataStartLine][m_sInternalParam.pYW[0] - m_sCtrlParam.RegStartW];
	m_sInternalParam.pPixelY[0][1] = m_sInternalParam.pLineBuffY[m_sInternalParam.pYH[0] - m_sInternalParam.pDataStartLine][m_sInternalParam.pYW[1] - m_sCtrlParam.RegStartW];
	m_sInternalParam.pPixelY[1][0] = m_sInternalParam.pLineBuffY[m_sInternalParam.pYH[1] - m_sInternalParam.pDataStartLine][m_sInternalParam.pYW[0] - m_sCtrlParam.RegStartW];
	m_sInternalParam.pPixelY[1][1] = m_sInternalParam.pLineBuffY[m_sInternalParam.pYH[1] - m_sInternalParam.pDataStartLine][m_sInternalParam.pYW[1] - m_sCtrlParam.RegStartW];

	temp1 = ((m_sInternalParam.pYGain[0] * (m_sInternalParam.pPixelY[0][1] - m_sInternalParam.pPixelY[0][0]) + PRECISIONHALF) >> FLOATPRECISION) + m_sInternalParam.pPixelY[0][0];
	temp2 = ((m_sInternalParam.pYGain[0] * (m_sInternalParam.pPixelY[1][1] - m_sInternalParam.pPixelY[1][0]) + PRECISIONHALF) >> FLOATPRECISION) + m_sInternalParam.pPixelY[1][0];

	return   clip(((m_sInternalParam.pYGain[1] * (temp2 - temp1) + PRECISIONHALF) >> FLOATPRECISION) + temp1, 0, MAX_DATA_VALUE);

	//temp1 = ((m_sInternalParam.pYGain[1] * (m_sInternalParam.pPixelY[1][0] - m_sInternalParam.pPixelY[0][0]) + PRECISIONHALF) >> FLOATPRECISION) + m_sInternalParam.pPixelY[0][0];
	//temp2 = ((m_sInternalParam.pYGain[1] * (m_sInternalParam.pPixelY[1][1] - m_sInternalParam.pPixelY[0][1]) + PRECISIONHALF) >> FLOATPRECISION) + m_sInternalParam.pPixelY[0][1];

	//return   clip(((m_sInternalParam.pYGain[0] * (temp2 - temp1) + PRECISIONHALF) >> FLOATPRECISION) + temp1, 0, MAX_DATA_VALUE);
}

int32_t CDewarp::ProcessOnePixelUV(int32_t Offset)
{
	int32_t temp1, temp2;
	m_sInternalParam.pPixelUV[0][0] = m_sInternalParam.pLineBuffUV[m_sInternalParam.pUVH[0] - m_sInternalParam.pDataStartLine][(m_sInternalParam.pUVW[0] << 1) + Offset - m_sCtrlParam.RegStartW];
	m_sInternalParam.pPixelUV[0][1] = m_sInternalParam.pLineBuffUV[m_sInternalParam.pUVH[0] - m_sInternalParam.pDataStartLine][(m_sInternalParam.pUVW[1] << 1) + Offset - m_sCtrlParam.RegStartW];
	m_sInternalParam.pPixelUV[1][0] = m_sInternalParam.pLineBuffUV[m_sInternalParam.pUVH[1] - m_sInternalParam.pDataStartLine][(m_sInternalParam.pUVW[0] << 1) + Offset - m_sCtrlParam.RegStartW];
	m_sInternalParam.pPixelUV[1][1] = m_sInternalParam.pLineBuffUV[m_sInternalParam.pUVH[1] - m_sInternalParam.pDataStartLine][(m_sInternalParam.pUVW[1] << 1) + Offset - m_sCtrlParam.RegStartW];

	temp1 = ((m_sInternalParam.pUVGain[0] * (m_sInternalParam.pPixelUV[0][1] - m_sInternalParam.pPixelUV[0][0]) + (1 << FLOATPRECISION)) >> (FLOATPRECISION + 1)) + m_sInternalParam.pPixelUV[0][0];
	temp2 = ((m_sInternalParam.pUVGain[0] * (m_sInternalParam.pPixelUV[1][1] - m_sInternalParam.pPixelUV[1][0]) + (1 << FLOATPRECISION)) >> (FLOATPRECISION + 1)) + m_sInternalParam.pPixelUV[1][0];
	return clip(((m_sInternalParam.pUVGain[1] * (temp2 - temp1) + PRECISIONHALF) >> FLOATPRECISION) + temp1, 0, MAX_DATA_VALUE);

	//temp1 = ((m_sInternalParam.pUVGain[1] * (m_sInternalParam.pPixelUV[1][0] - m_sInternalParam.pPixelUV[0][0]) + PRECISIONHALF) >> FLOATPRECISION) + m_sInternalParam.pPixelUV[0][0];
	//temp2 = ((m_sInternalParam.pUVGain[1] * (m_sInternalParam.pPixelUV[1][1] - m_sInternalParam.pPixelUV[0][1]) + PRECISIONHALF) >> FLOATPRECISION) + m_sInternalParam.pPixelUV[0][1];
	//return clip(((m_sInternalParam.pUVGain[0] * (temp2 - temp1) + (1 << FLOATPRECISION)) >> (FLOATPRECISION + 1)) + temp1, 0, MAX_DATA_VALUE);
}
int32_t CDewarp::ProcOneLine(uint16_t* DstY, uint16_t* DstUV, int32_t RowIndex)
{
	int32_t i, j, index;
	
	
	while(m_sInternalParam.pOutputColumn< m_sCtrlParam.RegFullWidth)
	{
		if (0 == (m_sInternalParam.pOutputColumn & (m_sInternalParam.pBlockWidth - 1)))
		{
			UpdateCoor((m_sInternalParam.pOutputColumn >> m_sCtrlParam.RegBlockWidth) + 1);
		}
		GetFinalCoor(m_sInternalParam.pOutputColumn);
		GetRowColIndexGain();
		DstY[m_sInternalParam.pOutputColumn] = ProcessOnePixelY();
		DstUV[m_sInternalParam.pOutputColumn] = ProcessOnePixelUV(m_sInternalParam.pOutputColumn & 1);
		m_sInternalParam.pOutputColumn += 1;
	}
	
	return 1;
}

int32_t CDewarp::GetDataToLineBuffer(uint16_t* Src)
{
	if (m_sInternalParam.pInputNum > 0)
	{
		int32_t i, j, index2;
		int32_t Offset = m_sCtrlParam.RegFullWidth * m_sCtrlParam.RegFullHeight;
		
		for (i = 0; i < m_sInternalParam.pInputNum; i++)
		{
			index2 = (m_sInternalParam.pDataValidLine + m_sInternalParam.pDataStartLine + i) * m_sCtrlParam.RegFullWidth + m_sCtrlParam.RegStartW;
			for (j = 0; j < m_sCtrlParam.RegInputWidth; j++)
			{
				m_sInternalParam.pLineBuffY[m_sInternalParam.pDataValidLine + i][j] = Src[index2 + j];
				m_sInternalParam.pLineBuffUV[m_sInternalParam.pDataValidLine + i][j] = Src[Offset + index2 + j];
			}
		}
		m_sInternalParam.pDataValidLine += m_sInternalParam.pInputNum;
	}

	return 1;
}

int32_t CDewarp::RollingDataOutLineBuffer()
{
	if (m_sInternalParam.pRollingNum > 0)
	{
		int32_t i, j;
		for (i = 0; i < m_sInternalParam.pDataValidLine- m_sInternalParam.pRollingNum; i++)
		{
			for (j = 0; j < m_sCtrlParam.RegInputWidth; j++)
			{
				m_sInternalParam.pLineBuffY[i][j] = m_sInternalParam.pLineBuffY[i + m_sInternalParam.pRollingNum][j];
				m_sInternalParam.pLineBuffUV[i][j] = m_sInternalParam.pLineBuffUV[i + m_sInternalParam.pRollingNum][j];
			}
		}

		for (i = m_sInternalParam.pDataValidLine - m_sInternalParam.pRollingNum; i < m_sInternalParam.pDataValidLine; i++)
		{
			for (j = 0; j < m_sCtrlParam.RegInputWidth; j++)
			{
				m_sInternalParam.pLineBuffY[i][j] = 0;
				m_sInternalParam.pLineBuffUV[i][j] = 0;
			}
		}

		m_sInternalParam.pDataValidLine -= m_sInternalParam.pRollingNum;
		m_sInternalParam.pDataStartLine += m_sInternalParam.pRollingNum;
	}

	return 1;
}

int32_t CDewarp::EndLine()
{
	RollingDataOutLineBuffer();
	if ((m_sInternalParam.pBlockWidth - 1) == ((m_sInternalParam.pOutputLine) & (m_sInternalParam.pBlockWidth - 1)))
	{
		RollingDataOutTableBuffer();
	}
	return 1;
}

int32_t CDewarp::GetCoorToTableBuffer(uint8_t* DewarpTable)
{
	int32_t index, j;
	index = m_sInternalParam.pMapStartLine * m_sInternalParam.pRealMapWidth;
	for (j = 0; j < m_sInternalParam.pRealMapWidth; j++)
	{
		m_sInternalParam.pTableLineBuff[2][j] = DewarpTable[index + j];
	}

	m_sInternalParam.pMapStartLine += 1;
	return 1;
}

int32_t CDewarp::RollingDataOutTableBuffer()
{
	int32_t i, j;
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < m_sInternalParam.pRealMapWidth; j++)
		{
			m_sInternalParam.pTableLineBuff[i][j] = m_sInternalParam.pTableLineBuff[i + 1][j];
		}
	}

	for (j = 0; j < m_sInternalParam.pRealMapWidth; j++)
	{
		m_sInternalParam.pTableLineBuff[2][j] = 0;
	}

	return 1;
}

bool CDewarp::ProcessImage(uint16_t* Dst, uint16_t* Src, uint8_t* DewarpTable)
{
	if (m_sCtrlParam.RegEnable)
	{
		int32_t i, j, index;

		uint16_t* pYDst = Dst;
		uint16_t* pUVDst = Dst + m_sCtrlParam.RegFullWidth * m_sCtrlParam.RegFullHeight;

		StartFrame(Src, DewarpTable);

		while(m_sInternalParam.pOutputLine< m_sCtrlParam.RegFullHeight)
		{
			StartLine(m_sInternalParam.pOutputLine);
			ProcOneLine(pYDst, pUVDst, m_sInternalParam.pOutputLine);
			// parallel Process start
			if (3 == (m_sInternalParam.pOutputLine & (m_sInternalParam.pBlockWidth - 1)))
			{
				GetCoorToTableBuffer(DewarpTable);
			}
			// serial Process start
			GetInputROllingNum(m_sInternalParam.pOutputLine);
			GetDataToLineBuffer(Src);
			// serial Process end
			
			// parallel Process end
			EndLine();
			pYDst += m_sCtrlParam.RegFullWidth;
			pUVDst += m_sCtrlParam.RegFullWidth;
			m_sInternalParam.pOutputLine += 1;
		}
	}
	else
	{
		memcpy(Dst, Src, m_sCtrlParam.RegFullWidth * m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
	}
	
	return 1;
}