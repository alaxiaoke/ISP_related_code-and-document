#include "dither.hpp"

void CDither::WriteOneLineToBuffer(ushort* Src, uint32_t RowIndex)
{
	for (int w = 0; w < m_sCtrlParam.RegWidth; w++)
	{
		pLineBuffY[w] = Src[RowIndex * m_sCtrlParam.RegWidth + w];
		pLineBuffUV[w] = Src[(m_sCtrlParam.RegHeight + RowIndex) * m_sCtrlParam.RegWidth + w];
	}
}

void CDither::GetOneLineToOutput(ushort* Dst, uint32_t RowIndex)
{
	for (int w = 0; w < m_sCtrlParam.RegWidth; w++)
	{
		Dst[RowIndex * m_sCtrlParam.RegWidth + w] = pLineBuffY[w];
		if (!m_sCtrlParam.RegEnableOutputFormat420)
			Dst[(m_sCtrlParam.RegHeight + RowIndex) * m_sCtrlParam.RegWidth + w] = pLineBuffUV[w];
	}
}

void CDither::UpdateState()
{
	uint32_t lsb = (m_sInternalParam.CurrentStateY ^ ~(m_sInternalParam.CurrentStateY >> 1)) & 0x01;
	m_sInternalParam.CurrentStateY = (m_sInternalParam.CurrentStateY >> 1) | (lsb << 21);

	lsb = (m_sInternalParam.CurrentStateU ^ ~(m_sInternalParam.CurrentStateU >> 1)) & 0x01;
	m_sInternalParam.CurrentStateU = (m_sInternalParam.CurrentStateU >> 1) | (lsb << 21);

	lsb = (m_sInternalParam.CurrentStateV ^ ~(m_sInternalParam.CurrentStateV >> 1)) & 0x01;
	m_sInternalParam.CurrentStateV = (m_sInternalParam.CurrentStateV >> 1) | (lsb << 21);
}

void CDither::GetPhaseFromRemainder(uint32_t Phase[4], uint32_t Remainder)
{
	switch (Remainder)
	{
	case 0:
		Phase[0] = 0;
		Phase[1] = 1;
		Phase[2] = 2;
		Phase[3] = 3;
		break;
	case 1:
		Phase[0] = 0;
		Phase[1] = 1;
		Phase[2] = 3;
		Phase[3] = 2;
		break;
	case 2:
		Phase[0] = 0;
		Phase[1] = 2;
		Phase[2] = 1;
		Phase[3] = 3;
		break;
	case 3:
		Phase[0] = 0;
		Phase[1] = 2;
		Phase[2] = 3;
		Phase[3] = 1;
		break;
	case 4:
		Phase[0] = 0;
		Phase[1] = 3;
		Phase[2] = 1;
		Phase[3] = 2;
		break;
	case 5:
		Phase[0] = 0;
		Phase[1] = 3;
		Phase[2] = 2;
		Phase[3] = 1;
		break;
	case 6:
		Phase[0] = 1;
		Phase[1] = 0;
		Phase[2] = 2;
		Phase[3] = 3;
		break;
	case 7:
		Phase[0] = 1;
		Phase[1] = 0;
		Phase[2] = 3;
		Phase[3] = 2;
		break;
	case 8:
		Phase[0] = 1;
		Phase[1] = 2;
		Phase[2] = 0;
		Phase[3] = 3;
		break;
	case 9:
		Phase[0] = 1;
		Phase[1] = 2;
		Phase[2] = 3;
		Phase[3] = 0;
		break;
	case 10:
		Phase[0] = 1;
		Phase[1] = 3;
		Phase[2] = 0;
		Phase[3] = 2;
		break;
	case 11:
		Phase[0] = 1;
		Phase[1] = 3;
		Phase[2] = 2;
		Phase[3] = 0;
		break;
	case 12:
		Phase[0] = 2;
		Phase[1] = 0;
		Phase[2] = 1;
		Phase[3] = 3;
		break;
	case 13:
		Phase[0] = 2;
		Phase[1] = 0;
		Phase[2] = 3;
		Phase[3] = 1;
		break;
	case 14:
		Phase[0] = 2;
		Phase[1] = 1;
		Phase[2] = 0;
		Phase[3] = 3;
		break;
	case 15:
		Phase[0] = 2;
		Phase[1] = 1;
		Phase[2] = 3;
		Phase[3] = 0;
		break;
	case 16:
		Phase[0] = 2;
		Phase[1] = 3;
		Phase[2] = 0;
		Phase[3] = 1;
		break;
	case 17:
		Phase[0] = 2;
		Phase[1] = 3;
		Phase[2] = 1;
		Phase[3] = 0;
		break;
	case 18:
		Phase[0] = 3;
		Phase[1] = 0;
		Phase[2] = 1;
		Phase[3] = 2;
		break;
	case 19:
		Phase[0] = 3;
		Phase[1] = 0;
		Phase[2] = 2;
		Phase[3] = 1;
		break;
	case 20:
		Phase[0] = 3;
		Phase[1] = 1;
		Phase[2] = 0;
		Phase[3] = 2;
		break;
	case 21:
		Phase[0] = 3;
		Phase[1] = 1;
		Phase[2] = 2;
		Phase[3] = 0;
		break;
	case 22:
		Phase[0] = 3;
		Phase[1] = 2;
		Phase[2] = 0;
		Phase[3] = 1;
		break;
	case 23:
		Phase[0] = 3;
		Phase[1] = 2;
		Phase[2] = 1;
		Phase[3] = 0;
		break;
	}
}

