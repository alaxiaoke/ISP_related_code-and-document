#pragma once
#include <array>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/core/hal/intrin.hpp"
#include "string"
#include "utility.hpp"
#include "rawproc.hpp"
#include "ocl.hpp"

#define GXR_CPU_REFERENCE_DEBUG 0
#if GXR_CPU_REFERENCE_DEBUG
#include "../samples/clprogram.hpp"
#endif

#define GXR_CV_DEBUG 1
#if GXR_CV_DEBUG
#include "opencv2/highgui.hpp"
#endif

#define AltekLSC 0
#define RefineCbCr 1

using std::array;
using std::vector;

enum Constants
{
	VerbParam = 0x1,
	VerbTimer = 0x2,
	VerbImage = 0x4,
	LINEARBIT = 12,
	LINEARSAT = (1 << LINEARBIT) - 1,
	LSCBIT = 10,
	LSCVAL = 1 << LSCBIT,
	GAMMABIT = 8,
	GAMMASAT = (1 << GAMMABIT) - 1,
};

namespace gxr
{
	class AWB
	{
	public:
		int verbose;
		void* handle;
		Mat blksum;

		AWB(int lsc_rows = 13, int lsc_cols = 17);
		~AWB();
		void feedstats(Mat& blksum_);
		void estimate();
		void getwb(float* rgain, float* bgain);
		void getccm(float* ccm);
		void getlsc(array<Mat, 4>& lsctable);
	};

	class AE
	{
	public:
		int verbose;
		void* handle;
		void* param[8];
		Mat blkstats;
		Mat histgram;

		AE(Size szblk, Size szsta);
		~AE();
		void feedstats(Mat& blkstats_, Mat& histgram_, unsigned shutter, float gain, int fps);
		void estimate();
		void getexp(unsigned* shutter, float* gain);
	};

	class RAW2BGR
	{
	public:

		RAW2BGR();
		RAW2BGR(Size const& szorg, Size const& szroi, Size const& szlsc, Size const& szsta);
		~RAW2BGR();
		void run_all();
		void prestats();
		void poststats();

		int64_t ticktotal, framenum;

		char verbose, start;
		int rpat, depth, aesat;
		int blacklevel[4];
		float wbgain[4];
		float ynoise[5], cnoise[5], sharp[5], bdlarge;
		float sensorgain, ispgain, adrcgain, luxindex;
		unsigned shutter;

		int srows, scols;
		int brows, bcols;
		int hrows, hcols;
		Rect roi;
		Size szorg, szroi, szlsc, szblk, szsta;

		float ccm[9];
		array<Mat, 4> lsctable;
		Mat gammatable;
		Mat raworg, raw, bgr, dst, pre;
		Mat luma, chroma;

		char wben, expen;
		AWB* awb;
		AE* ae;
		OCL* ocl;
		Mat awb_blockstats, ae_blockstats, histgram;

	private:

		char old;
		Vec4f wbcfa;
		Vec4i cfa, darkcfa;
		Mat lscflat;

	protected:

		void timer_step(char const* msg);
		void prepare();
		void rawgroup();
		int tickseq;
		int64_t tickprev, ticksum, tickcheck;
		int ntick;

		class TNR
		{
		private:
			int inline color(unsigned h, unsigned w, int cfa)
			{
				return (4 - ((((h & 1) << 1) + (w & 1)) ^ cfa)) >> 1;
			}

		public:
			RAW2BGR* raw2;
			int brows, bcols;
			int hrows, hcols;
			int firstframe;
			Mat& cur, & pre;
			OCL* ocl;

			Mat Filter_Th_R;
			Mat Filter_Th_G;
			Mat Filter_Th_Power;
			Mat MAE_Preserve_LUT;
			int Blending_Weight;
			int CFA_Order;
			Mat Var_R, Var_G;
			Mat Discount_LUT;
			Mat Anti_Ghost_LUT;
			int TemporalFrameWeight;
			int UseCalibrateVarOnly;

			cl_context& context;
			cl_command_queue& queue;
			cl_kernel& kernel_spafilter;
			cl_kernel& kernel_tnr_pre;
			cl_kernel& kernel_tnr_post;

