#define GXR_SOFTISP_Build
#include "gxr_softisp.hpp"
#include "raw2bgr.hpp"
#include <cerrno>

#define VERSION_VERSION "GXR_SOFTISP_3.5.8.5_video"

using namespace gxr;

static char const* RawPattName[4] =
{
	"RG/GB",
	"BG/GR",
	"GR/BG",
	"GB/RG",
};

static void gen_gammatable(const ushort* xtable, const ushort* ytable, uchar* gtab)
{
	int const len = LINEARSAT + 1;
	ushort xmax = xtable[256];
	float xratio = static_cast<float>(LINEARSAT) / xmax;

	ushort ymax = ytable[0];
	for (int i = 0; i < 257; i++)
	{
		ushort y = ytable[i];
		if (ymax < y)
			ymax = y;
	}
	float yratio = static_cast<float>(GAMMASAT) / ymax;

	vector<float> normXTable(len);
	vector<float> normYTable(len);
	for (int i = 0; i < 257; i++)
	{
		normXTable[i] = xtable[i] * xratio;
		normYTable[i] = ytable[i] * yratio;
	}

	for (int ind0 = 0; ind0 <= GAMMASAT; ind0++)
	{
		int ind1 = ind0 + 1;
		float x0 = normXTable[ind0], x1 = normXTable[ind1], ratio = 1.0 / (x1 - x0);
		int i0 = ceil(x0), i1 = floor(x1);
		for (int i = i0; i <= i1; i++)
		{
			float w1 = (i - x0) * ratio;
			gtab[i] = gxr_clamp<int>(normYTable[ind0] + (normYTable[ind1] - normYTable[ind0]) * w1 + 0.5f, 0, GAMMASAT);
		}
	}
}

int GXR_Softisp::create(SizeParam& p)
{
	Size szorg(p.org_width, p.org_height);
	Size szroi(p.roi_width, p.roi_height);
	Size szlsc(p.lsc_width, p.lsc_height);
	Size szblk(p.block_width, p.block_height);
	RAW2BGR* raw2 = reinterpret_cast<RAW2BGR*>(impl);
	if (raw2 && raw2->szorg == szorg && raw2->szroi == szroi && raw2->szlsc == szlsc && raw2->szblk == szblk)
		return 0;

	release();
	raw2 = new(std::nothrow) RAW2BGR(szorg, szroi, szlsc, szblk);
	if (raw2)
	{
		handle = new Config();
		raw2->awb = new AWB(szlsc.height, szlsc.width);
		impl = raw2;

		return 0;
	}
	errno = ENOMEM;
	return ENOMEM;
}