void CDither::ProcOne2x2Y(ushort* pLineBuff, ushort* Src, ushort* Dst, uint32_t RowIndex, uint32_t ColIndex, uint32_t CurrentRemainderY)
{
	uint32_t Err;
	uint32_t Pattern[4][4] = { {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 1, 1} };
	uint32_t P00, P01, P10, P11;
	uint32_t PhaseY[4];

	P00 = pLineBuff[ColIndex]; P01 = pLineBuff[ColIndex + 1];
	P10 = Src[RowIndex * m_sCtrlParam.RegWidth + ColIndex]; P11 = Src[RowIndex * m_sCtrlParam.RegWidth + ColIndex + 1];

	if (m_sCtrlParam.RegEnableOutput8Bit && m_sCtrlParam.RegEnableDither)
	{
		Err = (P00 & 0x03) + (P01 & 0x03) + (P10 & 0x03) + (P11 & 0x03);
		Err = (Err >> 2) + ((CurrentRemainderY & 0x03) < (Err & 0x03) ? 1 : 0);
		GetPhaseFromRemainder(PhaseY, CurrentRemainderY);
	}

	uint32_t D00 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P00 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[Err][PhaseY[0]] : ((P00 >> 1) & 0x01)), 0, 255) : P00;
	uint32_t D01 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P01 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[Err][PhaseY[1]] : ((P01 >> 1) & 0x01)), 0, 255) : P01;
	uint32_t D10 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P10 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[Err][PhaseY[2]] : ((P10 >> 1) & 0x01)), 0, 255) : P10;
	uint32_t D11 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P11 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[Err][PhaseY[3]] : ((P11 >> 1) & 0x01)), 0, 255) : P11;
	Dst[(RowIndex - 1) * m_sCtrlParam.RegWidth + ColIndex] = D00;
	Dst[(RowIndex - 1) * m_sCtrlParam.RegWidth + ColIndex + 1] = D01;
	pLineBuff[ColIndex] = D10;
	pLineBuff[ColIndex + 1] = D11;
}

void CDither::ProcOne2x2UV(ushort* pLineBuff, ushort* Src, ushort* Dst, uint32_t RowIndex, uint32_t ColIndex, uint32_t CurrentRemainderU, uint32_t CurrentRemainderV)
{
	uint32_t ErrU, ErrV;
	uint32_t Pattern[4][4] = { {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 1, 1} };
	uint32_t P00, P01, P10, P11;
	uint32_t PhaseU[4];
	uint32_t PhaseV[4];

	P00 = pLineBuff[ColIndex]; P01 = pLineBuff[ColIndex + 1];
	P10 = Src[RowIndex * m_sCtrlParam.RegWidth + ColIndex]; P11 = Src[RowIndex * m_sCtrlParam.RegWidth + ColIndex + 1];

	if (m_sCtrlParam.RegEnableOutput8Bit && m_sCtrlParam.RegEnableDither)
	{
		ErrU = (((P00 & 0x03) + (P10 & 0x03)) << 1);
		ErrU = (ErrU >> 2) + ((CurrentRemainderU & 0x03) < (ErrU & 0x03) ? 1 : 0);
		GetPhaseFromRemainder(PhaseU, CurrentRemainderU);

		ErrV = (((P01 & 0x03) + (P11 & 0x03)) << 1);
		ErrV = (ErrV >> 2) + ((CurrentRemainderV & 0x03) < (ErrV & 0x03) ? 1 : 0);
		GetPhaseFromRemainder(PhaseV, CurrentRemainderV);
	}

	uint32_t D00 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P00 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[ErrU][PhaseU[0]] : ((P00 >> 1) & 0x01)), 0, 255) : P00;
	uint32_t D01 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P01 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[ErrV][PhaseV[0]] : ((P01 >> 1) & 0x01)), 0, 255) : P01;
	uint32_t D10 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P10 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[ErrU][PhaseU[2]] : ((P10 >> 1) & 0x01)), 0, 255) : P10;
	uint32_t D11 = m_sCtrlParam.RegEnableOutput8Bit ? clip((P11 >> 2) + (m_sCtrlParam.RegEnableDither ? Pattern[ErrV][PhaseV[2]] : ((P11 >> 1) & 0x01)), 0, 255) : P11;
	if (m_sCtrlParam.RegEnableOutputFormat420)
	{
		if (m_sCtrlParam.RegEnableUVMean)
		{
			Dst[(RowIndex >> 1) * m_sCtrlParam.RegWidth + ColIndex] = ((D00 + D10 + 1) >> 1);
			Dst[(RowIndex >> 1) * m_sCtrlParam.RegWidth + ColIndex + 1] = ((D01 + D11 + 1) >> 1);
		}
		else
		{
			Dst[(RowIndex >> 1) * m_sCtrlParam.RegWidth + ColIndex] = D00;
			Dst[(RowIndex >> 1) * m_sCtrlParam.RegWidth + ColIndex + 1] = D01;
		}
	}
	else
	{
		Dst[(RowIndex - 1) * m_sCtrlParam.RegWidth + ColIndex] = D00;
		Dst[(RowIndex - 1) * m_sCtrlParam.RegWidth + ColIndex + 1] = D01;
		pLineBuff[ColIndex] = D10;
		pLineBuff[ColIndex + 1] = D11;
	}
}