		public:
			TNR(RAW2BGR* r)
				: raw2(r), cur(r->raw), pre(r->pre), firstframe(r->start), 
				context(raw2->ocl->context), queue(raw2->ocl->queue), 
				kernel_spafilter(raw2->ocl->kernel_spafilter), kernel_tnr_pre(raw2->ocl->kernel_tnr_pre), kernel_tnr_post(raw2->ocl->kernel_tnr_post)
			{
				ocl = raw2->ocl;
				brows = cur.rows;
				bcols = cur.cols;
				hrows = brows >> 1;
				hcols = bcols >> 1;

				Filter_Th_R.create(1, 1 << 10, CV_8UC1);
				Filter_Th_G.create(1, 1 << 10, CV_8UC1);
				Filter_Th_Power.create(1, 4, CV_8UC1);
				MAE_Preserve_LUT.create(1, 64, CV_8UC1);
				Blending_Weight = 64;
				switch (raw2->rpat)
				{
				case Raw_RGGB:
					CFA_Order = 0;
					break;
				case Raw_BGGR:
					CFA_Order = 3;
					break;
				case Raw_GRBG:
					CFA_Order = 1;
					break;
				case Raw_GBRG:
					CFA_Order = 2;
					break;
				}
				Var_R.create(1, 1 << 9, CV_16UC1);
				Var_G.create(1, 1 << 9, CV_16UC1);
				Discount_LUT.create(1, 16, CV_8UC1);
				Anti_Ghost_LUT.create(1, 27, CV_8UC1);



				uchar* Th_R = Filter_Th_R.data;
				uchar* Th_G = Filter_Th_G.data;
				for (int i = 0; i < (1 << 10); i++)
				{
					Th_R[i] = 4 * sqrt(6 * (i / 1023.f) * (i / 1023.f));
					Th_G[i] = Th_R[i];
				}

				uchar* Th_Power = Filter_Th_Power.data;
				Th_Power[0] = 2;
				Th_Power[1] = 3;
				Th_Power[2] = 4;
				Th_Power[3] = 5;

				uchar* MAE_Preserve = MAE_Preserve_LUT.data;
				for (int i = 0; i < 64; i++)
				{
					MAE_Preserve[i] = 127 + 1.f * (i / 64.f) * (i / 64.f) * 127;
				}

				ushort* VR = Var_R.ptr<ushort>();
				ushort* VG = Var_G.ptr<ushort>();
				for (int i = 0; i < (1 << 9); i++)
				{
					VR[i] = 16 * (2 + 2 * i / 511.f);
					VG[i] = VR[i];
				}

				uchar* Discount = Discount_LUT.data;
				Discount[0] = 45;
				Discount[1] = 47;
				Discount[2] = 48;
				Discount[3] = 50;
				Discount[4] = 64;
				Discount[5] = 15;
				Discount[6] = 5;
				Discount[7] = 10;
				Discount[8] = 15;
				Discount[9] = 58;
				Discount[10] = 58;
				Discount[11] = 59;
				Discount[12] = 59;
				Discount[13] = 60;
				Discount[14] = 61;
				Discount[15] = 64;

				uchar* Anti_Ghost = Anti_Ghost_LUT.data;
				Anti_Ghost[0] = 127;
				Anti_Ghost[1] = 127;
				Anti_Ghost[2] = 126;
				Anti_Ghost[3] = 125;
				Anti_Ghost[4] = 120;
				Anti_Ghost[5] = 119;
				Anti_Ghost[6] = 110;
				Anti_Ghost[7] = 100;
				Anti_Ghost[8] = 95;
				Anti_Ghost[9] = 80;
				Anti_Ghost[10] = 70;
				Anti_Ghost[11] = 60;
				Anti_Ghost[12] = 50;
				Anti_Ghost[13] = 40;
				Anti_Ghost[14] = 30;
				Anti_Ghost[15] = 26;
				Anti_Ghost[16] = 23;
				Anti_Ghost[17] = 21;
				Anti_Ghost[18] = 19;
				Anti_Ghost[19] = 17;
				Anti_Ghost[20] = 16;
				Anti_Ghost[21] = 14;
				Anti_Ghost[22] = 13;
				Anti_Ghost[23] = 12;
				Anti_Ghost[24] = 11;
				Anti_Ghost[25] = 10;
				Anti_Ghost[26] = 0;

				TemporalFrameWeight = 64;

				UseCalibrateVarOnly = 1;
			}

#if GXR_CPU_REFERENCE_DEBUG
			void snr()
			{
				timer_start();
				spafilter(cur.ptr<short>(), pre.ptr<short>(), brows, bcols,
					Filter_Th[color(0, 0, CFA_Order)].data, Filter_Th[color(0, 1, CFA_Order)].data, Filter_Th[color(1, 0, CFA_Order)].data, Filter_Th[color(1, 1, CFA_Order)].data,
					Filter_Th_Power.data, MAE_Preserve_LUT.data, Blending_Weight, CFA_Order);
				timer_end("spatial filter");
			}

			void tnr()
			{
				Mat spares(brows, bcols, CV_16SC1);
				Mat confidence_map_pre(hrows, hcols, CV_8UC1);
				Mat confidence_map_post(brows, bcols, CV_8UC1);

				timer_start();
				tnr_pre(cur.ptr<short>(), pre.ptr<short>(), spares.ptr<short>(), confidence_map_pre.data, brows, bcols,
					Filter_Th[color(0, 0, CFA_Order)].data, Filter_Th[color(0, 1, CFA_Order)].data, Filter_Th[color(1, 0, CFA_Order)].data, Filter_Th[color(1, 1, CFA_Order)].data,
					Filter_Th_Power.data, MAE_Preserve_LUT.data, Blending_Weight, CFA_Order,
					Var_R.ptr<ushort>(), Var_G.ptr<ushort>(), Discount_LUT.data, Anti_Ghost_LUT.data, UseCalibrateVarOnly);
				timer_end("tnr pre");

				timer_start();
				tnr_post(cur.ptr<short>(), pre.ptr<short>(), spares.ptr<short>(), confidence_map_pre.data, brows, bcols, TemporalFrameWeight, confidence_map_post.data);
				timer_end("tnr post");

#if GXR_CV_DEBUG
				imwrite("confidence_pre.jpg", confidence_map_pre);
				imwrite("confidence_post.jpg", confidence_map_post);
#endif
			}

			void run()
			{
				if (firstframe)
					snr();
				else
					tnr();

				pre.copyTo(cur);
			}
#else
			void clspafilter()
			{
				cl_mem src_img, dst_img;
				cl_mem filter_th_r, filter_th_g;
				cl_mem filter_th_power, mae_preserve_lut;
				int blending_weight = Blending_Weight;

				cl_int num_groups, err;

				src_img = clCreateBuffer(context, CL_MEM_READ_ONLY, bcols * brows * sizeof(short), NULL, &err);
				dst_img = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bcols * brows * sizeof(short), NULL, &err);
				filter_th_r = clCreateBuffer(context, CL_MEM_READ_ONLY, (1 << 10) * sizeof(uchar), NULL, &err);
				filter_th_g = clCreateBuffer(context, CL_MEM_READ_ONLY, (1 << 10) * sizeof(uchar), NULL, &err);
				filter_th_power = clCreateBuffer(context, CL_MEM_READ_ONLY, 4 * sizeof(uchar), NULL, &err);
				mae_preserve_lut = clCreateBuffer(context, CL_MEM_READ_ONLY, 64 * sizeof(uchar), NULL, &err);

