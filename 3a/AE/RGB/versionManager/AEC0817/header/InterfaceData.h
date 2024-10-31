#include <cstdint>

//block number
#define BLOCKNUM_W_MAX  36		//max
#define BLOCKNUM_H_MAX  27		//max

typedef struct AEC_stats
{
	uint32_t		sum[4][BLOCKNUM_W_MAX * BLOCKNUM_H_MAX];
	uint16_t		non_satNum[4][BLOCKNUM_W_MAX * BLOCKNUM_H_MAX];
	int				histogram[256];
};

typedef struct FaceInfo {
	//todo
	uint16_t		x;
	uint16_t		y;
	uint16_t		dx;
	uint16_t		dy;
};

typedef struct FlickInfo {
	bool			isFlickDetection;
	uint8_t			flickFrequence;
};

typedef struct ImgInfo {
	// raw size info
	int				imgHeight;
	int				imgWeight;
	// block info  size & num
	int				blockHeight;
	int				blockWeight;
	int				horBlockNum;
	int				verBlockNum;
};

typedef struct SensorInfo {
	uint32_t		sensorExposureTime;  //us   7000
	float			sensorExposureGain;
	int				frameRate;   //30/60/90
};

typedef struct APEX {
	float			tv;
	float			av;
	float			sv;
	float			lv;
};

typedef struct AECTuningData {
	//cw parameters
	int			sigma;  // central weight table cal 0.33 ~ 3 
	// target luma modify
	float		satNumberPercent;  //�����ʣ������ñ�����block������Ϊ�ǹ�������
	int			tableSelect;    //target table select
	float		satRatioTolerance;  //target modify,ȫͼ����block������ռ��ʱ��Ϊ��HDR����
	float		DRRatioTolerance; //  ������ƽ��ֵ/������ƽ��ֵ > ��ֵʱ��Ϊ��HDR����
	float		brightAreaPerc;   // ������ռ�Ⱦ���ֵ  ��ȫ��ֱ��ͼʱ��ֵ�ɻ�Ϊ�����ٽ�ֵ����250
	float		darkAreaPerc;	// ������ռ�Ⱦ���ֵ		��ȫ��ֱ��ͼʱ��ֵ�ɻ�Ϊ�����ٽ�ֵ����20
};

enum CurExpoMode
{
	AUTO_EXP,
	MANU_EXP,
	AntiFlickModeAE
};

typedef struct InitParma {
	double			F_number;
	ImgInfo			img;
};

enum  setParamType
{
	EXPMODE,
	FACEINFO,
	IMGINFO,
	FLICKINFO,
	SENSORINFO,
	TUNINGDATA
};

union setParamUnion
{
	CurExpoMode		expMode;
	FaceInfo		faceInfo;
	ImgInfo			imgInfo;
	FlickInfo       flickInfo;
	SensorInfo		sensorInfo;
	AECTuningData	tuningData;
};

typedef struct AECSetParam
{
	setParamType	type;
	setParamUnion*	param;
	int				paramSize;
};

typedef struct AEC_algo_input {
	uint16_t		frameId;
	CurExpoMode		sCurExpoMode;
	AEC_stats		statsAE;
	FaceInfo		faceInfo;
	FlickInfo       flickInfo;
	SensorInfo		sensorInfo;
};

typedef struct AEC_algo_out {
	uint32_t		exposureTime;  //us
	float			exposureGain;
};