int GXR_Softisp::config(Config& c)
{
	Config& cfg = *reinterpret_cast<Config*>(handle);
	memcpy(&cfg, &c, sizeof(c));
	RAW2BGR* raw2 = reinterpret_cast<RAW2BGR*>(impl);
	raw2->verbose = cfg.verbose;
	raw2->awb->verbose = cfg.verbose & VerbParam;
	if (cfg.nthread < 0)
		cfg.nthread = getNumberOfCPUs() - 1;
	setNumThreads(cfg.nthread);
	raw2->wben = cfg.wben;
	raw2->expen = cfg.expen;
	raw2->aesat = cfg.aesat;
	CV_Assert(cfg.gmtablex);
	CV_Assert(cfg.gmtabley);
	gen_gammatable(cfg.gmtablex, cfg.gmtabley, raw2->gammatable.data);

	if (cfg.verbose & VerbParam)
	{
		fprintf(stdout, "Softisp configuration (%dx%d, %dx%d, %dx%d, %dx%d)\n"
			"\tverbose %d, nthread %d, wben %d, expen %d, aesat %d\n"
			"\tynoise1 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise2 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise3 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise4 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise5 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise6 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise7 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tynoise8 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise1 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise2 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise3 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise4 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise5 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise6 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise7 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tcnoise8 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp1 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp2 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp3 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp4 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp5 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp6 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp7 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tsharp8 %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\tbdlarge %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
			raw2->szorg.width, raw2->szorg.height, raw2->szroi.width, raw2->szroi.height, raw2->szlsc.width, raw2->szlsc.height, raw2->szblk.width, raw2->szblk.height,
			cfg.verbose, cfg.nthread, cfg.wben, cfg.expen, cfg.aesat,
			cfg.ynoise[0], cfg.ynoise[1], cfg.ynoise[2], cfg.ynoise[3], cfg.ynoise[4],
			cfg.ynoise[5 + 0], cfg.ynoise[5 + 1], cfg.ynoise[5 + 2], cfg.ynoise[5 + 3], cfg.ynoise[5 + 4],
			cfg.ynoise[10 + 0], cfg.ynoise[10 + 1], cfg.ynoise[10 + 2], cfg.ynoise[10 + 3], cfg.ynoise[10 + 4],
			cfg.ynoise[15 + 0], cfg.ynoise[15 + 1], cfg.ynoise[15 + 2], cfg.ynoise[15 + 3], cfg.ynoise[15 + 4],
			cfg.ynoise[20 + 0], cfg.ynoise[20 + 1], cfg.ynoise[20 + 2], cfg.ynoise[20 + 3], cfg.ynoise[20 + 4],
			cfg.ynoise[25 + 0], cfg.ynoise[25 + 1], cfg.ynoise[25 + 2], cfg.ynoise[25 + 3], cfg.ynoise[25 + 4],
			cfg.ynoise[30 + 0], cfg.ynoise[30 + 1], cfg.ynoise[30 + 2], cfg.ynoise[30 + 3], cfg.ynoise[30 + 4],
			cfg.ynoise[35 + 0], cfg.ynoise[35 + 1], cfg.ynoise[35 + 2], cfg.ynoise[35 + 3], cfg.ynoise[35 + 4],
			cfg.cnoise[0], cfg.cnoise[1], cfg.cnoise[2], cfg.cnoise[3], cfg.cnoise[4],
			cfg.cnoise[5 + 0], cfg.cnoise[5 + 1], cfg.cnoise[5 + 2], cfg.cnoise[5 + 3], cfg.cnoise[5 + 4],
			cfg.cnoise[10 + 0], cfg.cnoise[10 + 1], cfg.cnoise[10 + 2], cfg.cnoise[10 + 3], cfg.cnoise[10 + 4],
			cfg.cnoise[15 + 0], cfg.cnoise[15 + 1], cfg.cnoise[15 + 2], cfg.cnoise[15 + 3], cfg.cnoise[15 + 4],
			cfg.cnoise[20 + 0], cfg.cnoise[20 + 1], cfg.cnoise[20 + 2], cfg.cnoise[20 + 3], cfg.cnoise[20 + 4],
			cfg.cnoise[25 + 0], cfg.cnoise[25 + 1], cfg.cnoise[25 + 2], cfg.cnoise[25 + 3], cfg.cnoise[25 + 4],
			cfg.cnoise[30 + 0], cfg.cnoise[30 + 1], cfg.cnoise[30 + 2], cfg.cnoise[30 + 3], cfg.cnoise[30 + 4],
			cfg.cnoise[35 + 0], cfg.cnoise[35 + 1], cfg.cnoise[35 + 2], cfg.cnoise[35 + 3], cfg.cnoise[35 + 4],
			cfg.sharp[0], cfg.sharp[1], cfg.sharp[2], cfg.sharp[3], cfg.sharp[4],
			cfg.sharp[5 + 0], cfg.sharp[5 + 1], cfg.sharp[5 + 2], cfg.sharp[5 + 3], cfg.sharp[5 + 4],
			cfg.sharp[10 + 0], cfg.sharp[10 + 1], cfg.sharp[10 + 2], cfg.sharp[10 + 3], cfg.sharp[10 + 4],
			cfg.sharp[15 + 0], cfg.sharp[15 + 1], cfg.sharp[15 + 2], cfg.sharp[15 + 3], cfg.sharp[15 + 4],
			cfg.sharp[20 + 0], cfg.sharp[20 + 1], cfg.sharp[20 + 2], cfg.sharp[20 + 3], cfg.sharp[20 + 4],
			cfg.sharp[25 + 0], cfg.sharp[25 + 1], cfg.sharp[25 + 2], cfg.sharp[25 + 3], cfg.sharp[25 + 4],
			cfg.sharp[30 + 0], cfg.sharp[30 + 1], cfg.sharp[30 + 2], cfg.sharp[30 + 3], cfg.sharp[30 + 4],
			cfg.sharp[35 + 0], cfg.sharp[35 + 1], cfg.sharp[35 + 2], cfg.sharp[35 + 3], cfg.sharp[35 + 4],
			cfg.bdlarge[0], cfg.bdlarge[1], cfg.bdlarge[2], cfg.bdlarge[3],
			cfg.bdlarge[4], cfg.bdlarge[5], cfg.bdlarge[6], cfg.bdlarge[7]);
	}
	return 0;
}