				ocl->timer_start();
				err = clEnqueueWriteBuffer(queue, src_img, CL_TRUE, 0, bcols * brows * sizeof(short), cur.data, 0, NULL, NULL);
				err = clEnqueueWriteBuffer(queue, filter_th_r, CL_TRUE, 0, (1 << 10) * sizeof(uchar), Filter_Th_R.data, 0, NULL, NULL);
				err = clEnqueueWriteBuffer(queue, filter_th_g, CL_TRUE, 0, (1 << 10) * sizeof(uchar), Filter_Th_G.data, 0, NULL, NULL);
				err = clEnqueueWriteBuffer(queue, filter_th_power, CL_TRUE, 0, 4 * sizeof(uchar), Filter_Th_Power.data, 0, NULL, NULL);
				err = clEnqueueWriteBuffer(queue, mae_preserve_lut, CL_TRUE, 0, 64 * sizeof(uchar), MAE_Preserve_LUT.data, 0, NULL, NULL);
				ocl->timer_end("write input");
				if (err < 0)
				{
					GXR_Printf("Fail to write the buffer\n");
					CV_Assert(err >= 0);
				}

				err = clSetKernelArg(kernel_spafilter, 0, sizeof(cl_mem), &src_img);
				err |= clSetKernelArg(kernel_spafilter, 1, sizeof(cl_mem), &dst_img);
				err |= clSetKernelArg(kernel_spafilter, 2, sizeof(int), &brows);
				err |= clSetKernelArg(kernel_spafilter, 3, sizeof(int), &bcols);
				err |= clSetKernelArg(kernel_spafilter, 4, sizeof(cl_mem), color(0, 0, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_spafilter, 5, sizeof(cl_mem), color(0, 1, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_spafilter, 6, sizeof(cl_mem), color(1, 0, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_spafilter, 7, sizeof(cl_mem), color(1, 1, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_spafilter, 8, sizeof(cl_mem), &filter_th_power);
				err |= clSetKernelArg(kernel_spafilter, 9, sizeof(cl_mem), &mae_preserve_lut);
				err |= clSetKernelArg(kernel_spafilter, 10, sizeof(int), &blending_weight);
				err |= clSetKernelArg(kernel_spafilter, 11, sizeof(int), &CFA_Order);
				if (err < 0)
				{
					GXR_Printf("Fail to create spafilter kernel argument\n");
					CV_Assert(err >= 0);
				}

				size_t local_size[2] = { 8, 8 };
				size_t global_size[2];
				global_size[0] = (hcols + local_size[0] - 1) / local_size[0] * local_size[0];
				global_size[1] = (hrows + local_size[1] - 1) / local_size[1] * local_size[1];

				cl_event event;
				err = clEnqueueNDRangeKernel(queue, kernel_spafilter, 2, NULL, global_size, local_size, 0, NULL, &event);
				clWaitForEvents(1, &event);
				clFinish(queue);
				if (err < 0)
				{
					GXR_Printf("Fail to execute the spafilter kernel\n");
					CV_Assert(err >= 0);
				}
				else
				{
					cl_ulong time_start;
					cl_ulong time_end;
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
					double nanoSeconds = time_end - time_start;
					printf("OpenCl Spafilter Kernel Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
				}

				ocl->timer_start();
				err = clEnqueueReadBuffer(queue, dst_img, CL_TRUE, 0, bcols * brows * sizeof(short), pre.data, 0, NULL, NULL);
				ocl->timer_end("read output");
				if (err < 0)
				{
					GXR_Printf("Fail to read the buffer\n");
					CV_Assert(err >= 0);
				}

				clReleaseMemObject(src_img);
				clReleaseMemObject(dst_img);
				clReleaseMemObject(filter_th_r);
				clReleaseMemObject(filter_th_g);
				clReleaseMemObject(filter_th_power);
				clReleaseMemObject(mae_preserve_lut);
			}

			void cltnr()
			{
				cl_mem src_img, pre_img, spafilter_img, confidence_img, confidence_img_post;
				cl_mem filter_th_r, filter_th_g;
				cl_mem filter_th_power;
				cl_mem mae_preserve_lut;
				cl_mem var_r;
				cl_mem var_g;
				cl_mem discount_lut;
				cl_mem anti_ghost_lut;

				cl_int err;

				src_img = clCreateBuffer(context, CL_MEM_READ_ONLY, bcols * brows * sizeof(short), NULL, &err);
				pre_img = clCreateBuffer(context, CL_MEM_READ_WRITE, bcols * brows * sizeof(short), NULL, &err);
				spafilter_img = clCreateBuffer(context, CL_MEM_READ_WRITE, bcols * brows * sizeof(short), NULL, &err);
				confidence_img = clCreateBuffer(context, CL_MEM_READ_WRITE, hcols * hrows * sizeof(uchar), NULL, &err);
				confidence_img_post = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bcols * brows * sizeof(uchar), NULL, &err);

				filter_th_r = clCreateBuffer(context, CL_MEM_READ_ONLY, (1 << 10) * sizeof(uchar), NULL, &err);
				filter_th_g = clCreateBuffer(context, CL_MEM_READ_ONLY, (1 << 10) * sizeof(uchar), NULL, &err);
				filter_th_power = clCreateBuffer(context, CL_MEM_READ_ONLY, 4 * sizeof(uchar), NULL, &err);
				mae_preserve_lut = clCreateBuffer(context, CL_MEM_READ_ONLY, 64 * sizeof(uchar), NULL, &err);
				var_r = clCreateBuffer(context, CL_MEM_READ_ONLY, (1 << 9) * sizeof(ushort), NULL, &err);
				var_g = clCreateBuffer(context, CL_MEM_READ_ONLY, (1 << 9) * sizeof(ushort), NULL, &err);
				discount_lut = clCreateBuffer(context, CL_MEM_READ_ONLY, 16 * sizeof(uchar), NULL, &err);
				anti_ghost_lut = clCreateBuffer(context, CL_MEM_READ_ONLY, 27 * sizeof(uchar), NULL, &err);

				ocl->timer_start();
				err = clEnqueueWriteBuffer(queue, src_img, CL_TRUE, 0, bcols * brows * sizeof(short), cur.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, pre_img, CL_TRUE, 0, bcols * brows * sizeof(short), pre.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, filter_th_r, CL_TRUE, 0, (1 << 10) * sizeof(uchar), Filter_Th_R.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, filter_th_g, CL_TRUE, 0, (1 << 10) * sizeof(uchar), Filter_Th_G.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, filter_th_power, CL_TRUE, 0, 4 * sizeof(uchar), Filter_Th_Power.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, mae_preserve_lut, CL_TRUE, 0, 64 * sizeof(uchar), MAE_Preserve_LUT.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, var_r, CL_TRUE, 0, (1 << 9) * sizeof(ushort), Var_R.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, var_g, CL_TRUE, 0, (1 << 9) * sizeof(ushort), Var_G.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, discount_lut, CL_TRUE, 0, 16 * sizeof(uchar), Discount_LUT.data, 0, NULL, NULL);
				err |= clEnqueueWriteBuffer(queue, anti_ghost_lut, CL_TRUE, 0, 27 * sizeof(uchar), Anti_Ghost_LUT.data, 0, NULL, NULL);
				ocl->timer_end("write input");
				if (err < 0)
				{
					GXR_Printf("Fail to write the buffer\n");
					CV_Assert(err >= 0);
				}

				err = clSetKernelArg(kernel_tnr_pre, 0, sizeof(cl_mem), &src_img);
				err |= clSetKernelArg(kernel_tnr_pre, 1, sizeof(cl_mem), &pre_img);
				err |= clSetKernelArg(kernel_tnr_pre, 2, sizeof(cl_mem), &spafilter_img);
				err |= clSetKernelArg(kernel_tnr_pre, 3, sizeof(cl_mem), &confidence_img);
				err |= clSetKernelArg(kernel_tnr_pre, 4, sizeof(int), &brows);
				err |= clSetKernelArg(kernel_tnr_pre, 5, sizeof(int), &bcols);
				err |= clSetKernelArg(kernel_tnr_pre, 6, sizeof(cl_mem), color(0, 0, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_tnr_pre, 7, sizeof(cl_mem), color(0, 1, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_tnr_pre, 8, sizeof(cl_mem), color(1, 0, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_tnr_pre, 9, sizeof(cl_mem), color(1, 1, CFA_Order) == 1 ? &filter_th_g : &filter_th_r);
				err |= clSetKernelArg(kernel_tnr_pre, 10, sizeof(cl_mem), &filter_th_power);
				err |= clSetKernelArg(kernel_tnr_pre, 11, sizeof(cl_mem), &mae_preserve_lut);
				err |= clSetKernelArg(kernel_tnr_pre, 12, sizeof(int), &Blending_Weight);
				err |= clSetKernelArg(kernel_tnr_pre, 13, sizeof(int), &CFA_Order);
				err |= clSetKernelArg(kernel_tnr_pre, 14, sizeof(cl_mem), &var_r);
				err |= clSetKernelArg(kernel_tnr_pre, 15, sizeof(cl_mem), &var_g);
				err |= clSetKernelArg(kernel_tnr_pre, 16, sizeof(cl_mem), &discount_lut);
				err |= clSetKernelArg(kernel_tnr_pre, 17, sizeof(cl_mem), &anti_ghost_lut);
				err |= clSetKernelArg(kernel_tnr_pre, 18, sizeof(int), &UseCalibrateVarOnly);
				if (err < 0)
				{
					GXR_Printf("Fail to create tnr_pre kernel argument\n");
					CV_Assert(err >= 0);
				}

				size_t local_size[2] = { 8, 8 };
				size_t global_size[2];
				global_size[0] = (hcols + local_size[0] - 1) / local_size[0] * local_size[0];
				global_size[1] = (hrows + local_size[1] - 1) / local_size[1] * local_size[1];

				cl_event event;
				for (int count = 0; count < 1; count++)
				{
					err = clEnqueueNDRangeKernel(queue, kernel_tnr_pre, 2, NULL, global_size, local_size, 0, NULL, &event);
					clWaitForEvents(1, &event);
					clFinish(queue);
					if (err < 0)
					{
						GXR_Printf("Fail to execute the tnr_pre kernel\n");
						CV_Assert(err >= 0);
					}
					else
					{
						cl_ulong time_start;
						cl_ulong time_end;
						clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
						clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
						double nanoSeconds = time_end - time_start;
						GXR_Printf("OpenCl TNR_Pre Kernel Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
					}

					local_size[0] = 16;
					local_size[1] = 16;
					global_size[0] = (hcols + local_size[0] - 1) / local_size[0] * local_size[0];
					global_size[1] = (hrows + local_size[1] - 1) / local_size[1] * local_size[1];

					err = clSetKernelArg(kernel_tnr_post, 0, sizeof(cl_mem), &src_img);
					err |= clSetKernelArg(kernel_tnr_post, 1, sizeof(cl_mem), &pre_img);
					err |= clSetKernelArg(kernel_tnr_post, 2, sizeof(cl_mem), &spafilter_img);
					err |= clSetKernelArg(kernel_tnr_post, 3, sizeof(cl_mem), &confidence_img);
					err |= clSetKernelArg(kernel_tnr_post, 4, sizeof(int), &brows);
					err |= clSetKernelArg(kernel_tnr_post, 5, sizeof(int), &bcols);
					err |= clSetKernelArg(kernel_tnr_post, 6, sizeof(int), &TemporalFrameWeight);
					err |= clSetKernelArg(kernel_tnr_post, 7, sizeof(cl_mem), &confidence_img_post);
					if (err < 0)
					{
						GXR_Printf("Fail to create tnr_post kernel argument\n");
						CV_Assert(err >= 0);
					}

					err = clEnqueueNDRangeKernel(queue, kernel_tnr_post, 2, NULL, global_size, local_size, 0, NULL, &event);
					clWaitForEvents(1, &event);
					clFinish(queue);
					if (err < 0)
					{
						GXR_Printf("Fail to execute the tnr_post kernel\n");
						CV_Assert(err >= 0);
					}
					else
					{
						cl_ulong time_start;
						cl_ulong time_end;
						clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
						clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
						double nanoSeconds = time_end - time_start;
						GXR_Printf("OpenCl TNR_Post Kernel Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
					}
				}

				ocl->timer_start();
				err = clEnqueueReadBuffer(queue, pre_img, CL_TRUE, 0, bcols * brows * sizeof(short), pre.data, 0, NULL, NULL);
#if GXR_CV_DEBUG
				Mat mapviz(hrows, hcols, CV_8UC1);
				err |= clEnqueueReadBuffer(queue, confidence_img, CL_TRUE, 0, hcols * hrows * sizeof(uchar), mapviz.data, 0, NULL, NULL);
				imwrite("confidence_pre.jpg", mapviz);

				mapviz.create(brows, bcols, CV_8UC1);
				err |= clEnqueueReadBuffer(queue, confidence_img_post, CL_TRUE, 0, bcols * brows * sizeof(uchar), mapviz.data, 0, NULL, NULL);
				imwrite("confidence_post.jpg", mapviz);
#endif
				ocl->timer_end("read output");
				if (err < 0)
				{
					GXR_Printf("Fail to read the buffer");
					CV_Assert(err >= 0);
				}

				clReleaseMemObject(src_img);
				clReleaseMemObject(pre_img);
				clReleaseMemObject(spafilter_img);
				clReleaseMemObject(confidence_img);
				clReleaseMemObject(filter_th_r);
				clReleaseMemObject(filter_th_g);
				clReleaseMemObject(filter_th_power);
				clReleaseMemObject(mae_preserve_lut);
				clReleaseMemObject(var_r);
				clReleaseMemObject(var_g);
				clReleaseMemObject(discount_lut);
				clReleaseMemObject(anti_ghost_lut);
			}

			void run()
			{
				if (firstframe)
					clspafilter();
				else
					cltnr();

				pre.copyTo(cur);
			}
#endif
		};

		class Demosaic
		{
		public:
			OCL* ocl;
			cl_context& context;
			cl_command_queue& queue;
			cl_kernel& kernel_dm_pre;
			cl_kernel& kernel_dm_post;

			Mat& bgr_img, & yuv_img;
			int rows, cols;
			int hrows, hcols;

		public:
			Demosaic(RAW2BGR* r, OCL* ocl_, Mat& bgr_, Mat& yuv_)
				: ocl(ocl_), context(ocl_->context), queue(ocl_->queue), 
				kernel_dm_pre(ocl_->kernel_dm_pre), kernel_dm_post(ocl_->kernel_dm_post),
				bgr_img(bgr_), yuv_img(yuv_)
			{
				rows = bgr_img.rows;
				cols = bgr_img.cols;
				hrows = rows >> 1;
				hcols = cols >> 1;
			}

		public:
			void pre()
			{
				cl_mem bgr;
				cl_int err;
				cl_event event;

				cl_image_format format;
				format.image_channel_order = CL_RGB;
				format.image_channel_data_type = CL_SIGNED_INT16;

				cl_image_desc image_desc;
				image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
				image_desc.image_width = cols;
				image_desc.image_height = rows;
				image_desc.image_depth = 1;
				image_desc.image_array_size = 1;
				image_desc.image_row_pitch = 0;
				image_desc.image_slice_pitch = 0;
				image_desc.num_mip_levels = 0;
				image_desc.num_samples = 0;
				image_desc.mem_object = NULL;
				bgr = clCreateImage(context, CL_MEM_READ_ONLY, &format, &image_desc, NULL, &err);

				size_t origin[3] = { 0, 0, 0 };
				size_t region[3] = { cols, rows, 1 };
				ocl->timer_start();
				err = clEnqueueWriteImage(queue, bgr, CL_TRUE, origin, region, sizeof(short) * cols, 0, bgr_img.data, 0, NULL, &event);
				ocl->timer_end("write bgr src image");
				if (err < 0)
				{
					GXR_Printf("Fail to write the bgr src image\n");
					CV_Assert(err >= 0);
				}

				err = clSetKernelArg(kernel_dm_pre, 0, sizeof(cl_mem), &raw);
				err |= clSetKernelArg(kernel_dm_pre, 1, sizeof(cl_mem), &gc);
				err |= clSetKernelArg(kernel_dm_pre, 2, sizeof(int), &rows);
				err |= clSetKernelArg(kernel_dm_pre, 3, sizeof(int), &cols);
				if (err < 0)
				{
					GXR_Printf("Fail to create kernel_dm_pre argument");
					CV_Assert(err >= 0);
				}

				size_t local_size[2] = { 8, 8 };
				size_t global_size[2];
				global_size[0] = (hcols + local_size[0] - 1) / local_size[0] * local_size[0];
				global_size[1] = (hrows + local_size[1] - 1) / local_size[1] * local_size[1];

				err = clEnqueueNDRangeKernel(queue, kernel_dm_pre, 2, NULL, global_size, local_size, 0, NULL, &event);
				clWaitForEvents(1, &event);
				clFinish(queue);
				if (err < 0)
				{
					GXR_Printf("Fail to execute the kernel_dm_pre");
					CV_Assert(err >= 0);
				}
				else
				{
					cl_ulong time_start;
					cl_ulong time_end;
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
					double nanoSeconds = time_end - time_start;
					printf("OpenCl kernel_dm_pre Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
				}

				ocl->timer_start();
				err = clEnqueueReadBuffer(queue, gc, CL_TRUE, 0, sizeof(short) * cols * rows, gc_img.data, 0, NULL, &event);
				ocl->timer_end("read gc image");

#if GXR_CV_DEBUG
				Mat viz;
				gc_img.convertTo(viz, CV_8U, 255.0 / 4096.0);
				imwrite("gc.jpg", src_img);
#endif

				clReleaseMemObject(src);
				clReleaseMemObject(dst);
				clReleaseMemObject(Th_L);
				clReleaseMemObject(Th_H);
			}

			void post()
			{}

			void run()
			{
			}
		};

		class Denoiser
		{
		public:
			OCL * ocl;
			Mat& src_img, & dst_img;
			cl_context & context;
			cl_command_queue & queue;
			cl_kernel & kernel;

			int rows, cols, cn;
			int th_low[8], th_high[4];

		public:
			Denoiser(OCL* ocl_, Mat& src_, Mat& dst_)
				: ocl(ocl_), context(ocl_->context), queue(ocl_->queue), kernel(src_.channels() == 1 ? ocl_->kernel_ydenoise : ocl_->kernel_cdenoise), src_img(src_), dst_img(dst_)
			{
				rows = src_img.rows;
				cols = src_img.cols;
				cn = src_img.channels();
				th_low[0] = 0;
				th_low[1] = 1;
				th_low[2] = 1;
				th_low[3] = 2;
				th_low[4] = 2;
				th_low[5] = 3;
				th_low[6] = 3;
				th_low[7] = 3;
				th_high[0] = 1;
				th_high[1] = 2;
				th_high[2] = 2;
				th_high[3] = 3;
			}

			void run()
			{
				cl_mem src, dst;
				cl_mem Th_L, Th_H;
				cl_int err;
				cl_event event;
				
				cl_image_format format;
				format.image_channel_order = cn == 1 ? CL_R : CL_RG;
				format.image_channel_data_type = CL_UNSIGNED_INT8;

				cl_image_desc image_desc;
				image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
				image_desc.image_width = cols;
				image_desc.image_height = rows;
				image_desc.image_depth = 1;
				image_desc.image_array_size = 1;
				image_desc.image_row_pitch = 0;
				image_desc.image_slice_pitch = 0;
				image_desc.num_mip_levels = 0;
				image_desc.num_samples = 0;
				image_desc.mem_object = NULL;
				src = clCreateImage(context, CL_MEM_READ_ONLY, &format, &image_desc, NULL, &err);

				dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, cn * cols * rows, NULL, &err);
				Th_L = clCreateBuffer(context, CL_MEM_READ_ONLY, 8 * sizeof(int), NULL, &err);
				Th_H = clCreateBuffer(context, CL_MEM_READ_ONLY, 4 * sizeof(int), NULL, &err);

				size_t origin[3] = {0, 0, 0};
				size_t region[3] = {cols, rows, 1};
				ocl->timer_start();
				err = clEnqueueWriteImage(queue, src, CL_TRUE, origin, region, cn * cols, 0, src_img.data, 0, NULL, &event);
				ocl->timer_end("write src image");
				if (err < 0)
				{
					GXR_Printf("Fail to write the src image\n");
					CV_Assert(err >= 0);
				}

				ocl->timer_start();
				clEnqueueWriteBuffer(queue, Th_L, CL_TRUE, 0, 8 * sizeof(int), th_low, 0, NULL, &event);
				clEnqueueWriteBuffer(queue, Th_H, CL_TRUE, 0, 4 * sizeof(int), th_high, 0, NULL, &event);
				ocl->timer_end("write argument");

				err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &src);
				err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dst);
				err |= clSetKernelArg(kernel, 2, sizeof(int), &rows);
				err |= clSetKernelArg(kernel, 3, sizeof(int), &cols);
				err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &Th_L);
				err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &Th_H);
				if (err < 0)
				{
					GXR_Printf("Fail to create denoise kernel argument");
					CV_Assert(err >= 0);
				}

				size_t local_size[2] = { 16, 16 };
				size_t global_size[2];
				global_size[0] = (cols + local_size[0] - 1) / local_size[0] * local_size[0];
				global_size[1] = (rows + local_size[1] - 1) / local_size[1] * local_size[1];

				err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, &event);
				clWaitForEvents(1, &event);
				clFinish(queue);
				if (err < 0)
				{
					GXR_Printf("Fail to execute the denoise kernel");
					CV_Assert(err >= 0);
				}
				else
				{
					cl_ulong time_start;
					cl_ulong time_end;
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
					double nanoSeconds = time_end - time_start;
					printf("OpenCl denoise Kernel Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
				}

				ocl->timer_start();
				err = clEnqueueReadBuffer(queue, dst, CL_TRUE, 0, cn * cols * rows, dst_img.data, 0, NULL, &event);
				ocl->timer_end("read dst image");

#if GXR_CV_DEBUG
				if (cn == 1)
				{
					imwrite("src_img_denoise.jpg", src_img);
					imwrite("dst_img_denoise.jpg", dst_img);
				}
#endif

				clReleaseMemObject(src);
				clReleaseMemObject(dst);
				clReleaseMemObject(Th_L);
				clReleaseMemObject(Th_H);
			}
		};

		class Downer
		{
		public:
			OCL* ocl;
			Mat& src_img, & dst_img;
			cl_context& context;
			cl_command_queue& queue;
			cl_kernel& kernel;

			int rows, cols, cn;
			int drows, dcols;

		public:
			Downer(OCL* ocl_, Mat& src_, Mat& dst_)
				: ocl(ocl_), context(ocl_->context), queue(ocl_->queue), kernel(src_.channels() == 1 ? ocl_->kernel_ydown : ocl_->kernel_cdown), src_img(src_), dst_img(dst_)
			{
				rows = src_img.rows;
				cols = src_img.cols;
				cn = src_img.channels();
				drows = dst_img.rows;
				dcols = dst_img.cols;
			}

			void run()
			{
				cl_mem src, dst;
				cl_int err;
				cl_event event;

				cl_image_format format;
				format.image_channel_order = cn == 1 ? CL_R : CL_RG;
				format.image_channel_data_type = CL_UNSIGNED_INT8;

				cl_image_desc image_desc;
				image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
				image_desc.image_width = cols;
				image_desc.image_height = rows;
				image_desc.image_depth = 1;
				image_desc.image_array_size = 1;
				image_desc.image_row_pitch = 0;
				image_desc.image_slice_pitch = 0;
				image_desc.num_mip_levels = 0;
				image_desc.num_samples = 0;
				image_desc.mem_object = NULL;
				src = clCreateImage(context, CL_MEM_READ_ONLY, &format, &image_desc, NULL, &err);

				dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, cn * dcols * drows, NULL, &err);

				size_t origin[3] = { 0, 0, 0 };
				size_t region[3] = { cols, rows, 1 };
				ocl->timer_start();
				err = clEnqueueWriteImage(queue, src, CL_TRUE, origin, region, cn * cols, 0, src_img.data, 0, NULL, &event);
				ocl->timer_end("write src image");
				if (err < 0)
				{
					GXR_Printf("Fail to write the src image\n");
					CV_Assert(err >= 0);
				}

				err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &src);
				err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dst);
				err |= clSetKernelArg(kernel, 2, sizeof(int), &drows);
				err |= clSetKernelArg(kernel, 3, sizeof(int), &dcols);
				if (err < 0)
				{
					GXR_Printf("Fail to create down kernel argument");
					CV_Assert(err >= 0);
				}

				size_t local_size[2] = { 16, 16 };
				size_t global_size[2];
				global_size[0] = (dcols + local_size[0] - 1) / local_size[0] * local_size[0];
				global_size[1] = (drows + local_size[1] - 1) / local_size[1] * local_size[1];

				err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, &event);
				clWaitForEvents(1, &event);
				clFinish(queue);
				if (err < 0)
				{
					GXR_Printf("Fail to execute the down kernel");
					CV_Assert(err >= 0);
				}
				else
				{
					cl_ulong time_start;
					cl_ulong time_end;
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
					double nanoSeconds = time_end - time_start;
					printf("OpenCl down Kernel Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
				}

				ocl->timer_start();
				err = clEnqueueReadBuffer(queue, dst, CL_TRUE, 0, cn * dcols * drows, dst_img.data, 0, NULL, &event);
				ocl->timer_end("read dst image");

#if GXR_CV_DEBUG
				if (cn == 1)
				{
					imwrite("src_img_down.jpg", src_img);
					imwrite("dst_img_down.jpg", dst_img);
				}
#endif

				clReleaseMemObject(src);
				clReleaseMemObject(dst);
			}
		};

