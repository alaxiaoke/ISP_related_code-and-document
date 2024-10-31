#pragma once
#include <array>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/core/hal/intrin.hpp"
#include "opencv2/imgproc.hpp"
#include "utility.hpp"

using namespace cv;

namespace gxr
{
	class RawScaler
	{
		class ColPass : public ParallelLoopBody
		{
		public:
			RawScaler* rs;
			Mat& src, & tmp;
			int srows, scols;
			int drows, dcols;
			short* HorCoef, * HorPhase;
			unsigned pat;
			Mat coefgreen;
			short* CoefGreen;

		public:
			ColPass(RawScaler* rs_, Mat& src_, Mat& tmp_);
			void operator ()(Range const& range) const override;
		};

		class RowPass : public ParallelLoopBody
		{
		public:
			RawScaler* rs;
			Mat& dst, & tmp;
			int srows, scols;
			int drows, dcols;
			short* VerCoef, * VerPhase;
			unsigned pat;
			Mat coefgreen;
			short* CoefGreen;

		public:
			RowPass(RawScaler* rs_, Mat& dst_, Mat& tmp_);
			void operator ()(Range const& range) const override;
		};

	public:
		Mat& src, tmp, coef, phase;
		int srows, scols;
		int drows, dcols;
		int rpat;

	public:
		RawScaler(Mat& src_, Size dsize, Mat& buffer_, int rpat_);
		void run();
	};
}