int GXR_Softisp::prestats(Input& in)
{
	RAW2BGR* raw2 = reinterpret_cast<RAW2BGR*>(impl);
	Config& cfg = *reinterpret_cast<Config*>(handle);

#define Softisp_Args(var) raw2->var = in.var
	Softisp_Args(rpat); Softisp_Args(depth);
	Softisp_Args(blacklevel[0]); Softisp_Args(blacklevel[1]); Softisp_Args(blacklevel[2]); Softisp_Args(blacklevel[3]);
	Softisp_Args(wbgain[0]); Softisp_Args(wbgain[1]); Softisp_Args(wbgain[2]); Softisp_Args(wbgain[3]);
	Softisp_Args(sensorgain);  Softisp_Args(ispgain); Softisp_Args(adrcgain); Softisp_Args(shutter); Softisp_Args(luxindex);
#undef Softisp_Args
	CV_Assert(raw2->sensorgain >= 1.f && raw2->ispgain >= 1.f);
	memcpy(raw2->ccm, in.ccm, sizeof(in.ccm));
	raw2->roi.x = in.left;
	raw2->roi.y = in.top;

	float rawiso = log2f(gxr_max(in.sensorgain * in.ispgain, 1.f));
	int s0 = rawiso, s1 = gxr_min(s0 + 1, 7);
	float w = rawiso - s0;
	float* ynoise0 = &cfg.ynoise[s0 * 5], * ynoise1 = &cfg.ynoise[s1 * 5];
	float* cnoise0 = &cfg.cnoise[s0 * 5], * cnoise1 = &cfg.cnoise[s1 * 5];
	float* sharp0 = &cfg.sharp[s0 * 5], * sharp1 = &cfg.sharp[s1 * 5];
	for (int i = 0; i < 5; i++)
	{
		raw2->ynoise[i] = ynoise0[i] + w * (ynoise1[i] - ynoise0[i]);
		raw2->cnoise[i] = cnoise0[i] + w * (cnoise1[i] - cnoise0[i]);
		raw2->sharp[i] = sharp0[i] + w * (sharp1[i] - sharp0[i]);
	}
	raw2->bdlarge = cfg.bdlarge[s0] + w * (cfg.bdlarge[s1] - cfg.bdlarge[s0]);

	size_t lscsz = raw2->szlsc.height * raw2->szlsc.width * sizeof(float);
	if (raw2->wben || raw2->start)
	{
		for (int cn = 0; cn < 4; ++cn)
		{
			// lsctable: R, Gr, Gb, B
			memcpy(raw2->lsctable[cn].data, in.lsctable[cn], lscsz);
		}
	}

	if (raw2->verbose & VerbParam)
	{
		fprintf(stdout, "\t\tynoise %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\t\tcnoise %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\t\tsharp %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\t\tbdlarge %.3f, RawType %s, depth %d, top %d, left %d\n"
			"\t\tblacklevel %d, %d, %d, %d\n"
			"\t\twbgain %.3f, %.3f, %.3f, %.3f\n"
			"\t\tccm %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n"
			"\t\tsensorgain %.3f, ispgain %.3f, adrcgain %.3f, shutter %.3f, luxindex %.3f\n"
			"\t\tlsc %.3f, %.3f, %.3f, %.3f\n", 
			raw2->ynoise[0], raw2->ynoise[1], raw2->ynoise[2], raw2->ynoise[3], raw2->ynoise[4],
			raw2->cnoise[0], raw2->cnoise[1], raw2->cnoise[2], raw2->cnoise[3], raw2->cnoise[4],
			raw2->sharp[0], raw2->sharp[1], raw2->sharp[2], raw2->sharp[3], raw2->sharp[4],
			raw2->bdlarge, RawPattName[raw2->rpat], raw2->depth, raw2->roi.y, raw2->roi.x, 
			raw2->blacklevel[0], raw2->blacklevel[1], raw2->blacklevel[2], raw2->blacklevel[3],
			raw2->wbgain[0], raw2->wbgain[1], raw2->wbgain[2], raw2->wbgain[3],
			raw2->ccm[0], raw2->ccm[1], raw2->ccm[2], raw2->ccm[3], raw2->ccm[4], raw2->ccm[5], raw2->ccm[6], raw2->ccm[7], raw2->ccm[8],
			raw2->sensorgain, raw2->ispgain, raw2->adrcgain, raw2->shutter, raw2->luxindex,
			*raw2->lsctable[0].ptr<float>(), *raw2->lsctable[1].ptr<float>(), *raw2->lsctable[2].ptr<float>(), *raw2->lsctable[3].ptr<float>());
	}

	raw2->raw = Mat(raw2->srows, raw2->scols, CV_16SC1, in.data);
	raw2->prestats();
	memcpy(in.stats.awb_blockstats, raw2->awb_blockstats.data, raw2->awb_blockstats.rows * raw2->awb_blockstats.step[0]);
	memcpy(in.stats.ae_blockstats, raw2->ae_blockstats.data, raw2->ae_blockstats.rows * raw2->ae_blockstats.step[0]);
	memcpy(in.stats.histgram, raw2->histgram.data, raw2->histgram.rows * raw2->histgram.step[0]);
	return 0;
}

int GXR_Softisp::poststats(Output& out)
{
	RAW2BGR* raw2 = reinterpret_cast<RAW2BGR*>(impl);
	raw2->dst = Mat(raw2->brows, raw2->bcols, CV_8UC3, out.data);
	raw2->poststats();
	return 0;
}

int GXR_Softisp::release()
{
	if (impl)
	{
		RAW2BGR* raw2 = reinterpret_cast<RAW2BGR*>(impl);
		if (raw2->awb)
			delete raw2->awb;
		raw2->awb = 0;
		delete raw2;
	}
	impl = 0;
	if (handle)
		delete reinterpret_cast<Config*>(handle);
	handle = 0;
	return 0;
}

void GXR_Softisp::get_blocknum(int& blocknum_x, int& blocknum_y)
{
	RAW2BGR* raw2 = reinterpret_cast<RAW2BGR*>(impl);
	blocknum_x = raw2->szsta.width;
	blocknum_y = raw2->szsta.height;
}

GXR_SOFTISP_VERSION GXR_Softisp::get_version()
{
	GXR_SOFTISP_VERSION v;
	v.Version = VERSION_VERSION;
	return v;
}