		class Uper
		{
		public:
			OCL* ocl;
			Mat& src_img, & dst_img;
			Mat& hsrc_img, & hdst_img;
			cl_context& context;
			cl_command_queue& queue;
			cl_kernel& kernel;

			int rows, cols, cn;
			int drows, dcols;

		public:
			Uper(OCL* ocl_, Mat& src_, Mat& dst_, Mat& hsrc_, Mat& hdst_)
				: ocl(ocl_), context(ocl_->context), queue(ocl_->queue), kernel(src_.channels() == 1 ? ocl_->kernel_yupmerge : ocl_->kernel_cmerge),
				src_img(src_), dst_img(dst_), hsrc_img(hsrc_), hdst_img(hdst_)
			{
				rows = src_img.rows;
				cols = src_img.cols;
				cn = src_img.channels();
				drows = hsrc_img.rows;
				dcols = hsrc_img.cols;
			}

			void run()
			{
				cl_mem src, dst;
				cl_mem hsrc, hdst;
				cl_int err;
				cl_event event;

				{
					cl_image_format format;
					format.image_channel_order = cn == 1 ? CL_R : CL_RG;
					format.image_channel_data_type = CL_UNSIGNED_INT8;

					cl_image_desc image_desc;
					image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
					image_desc.image_width = cols;
					image_desc.image_height = rows;
					image_desc.image_depth = 1;
					image_desc.image_array_size = 1;
					image_desc.image_row_pitch = 0;
					image_desc.image_slice_pitch = 0;
					image_desc.num_mip_levels = 0;
					image_desc.num_samples = 0;
					image_desc.mem_object = NULL;
					src = clCreateImage(context, CL_MEM_READ_ONLY, &format, &image_desc, NULL, &err);
				}

				dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, cn * cols * rows, NULL, &err);

