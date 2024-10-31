#include "InterfaceData.h"

using namespace std;

#define MIN_SENSOR_EXP		10   //10us
#define MAX_SENSOR_EXP		10000  //10ms
#define MIN_SENSOR_GAIN		1
#define MAX_SENSOR_GAIN		64
#define HISNUM				6
#define	AEC_COERD_TSHD		8

class AECAlgo
{
public:
	AECAlgo();
	~AECAlgo();
	int		init(InitParma* initParma);
	void	setParam(AECSetParam* setParam);
	void	getStats(int* ptr);
	void	getStatsFromRaw(AEC_stats* stats);
	void	getHist(int* ptr);
	void	algoCtrl();
	void	getAEout(AEC_algo_out* out);
	int		deinit();
	
	AEC_algo_input	algoInput;
	AEC_algo_out	algoOut;
	APEX			apexStill;
	ImgInfo			imgInfo;

private:
	//func
	void	setTuningData(AECTuningData* data);
	int		AutoExp(AEC_algo_input* input, AEC_algo_out* out);
	int		ATFExp(AEC_algo_input* input, AEC_algo_out* out);
	int		aecProcess();
	int		CWtableCal(int iSize, int bias, float sigma);
	double	targetLumaModify(AEC_stats* aecStats);
	double	avglumaCal(AEC_stats* aecStats);

	//global param
	float		RGB2Y[3] = { 0.299, 0.587, 0.114 };
	uint16_t	sumCnt = BLOCKNUM_H_MAX * BLOCKNUM_W_MAX;
		//algo
	double	TARGETBRI = 200;
	float		a[BLOCKNUM_H_MAX * BLOCKNUM_W_MAX];
	float		Weight[BLOCKNUM_H_MAX * BLOCKNUM_W_MAX] = { 0.0 };
	double		aHistoryY[HISNUM] = { -999999.0, -999999.0, -999999.0, -999999.0, -999999.0, -999999.0 };
		//luma table select
	int tableSelect = 2;
		//CW
	float sigma = 1;   // central circle raidus 3*sigma   �ڴ��3�Ҿ���֮������ض����Կ�����������
	int centerR = BLOCKNUM_H_MAX < BLOCKNUM_W_MAX ? BLOCKNUM_H_MAX : BLOCKNUM_W_MAX;   //Ĭ�ϲ�������ƫ��
	int bias = BLOCKNUM_H_MAX < BLOCKNUM_W_MAX ? (BLOCKNUM_W_MAX - BLOCKNUM_H_MAX) / 2 : (BLOCKNUM_H_MAX - BLOCKNUM_W_MAX) / 2;
		//hdr case judge
	bool		hdrFlag = false;
	double		satNumberPercent = 0.9;
	int			satBlockCnt = 0;
	float		satRatioTolerance = 0.1;  //target modify,ȫͼ����block������ռ��ʱ��Ϊ��HDR����
	float		DRRatioTolerance = 6; //  ������ƽ��ֵ/������ƽ��ֵ > ��ֵʱ��Ϊ��HDR����
	float		brightAreaPrec = 1.0 / 36;   // ������ռ�Ⱦ���ֵ  ��ȫ��ֱ��ͼʱ��ֵ�ɻ�Ϊ�����ٽ�ֵ����250
	float		darkAreaPrec = 1.0 / 36;	// ������ռ�Ⱦ���ֵ		��ȫ��ֱ��ͼʱ��ֵ�ɻ�Ϊ�����ٽ�ֵ����20
};
