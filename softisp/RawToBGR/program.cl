#define clamp(x, a, b) (min(max((x), (a)), (b)))

__kernel void filter_(__global uchar const* src, __global uchar* dst, int rows, int cols)
{
	int h = get_global_id(0);
	int w = get_global_id(1);
	if (h < rows && w < cols)
	{
		int index = h * cols + w;
		int cur = src[index], sum = 0;
		for (int dh = -3; dh <= 3; dh++)
		{
			__global uchar const* line = src + clamp(h + dh, 0, rows - 1) * cols;
			for (int dw = -3; dw <= 3; dw++)
			{
				if (dh != 0 || dw != 0)
				{
					short dif = line[clamp(w + dw, 0, cols - 1)] - cur;
					short abs_dif = abs(dif);
					short res = clamp((short)(16 - (abs_dif >> 1)), (short)0, abs_dif);
					res = dif > 0 ? res : -res;
					sum += res;
				} 
			}
		}
		dst[index] = clamp(cur + sum / 48, 0, 255);
	}
}

__kernel void filter_n(__global uchar const* src, __global uchar* dst, int rows, int cols)
{
	int h = get_global_id(0);
	if (h < rows)
	{
		for (int w = 0; w < cols; w++)
		{
			int index = h * cols + w;
			int cur = src[index], sum = 0;
			for (int dh = -3; dh <= 3; dh++)
			{
				__global uchar const* line = src + clamp(h + dh, 0, rows - 1) * cols;
				for (int dw = -3; dw <= 3; dw++)
				{
					if (dh != 0 || dw != 0)
					{
						short dif = line[clamp(w + dw, 0, cols - 1)] - cur;
						short abs_dif = abs(dif);
						short res = clamp((short)(16 - (abs_dif >> 1)), (short)0, abs_dif);
						res = dif > 0 ? res : -res;
						sum += res;
					} 
				}
			}
			dst[index] = clamp(cur + sum / 48, 0, 255);
		}
	}
}

__kernel void filter(__global uchar const* src, __global uchar* dst, int rows, int cols)
{
	int h = get_global_id(0);
	if (h < rows)
	{
		for (int w = 0; w < cols; w++)
		{
			int index = h * cols + w;
			int cur = src[index], sum = 0;
			for (int dh = -0; dh <= 0; dh++)
			{
				__global uchar const* line = src + clamp(h + dh, 0, rows - 1) * cols;
				for (int dw = -0; dw <= 0; dw++)
				{
					if (dh != 0 || dw != 0)
					{
						short dif = line[clamp(w + dw, 0, cols - 1)] - cur;
						short abs_dif = abs(dif);
						short res = clamp((short)(16 - (abs_dif >> 1)), (short)0, abs_dif);
						res = dif > 0 ? res : -res;
						sum += res;
					} 
				}
			}
			dst[index] = clamp(cur + sum / 48, 0, 255);
		}
	}
}