				{
					cl_image_format format;
					format.image_channel_order = cn == 1 ? CL_R : CL_RG;
					format.image_channel_data_type = CL_UNSIGNED_INT8;

					cl_image_desc image_desc;
					image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
					image_desc.image_width = dcols;
					image_desc.image_height = drows;
					image_desc.image_depth = 1;
					image_desc.image_array_size = 1;
					image_desc.image_row_pitch = 0;
					image_desc.image_slice_pitch = 0;
					image_desc.num_mip_levels = 0;
					image_desc.num_samples = 0;
					image_desc.mem_object = NULL;
					hsrc = clCreateImage(context, CL_MEM_READ_ONLY, &format, &image_desc, NULL, &err);
					hdst = clCreateImage(context, CL_MEM_READ_ONLY, &format, &image_desc, NULL, &err);
				}

				{
					size_t origin[3] = { 0, 0, 0 };
					size_t region[3] = { cols, rows, 1 };
					ocl->timer_start();
					err = clEnqueueWriteImage(queue, src, CL_TRUE, origin, region, cn * cols, 0, src_img.data, 0, NULL, &event);
					ocl->timer_end("write src image");
					if (err < 0)
					{
						GXR_Printf("Fail to write the src image\n");
						CV_Assert(err >= 0);
					}
				}

