#pragma once
#include <array>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/core/hal/intrin.hpp"
#include "utility.hpp"

using namespace cv;

namespace gxr
{

	template<typename T, int shift> struct FixPtCast
	{
		typedef int type1;
		typedef T rtype;
		enum { delta = (1 << (shift - 1)) };
		rtype operator ()(type1 arg) const
		{
			return (T)((arg + delta) >> shift);
		}
	};

	template<typename T, int shift> struct FltCast
	{
		typedef T type1;
		typedef T rtype;
		rtype operator ()(type1 arg) const { return arg * (T)(1. / (1 << shift)); }
	};

	template<typename T1, typename T2, int cn> int GauDownVecH(const T1*, T2*, int)
	{
		//   row[x       ] = src[x * 2 + 2*cn  ] * 6 + (src[x * 2 +   cn  ] + src[x * 2 + 3*cn  ]) * 4 + src[x * 2       ] + src[x * 2 + 4*cn  ];
		//   row[x +    1] = src[x * 2 + 2*cn+1] * 6 + (src[x * 2 +   cn+1] + src[x * 2 + 3*cn+1]) * 4 + src[x * 2 +    1] + src[x * 2 + 4*cn+1];
		//   ....
		//   row[x + cn-1] = src[x * 2 + 3*cn-1] * 6 + (src[x * 2 + 2*cn-1] + src[x * 2 + 4*cn-1]) * 4 + src[x * 2 + cn-1] + src[x * 2 + 5*cn-1];
		return 0;
	}

	template<typename T1, typename T2, int cn> int GauUpVecH(const T1*, T2*, int)
	{
		return 0;
	}

	template<typename T1, typename T2> int GauDownVecV(T1**, T2*, int) { return 0; }

	template<typename T1, typename T2> int GauUpVecV(T1**, T2**, int) { return 0; }

	template<typename T1, typename T2> int DiffGauUpVecV(T1**, T2**, int) { return 0; }

