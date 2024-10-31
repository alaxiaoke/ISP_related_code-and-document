#include "gamma.hpp"

void Gamma::ProcessImage(ushort* Src, ushort* Dst)
{
	uint32_t Row = 0, Col, X;
	while (Row < m_sCtrlParam.RegHeight)
	{
		ushort* pSrcLine = Src + Row * m_sCtrlParam.RegWidth;
		ushort* pDstLine = Dst + Row * m_sCtrlParam.RegWidth;
		Col = 0;
		while (Col < m_sCtrlParam.RegWidth)
		{
			if (m_sCtrlParam.RegEnable)
			{
				uint32_t LutIndex, Coord, StepBit;
				X = pSrcLine[Col];
				if (X < m_sCtrlParam.RegBoundary[0])
				{
					StepBit = m_sCtrlParam.RegStepBit[0];
					LutIndex = (X >> StepBit) + m_sCtrlParam.RegIndex[0];
					Coord = (X >> StepBit) << StepBit;
				}
				else if (X < m_sCtrlParam.RegBoundary[1])
				{
					StepBit = m_sCtrlParam.RegStepBit[1];
					LutIndex = ((X - m_sCtrlParam.RegBoundary[0]) >> StepBit) + m_sCtrlParam.RegIndex[1];
					Coord = (((X - m_sCtrlParam.RegBoundary[0]) >> StepBit) << StepBit) + m_sCtrlParam.RegBoundary[0];
				}
				else if (X < m_sCtrlParam.RegBoundary[2])
				{
					StepBit = m_sCtrlParam.RegStepBit[2];
					LutIndex = ((X - m_sCtrlParam.RegBoundary[1]) >> StepBit) + m_sCtrlParam.RegIndex[2];
					Coord = (((X - m_sCtrlParam.RegBoundary[1]) >> StepBit) << StepBit) + m_sCtrlParam.RegBoundary[1];
				}
				else if (X < m_sCtrlParam.RegBoundary[3])
				{
					StepBit = m_sCtrlParam.RegStepBit[3];
					LutIndex = ((X - m_sCtrlParam.RegBoundary[2]) >> StepBit) + m_sCtrlParam.RegIndex[3];
					Coord = (((X - m_sCtrlParam.RegBoundary[2]) >> StepBit) << StepBit) + m_sCtrlParam.RegBoundary[2];
				}
				else if (X < m_sCtrlParam.RegBoundary[4])
				{
					StepBit = m_sCtrlParam.RegStepBit[4];
					LutIndex = ((X - m_sCtrlParam.RegBoundary[3]) >> StepBit) + m_sCtrlParam.RegIndex[4];
					Coord = (((X - m_sCtrlParam.RegBoundary[3]) >> StepBit) << StepBit) + m_sCtrlParam.RegBoundary[3];
				}
				else
				{
					StepBit = m_sCtrlParam.RegStepBit[5];
					LutIndex = ((X - m_sCtrlParam.RegBoundary[4]) >> StepBit) + m_sCtrlParam.RegIndex[5];
					Coord = (((X - m_sCtrlParam.RegBoundary[4]) >> StepBit) << StepBit) + m_sCtrlParam.RegBoundary[4];
				}
				if (StepBit == 0)
					pDstLine[Col] = m_sCtrlParam.RegLut[LutIndex];
				else
					pDstLine[Col] = (((X - Coord) * ((LutIndex == 63 ? m_sCtrlParam.RegLutFinal : m_sCtrlParam.RegLut[LutIndex + 1]) - m_sCtrlParam.RegLut[LutIndex]) + (1 << (StepBit - 1))) >> StepBit) + m_sCtrlParam.RegLut[LutIndex];
			}
			else
				pDstLine[Col] = pSrcLine[Col] >> 4;
			Col += 1;
		}
		Row += 1;
	}
}