				{
					size_t origin[3] = { 0, 0, 0 };
					size_t region[3] = { dcols, drows, 1 };
					ocl->timer_start();
					err = clEnqueueWriteImage(queue, hsrc, CL_TRUE, origin, region, cn * dcols, 0, hsrc_img.data, 0, NULL, &event);
					ocl->timer_end("write hsrc image");
					if (err < 0)
					{
						GXR_Printf("Fail to write the hsrc image\n");
						CV_Assert(err >= 0);
					}

					ocl->timer_start();
					err = clEnqueueWriteImage(queue, hdst, CL_TRUE, origin, region, cn * dcols, 0, hdst_img.data, 0, NULL, &event);
					ocl->timer_end("write hdst image");
					if (err < 0)
					{
						GXR_Printf("Fail to write the hdst image\n");
						CV_Assert(err >= 0);
					}
				}

				err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &hsrc);
				err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &hdst);
				err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &src);
				err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &dst);
				err |= clSetKernelArg(kernel, 4, sizeof(int), &rows);
				err |= clSetKernelArg(kernel, 5, sizeof(int), &cols);
				if (err < 0)
				{
					GXR_Printf("Fail to create merge kernel argument");
					CV_Assert(err >= 0);
				}

				size_t local_size[2] = { 16, 16 };
				size_t global_size[2];
				global_size[0] = (cols + local_size[0] - 1) / local_size[0] * local_size[0];
				global_size[1] = (rows + local_size[1] - 1) / local_size[1] * local_size[1];

				err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, &event);
				clWaitForEvents(1, &event);
				clFinish(queue);
				if (err < 0)
				{
					GXR_Printf("Fail to execute the merge kernel");
					CV_Assert(err >= 0);
				}
				else
				{
					cl_ulong time_start;
					cl_ulong time_end;
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
					clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
					double nanoSeconds = time_end - time_start;
					printf("OpenCl merge Kernel Execution time is: %0.3f milliseconds \n", nanoSeconds / 1000000.0);
				}

				ocl->timer_start();
				err = clEnqueueReadBuffer(queue, dst, CL_TRUE, 0, cn * cols * rows, dst_img.data, 0, NULL, &event);
				ocl->timer_end("read dst image");