	template<> int GauDownVecH<uchar, int, 1>(const uchar* src, int* row, int width)
	{
		int x = 0;
		const uchar* src01 = src, * src23 = src + 2, * src4 = src + 3;

		v_int16 v_1_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040001));
		v_int16 v_6_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040006));
		for (; x <= width - v_int32::nlanes; x += v_int32::nlanes, src01 += v_int16::nlanes, src23 += v_int16::nlanes, src4 += v_int16::nlanes, row += v_int32::nlanes)
			v_store(row, v_dotprod(v_reinterpret_as_s16(vx_load_expand(src01)), v_1_4) +
				v_dotprod(v_reinterpret_as_s16(vx_load_expand(src23)), v_6_4) +
				(v_reinterpret_as_s32(vx_load_expand(src4)) >> 16));
		vx_cleanup();

		return x;
	}
	template<> int GauDownVecH<uchar, int, 2>(const uchar * src, int* row, int width)
	{
		int x = 0;
		const uchar* src01 = src, * src23 = src + 4, * src4 = src + 6;

		v_int16 v_1_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040001));
		v_int16 v_6_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040006));
		for (; x <= width - v_int32::nlanes; x += v_int32::nlanes, src01 += v_int16::nlanes, src23 += v_int16::nlanes, src4 += v_int16::nlanes, row += v_int32::nlanes)
			v_store(row, v_dotprod(v_interleave_pairs(v_reinterpret_as_s16(vx_load_expand(src01))), v_1_4) +
				v_dotprod(v_interleave_pairs(v_reinterpret_as_s16(vx_load_expand(src23))), v_6_4) +
				(v_reinterpret_as_s32(v_interleave_pairs(vx_load_expand(src4))) >> 16));
		vx_cleanup();

		return x;
	}
	template<> int GauDownVecH<uchar, int, 3>(const uchar * src, int* row, int width)
	{
		int idx[v_int8::nlanes / 2 + 4];
		for (int i = 0; i < v_int8::nlanes / 4 + 2; i++)
		{
			idx[i] = 6 * i;
			idx[i + v_int8::nlanes / 4 + 2] = 6 * i + 3;
		}

		int x = 0;
		v_int16 v_6_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040006));
		for (; x <= width - v_int8::nlanes; x += 3 * v_int8::nlanes / 4, src += 6 * v_int8::nlanes / 4, row += 3 * v_int8::nlanes / 4)
		{
			v_uint16 r0l, r0h, r1l, r1h, r2l, r2h, r3l, r3h, r4l, r4h;
			v_expand(vx_lut_quads(src, idx), r0l, r0h);
			v_expand(vx_lut_quads(src, idx + v_int8::nlanes / 4 + 2), r1l, r1h);
			v_expand(vx_lut_quads(src, idx + 1), r2l, r2h);
			v_expand(vx_lut_quads(src, idx + v_int8::nlanes / 4 + 3), r3l, r3h);
			v_expand(vx_lut_quads(src, idx + 2), r4l, r4h);

			v_zip(r2l, r1l + r3l, r1l, r3l);
			v_zip(r2h, r1h + r3h, r1h, r3h);
			r0l += r4l; r0h += r4h;

			v_store(row, v_pack_triplets(v_dotprod(v_reinterpret_as_s16(r1l), v_6_4) + v_reinterpret_as_s32(v_expand_low(r0l))));
			v_store(row + 3 * v_int32::nlanes / 4, v_pack_triplets(v_dotprod(v_reinterpret_as_s16(r3l), v_6_4) + v_reinterpret_as_s32(v_expand_high(r0l))));
			v_store(row + 6 * v_int32::nlanes / 4, v_pack_triplets(v_dotprod(v_reinterpret_as_s16(r1h), v_6_4) + v_reinterpret_as_s32(v_expand_low(r0h))));
			v_store(row + 9 * v_int32::nlanes / 4, v_pack_triplets(v_dotprod(v_reinterpret_as_s16(r3h), v_6_4) + v_reinterpret_as_s32(v_expand_high(r0h))));
		}
		vx_cleanup();

		return x;
	}
	template<> int GauDownVecH<uchar, int, 4>(const uchar * src, int* row, int width)
	{
		int x = 0;
		const uchar* src01 = src, * src23 = src + 8, * src4 = src + 12;

		v_int16 v_1_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040001));
		v_int16 v_6_4 = v_reinterpret_as_s16(vx_setall_u32(0x00040006));
		for (; x <= width - v_int32::nlanes; x += v_int32::nlanes, src01 += v_int16::nlanes, src23 += v_int16::nlanes, src4 += v_int16::nlanes, row += v_int32::nlanes)
			v_store(row, v_dotprod(v_interleave_quads(v_reinterpret_as_s16(vx_load_expand(src01))), v_1_4) +
				v_dotprod(v_interleave_quads(v_reinterpret_as_s16(vx_load_expand(src23))), v_6_4) +
				(v_reinterpret_as_s32(v_interleave_quads(vx_load_expand(src4))) >> 16));
		vx_cleanup();

		return x;
	}
	template<> int GauDownVecV<int, uchar>(int** src, uchar * dst, int width)
	{
		int x = 0;
		const int* row0 = src[0], * row1 = src[1], * row2 = src[2], * row3 = src[3], * row4 = src[4];

		for (; x <= width - v_uint8::nlanes; x += v_uint8::nlanes)
		{
			v_uint16 r0, r1, r2, r3, r4, t0, t1;
			r0 = v_reinterpret_as_u16(v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)));
			r1 = v_reinterpret_as_u16(v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)));
			r2 = v_reinterpret_as_u16(v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)));
			r3 = v_reinterpret_as_u16(v_pack(vx_load(row3 + x), vx_load(row3 + x + v_int32::nlanes)));
			r4 = v_reinterpret_as_u16(v_pack(vx_load(row4 + x), vx_load(row4 + x + v_int32::nlanes)));
			t0 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);
			r0 = v_reinterpret_as_u16(v_pack(vx_load(row0 + x + 2 * v_int32::nlanes), vx_load(row0 + x + 3 * v_int32::nlanes)));
			r1 = v_reinterpret_as_u16(v_pack(vx_load(row1 + x + 2 * v_int32::nlanes), vx_load(row1 + x + 3 * v_int32::nlanes)));
			r2 = v_reinterpret_as_u16(v_pack(vx_load(row2 + x + 2 * v_int32::nlanes), vx_load(row2 + x + 3 * v_int32::nlanes)));
			r3 = v_reinterpret_as_u16(v_pack(vx_load(row3 + x + 2 * v_int32::nlanes), vx_load(row3 + x + 3 * v_int32::nlanes)));
			r4 = v_reinterpret_as_u16(v_pack(vx_load(row4 + x + 2 * v_int32::nlanes), vx_load(row4 + x + 3 * v_int32::nlanes)));
			t1 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);
			v_store(dst + x, v_rshr_pack<8>(t0, t1));
		}
		if (x <= width - v_int16::nlanes)
		{
			v_uint16 r0, r1, r2, r3, r4, t0;
			r0 = v_reinterpret_as_u16(v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)));
			r1 = v_reinterpret_as_u16(v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)));
			r2 = v_reinterpret_as_u16(v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)));
			r3 = v_reinterpret_as_u16(v_pack(vx_load(row3 + x), vx_load(row3 + x + v_int32::nlanes)));
			r4 = v_reinterpret_as_u16(v_pack(vx_load(row4 + x), vx_load(row4 + x + v_int32::nlanes)));
			t0 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);
			v_rshr_pack_store<8>(dst + x, t0);
			x += v_uint16::nlanes;
		}
		typedef int CV_DECL_ALIGNED(1) unaligned_int;
		for (; x <= width - v_int32x4::nlanes; x += v_int32x4::nlanes)
		{
			v_int32x4 r0, r1, r2, r3, r4, t0;
			r0 = v_load(row0 + x);
			r1 = v_load(row1 + x);
			r2 = v_load(row2 + x);
			r3 = v_load(row3 + x);
			r4 = v_load(row4 + x);
			t0 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);

			*((unaligned_int*)(dst + x)) = v_reinterpret_as_s32(v_rshr_pack<8>(v_pack_u(t0, t0), v_setzero_u16())).get0();
		}
		vx_cleanup();

		return x;
	}
	template <> int GauUpVecV<int, uchar>(int** src, uchar** dst, int width)
	{
		int x = 0;
		uchar* dst0 = dst[0], * dst1 = dst[1];
		const int* row0 = src[0], * row1 = src[1], * row2 = src[2];

		for (; x <= width - v_uint8::nlanes; x += v_uint8::nlanes)
		{
			v_int16 v_r00 = v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)),
				v_r01 = v_pack(vx_load(row0 + x + 2 * v_int32::nlanes), vx_load(row0 + x + 3 * v_int32::nlanes)),
				v_r10 = v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)),
				v_r11 = v_pack(vx_load(row1 + x + 2 * v_int32::nlanes), vx_load(row1 + x + 3 * v_int32::nlanes)),
				v_r20 = v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)),
				v_r21 = v_pack(vx_load(row2 + x + 2 * v_int32::nlanes), vx_load(row2 + x + 3 * v_int32::nlanes));
			v_int16 v_2r10 = v_r10 + v_r10, v_2r11 = (v_r11 + v_r11);
			v_store(dst0 + x, v_rshr_pack_u<6>(v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10), v_r01 + v_r21 + (v_2r11 + v_2r11 + v_2r11)));
			v_store(dst1 + x, v_rshr_pack_u<6>((v_r10 + v_r20) << 2, (v_r11 + v_r21) << 2));
		}
		if (x <= width - v_uint16::nlanes)
		{
			v_int16 v_r00 = v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)),
				v_r10 = v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)),
				v_r20 = v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes));
			v_int16 v_2r10 = v_r10 + v_r10;
			v_rshr_pack_u_store<6>(dst0 + x, v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10));
			v_rshr_pack_u_store<6>(dst1 + x, (v_r10 + v_r20) << 2);
			x += v_uint16::nlanes;
		}
		typedef int CV_DECL_ALIGNED(1) unaligned_int;
		for (; x <= width - v_int32x4::nlanes; x += v_int32x4::nlanes)
		{
			v_int32 v_r00 = vx_load(row0 + x),
				v_r10 = vx_load(row1 + x),
				v_r20 = vx_load(row2 + x);
			v_int32 v_2r10 = v_r10 + v_r10;
			v_int16 d = v_pack(v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10), (v_r10 + v_r20) << 2);
			*(unaligned_int*)(dst0 + x) = v_reinterpret_as_s32(v_rshr_pack_u<6>(d, vx_setzero_s16())).get0();
			*(unaligned_int*)(dst1 + x) = v_reinterpret_as_s32(v_rshr_pack_u<6>(v_combine_high(d, d), vx_setzero_s16())).get0();
		}
		vx_cleanup();

		return x;
	}
	template <> int DiffGauUpVecV<int, uchar>(int** src, uchar** dst, int width)
	{
		int x = 0;
		uchar* dst0 = dst[0], * dst1 = dst[1];
		const int* row0 = src[0], * row1 = src[1], * row2 = src[2];
		for (; x <= width - v_uint8::nlanes; x += v_uint8::nlanes)
		{
			v_int16 v_r00 = v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)),
				v_r01 = v_pack(vx_load(row0 + x + 2 * v_int32::nlanes), vx_load(row0 + x + 3 * v_int32::nlanes)),
				v_r10 = v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)),
				v_r11 = v_pack(vx_load(row1 + x + 2 * v_int32::nlanes), vx_load(row1 + x + 3 * v_int32::nlanes)),
				v_r20 = v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)),
				v_r21 = v_pack(vx_load(row2 + x + 2 * v_int32::nlanes), vx_load(row2 + x + 3 * v_int32::nlanes));
			v_int16 v_2r10 = v_r10 + v_r10, v_2r11 = (v_r11 + v_r11);
			v_uint8 vd = vx_load(dst0 + x);
			v_store(dst0 + x, v_rshr_pack_u<6>(v_reinterpret_as_s16(v_expand_low(vd) << 6) + v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10), v_reinterpret_as_s16(v_expand_high(vd) << 6) + v_r01 + v_r21 + (v_2r11 + v_2r11 + v_2r11)));
			vd = vx_load(dst1 + x);
			v_store(dst1 + x, v_rshr_pack_u<6>(v_reinterpret_as_s16(v_expand_low(vd) << 6) + ((v_r10 + v_r20) << 2), v_reinterpret_as_s16(v_expand_high(vd) << 6) + ((v_r11 + v_r21) << 2)));
		}
		if (x <= width - v_uint16::nlanes)
		{
			v_int16 v_r00 = v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)),
				v_r10 = v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)),
				v_r20 = v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes));
			v_int16 v_2r10 = v_r10 + v_r10;
			v_rshr_pack_u_store<6>(dst0 + x, v_reinterpret_as_s16(vx_load_expand(dst0 + x) << 6) + v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10));
			v_rshr_pack_u_store<6>(dst1 + x, v_reinterpret_as_s16(vx_load_expand(dst1 + x) << 6) + ((v_r10 + v_r20) << 2));
			x += v_uint16::nlanes;
		}
		typedef int CV_DECL_ALIGNED(1) unaligned_int;
		for (; x <= width - v_int32x4::nlanes; x += v_int32x4::nlanes)
		{
			v_int32 v_r00 = vx_load(row0 + x),
				v_r10 = vx_load(row1 + x),
				v_r20 = vx_load(row2 + x);
			v_int32 v_2r10 = v_r10 + v_r10;
			v_int16 d = v_pack((v_reinterpret_as_s32(vx_load_expand_q(dst0 + x)) << 6) + v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10), (v_reinterpret_as_s32(vx_load_expand_q(dst1 + x)) << 6) + ((v_r10 + v_r20) << 2));
			*(unaligned_int*)(dst0 + x) = v_reinterpret_as_s32(v_rshr_pack_u<6>(d, vx_setzero_s16())).get0();
			*(unaligned_int*)(dst1 + x) = v_reinterpret_as_s32(v_rshr_pack_u<6>(v_combine_high(d, d), vx_setzero_s16())).get0();
		}
		vx_cleanup();

		return x;
	}

	template<class CastOp>
	struct GauDownInvoker
	{
		GauDownInvoker(const Mat& src, const Mat& dst, int borderType, int** tabR, int** tabM, int** tabL)
		{
			_src = &src;
			_dst = &dst;
			_borderType = borderType;
			_tabR = tabR;
			_tabM = tabM;
			_tabL = tabL;
		}

		void operator()(const Range& range) const;

		int** _tabR;
		int** _tabM;
		int** _tabL;
		const Mat* _src;
		const Mat* _dst;
		int _borderType;
	};


	template<class CastOp>
	void GauDownInvoker<CastOp>::operator()(const Range & range) const
	{
		const int PD_SZ = 5;
		typedef typename CastOp::type1 WT;
		typedef typename CastOp::rtype T;
		Size ssize = _src->size(), dsize = _dst->size();
		int cn = _src->channels();
		int bufstep = (int)alignSize(dsize.width * cn, 16);
		AutoBuffer<WT> _buf(bufstep * PD_SZ + 16);
		WT * buf = alignPtr((WT*)_buf.data(), 16);
		WT * rows[PD_SZ];
		CastOp castOp;

		int sy0 = -PD_SZ / 2, sy = range.start * 2 + sy0, width0 = std::min((ssize.width - PD_SZ / 2 - 1) / 2 + 1, dsize.width);

		ssize.width *= cn;
		dsize.width *= cn;
		width0 *= cn;

		for (int y = range.start; y < range.end; y++)
		{
			T* dst = (T*)_dst->ptr<T>(y);
			WT* row0, * row1, * row2, * row3, * row4;

			int sy_limit = y * 2 + 2;
			for (; sy <= sy_limit; sy++)
			{
				WT* row = buf + ((sy - sy0) % PD_SZ) * bufstep;
				int _sy = borderInterpolate(sy, ssize.height, _borderType);
				const T * src = _src->ptr<T>(_sy);

				do
				{
					int x = 0;
					const int* tabL = *_tabL;
					for (; x < cn; x++)
					{
						row[x] = src[tabL[x + cn * 2]] * 6 + (src[tabL[x + cn]] + src[tabL[x + cn * 3]]) * 4 +
							src[tabL[x]] + src[tabL[x + cn * 4]];
					}

					if (x == dsize.width)
						break;

					if (cn == 1)
					{
						x += GauDownVecH<T, WT, 1>(src + x * 2 - 2, row + x, width0 - x);
						for (; x < width0; x++)
							row[x] = src[x * 2] * 6 + (src[x * 2 - 1] + src[x * 2 + 1]) * 4 +
							src[x * 2 - 2] + src[x * 2 + 2];
					}
					else if (cn == 2)
					{
						x += GauDownVecH<T, WT, 2>(src + x * 2 - 4, row + x, width0 - x);
						for (; x < width0; x += 2)
						{
							const T* s = src + x * 2;
							WT t0 = s[0] * 6 + (s[-2] + s[2]) * 4 + s[-4] + s[4];
							WT t1 = s[1] * 6 + (s[-1] + s[3]) * 4 + s[-3] + s[5];
							row[x] = t0; row[x + 1] = t1;
						}
					}
					else if (cn == 3)
					{
						x += GauDownVecH<T, WT, 3>(src + x * 2 - 6, row + x, width0 - x);
						for (; x < width0; x += 3)
						{
							const T* s = src + x * 2;
							WT t0 = s[0] * 6 + (s[-3] + s[3]) * 4 + s[-6] + s[6];
							WT t1 = s[1] * 6 + (s[-2] + s[4]) * 4 + s[-5] + s[7];
							WT t2 = s[2] * 6 + (s[-1] + s[5]) * 4 + s[-4] + s[8];
							row[x] = t0; row[x + 1] = t1; row[x + 2] = t2;
						}
					}
					else if (cn == 4)
					{
						x += GauDownVecH<T, WT, 4>(src + x * 2 - 8, row + x, width0 - x);
						for (; x < width0; x += 4)
						{
							const T* s = src + x * 2;
							WT t0 = s[0] * 6 + (s[-4] + s[4]) * 4 + s[-8] + s[8];
							WT t1 = s[1] * 6 + (s[-3] + s[5]) * 4 + s[-7] + s[9];
							row[x] = t0; row[x + 1] = t1;
							t0 = s[2] * 6 + (s[-2] + s[6]) * 4 + s[-6] + s[10];
							t1 = s[3] * 6 + (s[-1] + s[7]) * 4 + s[-5] + s[11];
							row[x + 2] = t0; row[x + 3] = t1;
						}
					}
					else
					{
						for (; x < width0; x++)
						{
							int sx = (*_tabM)[x];
							row[x] = src[sx] * 6 + (src[sx - cn] + src[sx + cn]) * 4 +
								src[sx - cn * 2] + src[sx + cn * 2];
						}
					}

					const int* tabR = *_tabR;
					for (int x_ = 0; x < dsize.width; x++, x_++)
					{
						row[x] = src[tabR[x_ + cn * 2]] * 6 + (src[tabR[x_ + cn]] + src[tabR[x_ + cn * 3]]) * 4 +
							src[tabR[x_]] + src[tabR[x_ + cn * 4]];
					}
				} while (0);
			}

			for (int k = 0; k < PD_SZ; k++)
				rows[k] = buf + ((y * 2 - PD_SZ / 2 + k - sy0) % PD_SZ) * bufstep;
			row0 = rows[0]; row1 = rows[1]; row2 = rows[2]; row3 = rows[3]; row4 = rows[4];

			int x = GauDownVecV<WT, T>(rows, dst, dsize.width);
			for (; x < dsize.width; x++)
				dst[x] = castOp(row2[x] * 6 + (row1[x] + row3[x]) * 4 + row0[x] + row4[x]);
		}
	}



	template<class CastOp> void
		gauDown_(const Mat & _src, Mat & _dst, int borderType)
	{
		const int PD_SZ = 5;
		CV_Assert(!_src.empty());
		Size ssize = _src.size(), dsize = _dst.size();
		int cn = _src.channels();

		int tabL[4 * (PD_SZ + 2)], tabR[4 * (PD_SZ + 2)];
		AutoBuffer<int> _tabM(dsize.width * cn);
		int* tabM = _tabM.data();

		CV_Assert(ssize.width > 0 && ssize.height > 0 &&
			std::abs(dsize.width * 2 - ssize.width) <= 2 &&
			std::abs(dsize.height * 2 - ssize.height) <= 2);
		int width0 = std::min((ssize.width - PD_SZ / 2 - 1) / 2 + 1, dsize.width);

		for (int x = 0; x <= PD_SZ + 1; x++)
		{
			int sx0 = borderInterpolate(x - PD_SZ / 2, ssize.width, borderType) * cn;
			int sx1 = borderInterpolate(x + width0 * 2 - PD_SZ / 2, ssize.width, borderType) * cn;
			for (int k = 0; k < cn; k++)
			{
				tabL[x * cn + k] = sx0 + k;
				tabR[x * cn + k] = sx1 + k;
			}
		}

		for (int x = 0; x < dsize.width * cn; x++)
			tabM[x] = (x / cn) * 2 * cn + x % cn;

		int* tabLPtr = tabL;
		int* tabRPtr = tabR;

		GauDownInvoker<CastOp>(_src, _dst, borderType, &tabRPtr, &tabM, &tabLPtr)(Range(0, dsize.height));
	}



	template<class CastOp> void
		gauUp_(const Mat & _src, Mat & _dst, int)
	{
		const int PU_SZ = 3;
		typedef typename CastOp::type1 WT;
		typedef typename CastOp::rtype T;

		Size ssize = _src.size(), dsize = _dst.size();
		int cn = _src.channels();
		int bufstep = (int)alignSize((dsize.width + 1) * cn, 16);
		AutoBuffer<WT> _buf(bufstep * PU_SZ + 16);
		WT * buf = alignPtr((WT*)_buf.data(), 16);
		AutoBuffer<int> _dtab(ssize.width * cn);
		int* dtab = _dtab.data();
		WT * rows[PU_SZ];
		T * dsts[2];
		CastOp castOp;

		CV_Assert(abs(dsize.width - ssize.width * 2) == dsize.width % 2 &&
			abs(dsize.height - ssize.height * 2) == dsize.height % 2);
		int k, x, sy0 = -PU_SZ / 2, sy = sy0;

		ssize.width *= cn;
		dsize.width *= cn;

		for (x = 0; x < ssize.width; x++)
			dtab[x] = (x / cn) * 2 * cn + x % cn;

		for (int y = 0; y < ssize.height; y++)
		{
			T* dst0 = _dst.ptr<T>(y * 2);
			T* dst1 = _dst.ptr<T>(gxr_min(y * 2 + 1, dsize.height - 1));
			WT * row0, *row1, *row2;

			for (; sy <= y + 1; sy++)
			{
				WT* row = buf + ((sy - sy0) % PU_SZ) * bufstep;
				int _sy = borderInterpolate(sy * 2, ssize.height * 2, BORDER_REFLECT_101) / 2;
				const T * src = _src.ptr<T>(_sy);

				if (ssize.width == cn)
				{
					for (x = 0; x < cn; x++)
						row[x] = row[x + cn] = src[x] * 8;
					continue;
				}

				for (x = 0; x < cn; x++)
				{
					int dx = dtab[x];
					WT t0 = src[x] * 6 + src[x + cn] * 2;
					WT t1 = (src[x] + src[x + cn]) * 4;
					row[dx] = t0; row[dx + cn] = t1;
					dx = dtab[ssize.width - cn + x];
					int sx = ssize.width - cn + x;
					t0 = src[sx - cn] + src[sx] * 7;
					t1 = src[sx] * 8;
					row[dx] = t0; row[dx + cn] = t1;

					if (dsize.width > ssize.width * 2)
					{
						row[(_dst.cols - 1) + x] = row[dx + cn];
					}
				}

				for (x = cn; x < ssize.width - cn; x++)
				{
					int dx = dtab[x];
					WT t0 = src[x - cn] + src[x] * 6 + src[x + cn];
					WT t1 = (src[x] + src[x + cn]) * 4;
					row[dx] = t0;
					row[dx + cn] = t1;
				}
			}

			for (k = 0; k < PU_SZ; k++)
				rows[k] = buf + ((y - PU_SZ / 2 + k - sy0) % PU_SZ) * bufstep;
			row0 = rows[0]; row1 = rows[1]; row2 = rows[2];
			dsts[0] = dst0; dsts[1] = dst1;

			x = GauUpVecV<WT, T>(rows, dsts, dsize.width);
			for (; x < dsize.width; x++)
			{
				T t1 = castOp((row1[x] + row2[x]) * 4);
				T t0 = castOp(row0[x] + row1[x] * 6 + row2[x]);
				dst1[x] = t1; dst0[x] = t0;
			}
		}

		if (dsize.height > ssize.height * 2)
		{
			T* dst0 = _dst.ptr<T>(ssize.height * 2 - 2);
			T * dst2 = _dst.ptr<T>(ssize.height * 2);

			for (x = 0; x < dsize.width; x++)
			{
				dst2[x] = dst0[x];
			}
		}
	}



	template<class CastOp> void
		diffgauUp_(const Mat& _src0, const Mat& _src1, Mat& _dst, int)
	{
		const int PU_SZ = 3;
		typedef typename CastOp::type1 WT;
		typedef typename CastOp::rtype T;

		Size ssize = _src0.size(), dsize = _dst.size();
		int cn = _src0.channels();
		int bufstep = (int)alignSize((dsize.width + 1) * cn, 16);
		AutoBuffer<WT> _buf(bufstep * PU_SZ + 16);
		WT * buf = alignPtr((WT*)_buf.data(), 16);
		AutoBuffer<int> _dtab(ssize.width * cn);
		int* dtab = _dtab.data();
		WT * rows[PU_SZ];
		T * dsts[2];

		CV_Assert(abs(dsize.width - ssize.width * 2) == dsize.width % 2 &&
			abs(dsize.height - ssize.height * 2) == dsize.height % 2);
		int k, x, sy0 = -PU_SZ / 2, sy = sy0;

		ssize.width *= cn;
		dsize.width *= cn;

		for (x = 0; x < ssize.width; x++)
			dtab[x] = (x / cn) * 2 * cn + x % cn;

		for (int y = 0; y < ssize.height; y++)
		{
			T* dst0 = _dst.ptr<T>(y * 2);
			T* dst1 = _dst.ptr<T>(gxr_min(y * 2 + 1, dsize.height - 1));
			WT * row0, *row1, *row2;

			for (; sy <= y + 1; sy++)
			{
				WT* row = buf + ((sy - sy0) % PU_SZ) * bufstep;
				int _sy = borderInterpolate(sy * 2, ssize.height * 2, BORDER_REFLECT_101) / 2;
				const T * src0 = _src0.ptr<T>(_sy);
				const T * src1 = _src1.ptr<T>(_sy);

				if (ssize.width == cn)
				{
					for (x = 0; x < cn; x++)
						row[x] = row[x + cn] = ((src0[x] - src1[x]) << 3);
					continue;
				}

				for (x = 0; x < cn; x++)
				{
					int dx = dtab[x];
					WT tle, t = src0[x] - src1[x], trg = src0[x + cn] - src1[x + cn];
					WT t0 = t * 6 + trg * 2;
					WT t1 = (t + trg) * 4;
					row[dx] = t0; row[dx + cn] = t1;
					dx = dtab[ssize.width - cn + x];
					int sx = ssize.width - cn + x;
					tle = src0[sx - cn] - src1[sx - cn], t = src0[sx] - src1[sx];
					t0 = tle + t * 7;
					t1 = t * 8;
					row[dx] = t0; row[dx + cn] = t1;

					if (dsize.width > ssize.width * 2)
					{
						row[(_dst.cols - 1) + x] = row[dx + cn];
					}
				}

				for (x = cn; x < ssize.width - cn; x++)
				{
					int dx = dtab[x];
					WT tle = src0[x - cn] - src1[x - cn], t = src0[x] - src1[x], trg = src0[x + cn] - src1[x + cn];
					WT t0 = tle + t * 6 + trg;
					WT t1 = (t + trg) << 2;
					row[dx] = t0;
					row[dx + cn] = t1;
				}
			}

			for (k = 0; k < PU_SZ; k++)
				rows[k] = buf + ((y - PU_SZ / 2 + k - sy0) % PU_SZ) * bufstep;
			row0 = rows[0]; row1 = rows[1]; row2 = rows[2];
			dsts[0] = dst0; dsts[1] = dst1;

			x = DiffGauUpVecV<WT, T>(rows, dsts, dsize.width);
			for (; x < dsize.width; x++)
			{
				T t1 = gxr_clamp(((row1[x] + row2[x]) >> 4) + dst1[x], 0, 255);
				T t0 = gxr_clamp(((row0[x] + row1[x] * 6 + row2[x]) >> 6) + dst0[x], 0, 255);
				dst1[x] = t1; dst0[x] = t0;
			}
		}

		if (dsize.height > ssize.height * 2)
		{
			T* dst0 = _dst.ptr<T>(ssize.height * 2 - 2);
			T * dst2 = _dst.ptr<T>(ssize.height * 2);

			for (x = 0; x < dsize.width; x++)
			{
				dst2[x] = dst0[x];
			}
		}
	}
}