void CDither::ProcessImage(ushort* Src, ushort* Dst)
{
	uint32_t CurrentRemainderY;
	uint32_t CurrentRemainderU;
	uint32_t CurrentRemainderV;
	uint32_t PhaseY[4];
	uint32_t PhaseU[4];
	uint32_t PhaseV[4];

	if (m_sCtrlParam.RegEnable)
	{
		m_sInternalParam.InputRowIndex = 0;
		while (m_sInternalParam.InputRowIndex < m_sCtrlParam.RegHeight + 1)
		{
			if ((m_sInternalParam.InputRowIndex & 0x01) == 0)
			{
				if (m_sInternalParam.InputRowIndex != 0)
					GetOneLineToOutput(Dst, m_sInternalParam.InputRowIndex - 1);
				if (m_sInternalParam.InputRowIndex < m_sCtrlParam.RegHeight)
					WriteOneLineToBuffer(Src, m_sInternalParam.InputRowIndex);
			}
			else
			{
				m_sInternalParam.InputColIndex = 0;
				while (m_sInternalParam.InputColIndex < m_sCtrlParam.RegWidth)
				{
					// 产生随机数
					if (m_sCtrlParam.RegEnableOutput8Bit && m_sCtrlParam.RegEnableDither)
					{
						if (m_sInternalParam.InputRowIndex == 1 && m_sInternalParam.InputColIndex == 0)
						{
							m_sInternalParam.CurrentStateY = m_sCtrlParam.RegInitStateY;
							m_sInternalParam.CurrentStateU = m_sCtrlParam.RegInitStateU;
							m_sInternalParam.CurrentStateV = m_sCtrlParam.RegInitStateV;
						}
						else
							UpdateState();
						CurrentRemainderY = m_sInternalParam.CurrentStateY % 24;
						if (CurrentRemainderY == m_sInternalParam.PrevRemainderY)
							CurrentRemainderY = (CurrentRemainderY + 5) >= 24 ? CurrentRemainderY - 19 : CurrentRemainderY + 5;
						m_sInternalParam.PrevRemainderY = CurrentRemainderY;

						CurrentRemainderU = m_sInternalParam.CurrentStateU % 24;
						if (CurrentRemainderU == m_sInternalParam.PrevRemainderU)
							CurrentRemainderU = (CurrentRemainderU + 5) >= 24 ? CurrentRemainderU - 19 : CurrentRemainderU + 5;
						m_sInternalParam.PrevRemainderU = CurrentRemainderU;

						CurrentRemainderV = m_sInternalParam.CurrentStateV % 24;
						if (CurrentRemainderV == m_sInternalParam.PrevRemainderV)
							CurrentRemainderV = (CurrentRemainderV + 5) >= 24 ? CurrentRemainderV - 19 : CurrentRemainderV + 5;
						m_sInternalParam.PrevRemainderV = CurrentRemainderV;
					}

					// 做处理
					ProcOne2x2Y(pLineBuffY, Src, Dst, m_sInternalParam.InputRowIndex, m_sInternalParam.InputColIndex, CurrentRemainderY);
					ProcOne2x2UV(pLineBuffUV, Src + m_sCtrlParam.RegHeight * m_sCtrlParam.RegWidth, Dst + m_sCtrlParam.RegHeight * m_sCtrlParam.RegWidth, m_sInternalParam.InputRowIndex, m_sInternalParam.InputColIndex, CurrentRemainderU, CurrentRemainderV);
					
					m_sInternalParam.InputColIndex += 2;
				}
			}
			m_sInternalParam.InputRowIndex += 1;
		}
	}
	else
		memcpy(Dst, Src, 2 * m_sCtrlParam.RegHeight * m_sCtrlParam.RegWidth * sizeof(ushort));
}