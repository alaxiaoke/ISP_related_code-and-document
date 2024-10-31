#pragma once
#include <array>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/core/hal/intrin.hpp"
#include "utility.hpp"

using namespace cv;

namespace gxr
{
	class RawGroup
	{
		class ColPass : public ParallelLoopBody
		{
		public:
			RawGroup* rg;
			Mat& src, & tmp;
			int scols, dcols;
			int sw0, sw1;
			int dw0, dw1;
			short* GroupKernel;

		public:
			ColPass(RawGroup* rg_, Mat& src_, Mat& tmp_);			
			void operator ()(Range const& range) const override;
		};

		class RowPass : public ParallelLoopBody
		{
		public:
			RawGroup* rg;
			Mat& dst, & tmp;
			int srows, scols;
			int drows, dcols;
			int sh0, sh1;
			int dh0, dh1;
			short* GroupKernel;

		public:
			RowPass(RawGroup* rg_, Mat& dst_, Mat& tmp_);
			void operator ()(Range const& range) const override;
		};

	public:
		Mat& src, tmp;
		Rect& roi;
		int srows, scols;
		int drows, dcols;
		int sw0, sh0, dw0, dh0;
		int sw1, sh1, dw1, dh1;
		short GroupKernel[16];

	public:
		RawGroup(Mat& src_, Rect& roi_, Mat& buffer_);
		void run();
	};
}