namespace gxr
{
	void gauDown(Mat const& src, Mat& dst, Size dsz = Size(), int borderType = BORDER_DEFAULT)
	{
		if (dsz.height == 0 || dsz.width == 0)
		{
			dsz.height = (src.rows + 1) / 2;
			dsz.width = (src.cols + 1) / 2;
		}
		dst.create(dsz.height, dsz.width, src.flags);

		gauDown_< FixPtCast<uchar, 8> >(src, dst, borderType);
	}


	void gauUp(Mat const& src, Mat & dst, Size dsz = Size(), int borderType = BORDER_DEFAULT)
	{
		if (dsz.height == 0 || dsz.width == 0)
		{
			dsz.height = src.rows * 2;
			dsz.width = src.cols * 2;
		}
		dst.create(dsz.height, dsz.width, src.flags);

		gauUp_< FixPtCast<uchar, 6> >(src, dst, borderType);
	}


	void diffgauUp(Mat const& src0, Mat const& src1, Mat& dst, Size dsz = Size(), int borderType = BORDER_DEFAULT)
	{
		if (dsz.height == 0 || dsz.width == 0)
		{
			dsz.height = src0.rows * 2;
			dsz.width = src0.cols * 2;
		}
		dst.create(dsz.height, dsz.width, src0.flags);

		diffgauUp_< FixPtCast<uchar, 6> >(src0, src1, dst, borderType);
	}
}