#if GXR_CV_DEBUG
				if (cn == 1)
				{
					imwrite("src_img_upmerge.jpg", src_img);
					imwrite("dst_img_upmerge.jpg", dst_img);
				}
#endif

				clReleaseMemObject(src);
				clReleaseMemObject(dst);
			}
		};

		class LSC : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			unsigned pat;
			int rows, cols, slli;
			int kcfa[4];
			Mat& lscflat, &raw;

		public:
			LSC(RAW2BGR* r);
			void operator ()(Range const& range) const override;
		};

		class Stats : public ParallelLoopBody
		{
		public:
			RAW2BGR* raw2;
			unsigned pat;
			Vec4i cfa;
			int aesat;
			int rows, cols;
			int blksz_y, blksz_x;
			int blknum_y, blknum_x;
			Mat& raw, & awb_blockstats, & ae_blockstats, & histgram;

		public:
			Stats(RAW2BGR* r);
			void operator ()(Range const& range) const override;
		};

		class Gain : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			unsigned pat;
			int rows, cols;
			int gcfa[4];
			Mat& raw, & bgr;

		public:
			Gain(RAW2BGR* r);
			void operator ()(Range const& range) const override;
		};

		class GetColor : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			unsigned pat;
			int rows, cols, pass;
			Mat& bgr;

		public:
			GetColor(RAW2BGR* r);
			void nextpass();
			void operator ()(Range const& range) const override;
		};

		class CCMGamma : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			int rows, cols;
			uchar* T;
			short ccm[9];

		public:
			CCMGamma(RAW2BGR* r);
			void operator ()(Range const& range) const override;
		};

		class BGR2YCbCr : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			int brows, bcols;
			int hrows, hcols;
			Mat&bgr, & luma, & chroma;
			float y0, y1, y2, cb0, cr0;
			short y0s, y1s, y2s, cb0s, cr0s;

		public:
			BGR2YCbCr(RAW2BGR* r);
			void operator ()(Range const& range) const override;
		};

		class DenoiseImpl : public ParallelLoopBody
		{
			Mat& img, & buffer;
			int rows, cols, chs, step, noise;
			float bdlarge;

		public:
			DenoiseImpl(Mat& img_, Mat& buffer_, int noise_, float bdlarge_);
			void operator ()(Range const& range) const override;
		};

		class FilterUp : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			Mat& hsrc;
			Mat& hdst;
			Mat& dst;
			mutable Mat buffer;
		public:
			FilterUp(RAW2BGR* raw2_, Mat& hsrc_, Mat& hdst_, Mat& dst_);
			void operator ()(Range const& range) const override;
		};

		class YNR
		{
			RAW2BGR* raw2;
			Mat& luma;

		public:
			YNR(RAW2BGR* raw2);
			void run_each(Mat* src, Mat* dst, int* noise, int i);
			void run();
		};

		class Sharpen : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			Mat& src;
			mutable Mat buffer;
			int cols, sharpscale;

		public:
			Sharpen(RAW2BGR* raw2, Mat& src_, float sharpscale_);
			void operator ()(Range const& range) const override;
		};

		class CNR
		{
			RAW2BGR* raw2;
			Mat& chroma;

		public:
			CNR(RAW2BGR* raw2);
			void run_each(Mat* src, Mat* dst, int* noise, int i);
			void run();
		};

		class YCbCr2BGR : public ParallelLoopBody
		{
			RAW2BGR* raw2;
			int brows, bcols;
			int hrows, hcols;
			Mat& bgr, & luma, & chroma;
			mutable Mat tmp;
			float b0, g0, g1, r0;
			short b0s, g0s, g1s, r0s;

		public:
			YCbCr2BGR(RAW2BGR* r);
			void operator ()(Range const& range) const override;
		};

		class Ungroup
		{
			RAW2BGR* raw2;
			Mat& dst, tmp;
			int srows, scols;
			int brows, bcols;
			int sh0, sh1;
			int sw0, sw1;
			int dh0, dh1;
			int dw0, dw1;

		public:
			Ungroup(RAW2BGR* raw2);
			void run();
		};

		//RawGroup Crawgroup;
		//LSC Clsc;
		//Gain Cgain;
		//GetColor Cgetcolor;
		//CCMGamma Cccmgamma;
		//BGR2YCbCr Cbgr2ycbcr;
		//YNR Cynr;
		//CNR Ccnr;
		//YCbCr2BGR Cycbcr2bgr;
		//Ungroup Cungroup;
	};
}