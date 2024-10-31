#include "gravityxr_grouping.hpp"
#include <vector>
#include <string>
#include <cassert>

using std::vector;
using std::string;

typedef unsigned short ushort;
typedef unsigned char uchar;

//  ‰»Î ‰≥ˆ, YUV 422 semi-planar
static ushort* input, * output;

static int srows, scols;
static int drows, dcols;

static string name, outname;

int main(int argc, char** argv)
{
	Grouping* grouping = new Grouping();

	if (argc < 2)
	{
		printf("no file given, will exit\n");
		return -1;
	}
	else
		name = argv[1];
	FILE* fin = fopen(name.c_str(), "rb");
	if (!fin)
	{
		printf("no file named %s\n", name.c_str());
		return -1;
	}

	string cfgname = name;
	size_t dot = cfgname.find_last_of('.');
	if (dot > 0 && dot != string::npos)
		cfgname.erase(cfgname.begin() + dot, cfgname.end());
	char const* cfgdefault = (cfgname + ".ini").c_str();
	printf("cfg: %s\n", cfgdefault);
	char const* cfg = (argc > 2) ? argv[2] : cfgdefault;
	if (argc > 2 && !strcmp(argv[2], "/?"))
	{
		fprintf(stdout, "%s config file\n", argv[0]);
		return 1;
	}
	FILE* fconfig = fopen(cfg, "r");
	if (!fconfig)
	{
		printf("no config file named %s\n", fconfig);
		return -1;
	}
	grouping->parse_config(fconfig);
	grouping->updata_config();
	srows = grouping->params.Reg_SrcHeight;
	scols = grouping->params.Reg_SrcWidth;
	drows = grouping->params.Reg_DstHeight;
	dcols = grouping->params.Reg_DstWidth;
	fclose(fconfig);

	input = static_cast<ushort*>(malloc(sizeof(ushort) * srows * scols * 2));
	fread(input, sizeof(ushort), srows * scols * 2, fin);
	fclose(fin);
	printf("read file %s, size %dx%d\n", name.c_str(), scols, srows);

	output = static_cast<ushort*>(malloc(sizeof(ushort) * drows * dcols * 2));

	grouping->init();
	grouping->process(input, output);

	if (argc > 3)
		outname = argv[3];
	else
	{
		outname = name;
		size_t dot = outname.find_last_of('.');
		if (dot > 0 && dot != string::npos)
			outname.erase(outname.begin() + dot, outname.end());
		outname = outname + "_dst.yuv422";
	}
	FILE* fout = fopen(outname.c_str(), "wb");
	fwrite(output, sizeof(ushort), drows * dcols * 2, fout);
	fclose(fout);
	printf("write file %s\n", outname.c_str());

	free(input);
	free(output);

	grouping->release();
	return 0;
}



























