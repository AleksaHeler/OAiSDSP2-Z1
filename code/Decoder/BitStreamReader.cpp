#pragma once
#include "BitStreamReader.h"
#include "HuffmanDec.h"
#include "QString"
#include "QDebug"

#define PI 3.14159265359

// Read and check header
bool BitStreamReader::readHeader()
{
	char* header = new char[HEADER_SIZE];

	// Read header
	readData((char*)header, HEADER_SIZE);
	
	// Check header content
	for (int i = 0; i < HEADER_SIZE; i++)
	{
		if (header[i] != FILE_HEADER[i])
			return false;
	}

	delete[] header;

	return true;
}

// Read image size
bool BitStreamReader::readImageInfo(int& width, int& height)
{
	width = readShort();
	height = readShort();

	return true;
}

uchar BitStreamReader::readByte()
{
	uchar data;
	inputFile.getChar((char*)&data);
	return data;
}

ushort BitStreamReader::readShort()
{
	ushort data;
	inputFile.getChar(((char*)&data) + 1);
	inputFile.getChar((char*)&data);

	return data;
}

int BitStreamReader::readData(char data[], int size)
{
	return inputFile.read((char*)data, size);
}

bool BitStreamReader::skip(int bytes)
{
	inputFile.seek(inputFile.pos() + bytes);
	return true;
}

void BitStreamReader::YUV420toRGB(const uchar Y_buff[], const char U_buff[], const char V_buff[], int x, int y, uchar rgbImg[])
{
	double R, G, B;
	double Y, U, V;

	for (int j = 0; j<y; j += 2)
	{
		for (int i = 0; i<x; i += 2)
		{
			U = U_buff[j / 2 * x / 2 + i / 2];
			V = V_buff[j / 2 * x / 2 + i / 2];

			Y = Y_buff[j*x + i];

			R = Y + 1.13983*V;
			G = Y - 0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;

			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G< 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;

			rgbImg[j * 3 * x + i * 3] = R;
			rgbImg[j * 3 * x + i * 3 + 1] = G;
			rgbImg[j * 3 * x + i * 3 + 2] = B;

			Y = Y_buff[j*x + (i + 1)];

			R = Y + 1.13983*V;
			G = Y - 0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;

			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G< 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;

			rgbImg[j * 3 * x + (i + 1) * 3] = R;
			rgbImg[j * 3 * x + (i + 1) * 3 + 1] = G;
			rgbImg[j * 3 * x + (i + 1) * 3 + 2] = B;

			Y = Y_buff[(j + 1)*x + i];

			R = Y + 1.13983*V;
			G = Y - 0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;

			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G < 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;

			rgbImg[(j + 1) * 3 * x + i * 3] = R;
			rgbImg[(j + 1) * 3 * x + i * 3 + 1] = G;
			rgbImg[(j + 1) * 3 * x + i * 3 + 2] = B;

			Y = Y_buff[(j + 1)*x + (i + 1)];

			R = Y + 1.13983*V;
			G = Y - 0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;

			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G< 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;

			rgbImg[(j + 1) * 3 * x + (i + 1) * 3] = R;
			rgbImg[(j + 1) * 3 * x + (i + 1) * 3 + 1] = G;
			rgbImg[(j + 1) * 3 * x + (i + 1) * 3 + 2] = B;
		}
	}
}

uchar BitStreamReader::LuminanceQuantizationMatrix[64] =
{
	2, 2, 2, 2, 3, 5, 7, 8,
	2, 2, 2, 3, 4, 8, 8, 7,
	2, 2, 2, 3, 5, 8, 9, 7,
	2, 3, 3, 4, 7, 11, 10, 8,
	3, 3, 5, 7, 9, 14, 13, 10,
	3, 5, 7, 8, 11, 13, 15, 12,
	7, 8, 10, 11, 13, 16, 15, 13,
	9, 12, 12, 13, 29, 13, 13, 13
};

uchar BitStreamReader::ChrominanceQuantizationMatrix[64] =
{
	2, 2, 3, 3, 7, 7, 7, 7,
	2, 2, 2, 5, 7, 7, 7, 7,
	2, 2, 4, 7, 7, 7, 7, 7,
	3, 5, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7
};


void BitStreamReader::GenerateDCTmatrix(double* DCTKernel, int order)
{
	int i, j;
	int N;
	N = order;
	double alpha;
	double denominator;
	for (j = 0; j <= N - 1; j++)
	{
		DCTKernel[0, j] = sqrt(1 / (double)N);
	}
	alpha = sqrt(2 / (double)N);
	denominator = (double)2 * N;
	for (j = 0; j <= N - 1; j++)
		for (i = 1; i <= N - 1; i++)
		{
			DCTKernel[i*N + j] = alpha * cos(((2 * j + 1) * i * PI) / denominator);
		}
}


void BitStreamReader::IDCT(const short input[], uchar output[], int N, double* DCTKernel)
{
	double* temp = new double[N*N];
	double* DCTCoefficients = new double[N*N];

	double sum;
	for (int i = 0; i <= N - 1; i++)
	{
		for (int j = 0; j <= N - 1; j++)
		{
			sum = 0;
			for (int k = 0; k <= N - 1; k++)
			{
				sum = sum + DCTKernel[k*N + i] * (input[k*N + j]);
			}
			temp[i*N + j] = sum;
		}
	}

	for (int i = 0; i <= N - 1; i++)
	{
		for (int j = 0; j <= N - 1; j++)
		{
			sum = 0;
			for (int k = 0; k <= N - 1; k++)
			{
				sum = sum + temp[i*N + k] * DCTKernel[k*N + j];
			}
			DCTCoefficients[i*N + j] = sum;
		}
	}

	for (int i = 0; i < N*N; i++)
	{
		double x = floor(DCTCoefficients[i] + 0.5) + 128;
		output[i] = (x>255) ? 255 : ((x<0) ? 0 : x);
	}

	delete[] temp;
	delete[] DCTCoefficients;

	return;
}

void BitStreamReader::IDCT(const short input[], char output[], int N, double* DCTKernel)
{
	double* temp = new double[N*N];
	double* DCTCoefficients = new double[N*N];

	double sum;
	for (int i = 0; i <= N - 1; i++)
	{
		for (int j = 0; j <= N - 1; j++)
		{
			sum = 0;
			for (int k = 0; k <= N - 1; k++)
			{
				sum = sum + DCTKernel[k*N + i] * (input[k*N + j]);
			}
			temp[i*N + j] = sum;
		}
	}

	for (int i = 0; i <= N - 1; i++)
	{
		for (int j = 0; j <= N - 1; j++)
		{
			sum = 0;
			for (int k = 0; k <= N - 1; k++)
			{
				sum = sum + temp[i*N + k] * DCTKernel[k*N + j];
			}
			DCTCoefficients[i*N + j] = sum;
		}
	}

	for (int i = 0; i < N*N; i++)
	{
		double x = floor(DCTCoefficients[i] + 0.5);
		if (x > 127)
            output[i] = 127;
        else if (x < -128)
            output[i] = -128;
        else
            output[i] = x;
	}

	delete[] temp;
	delete[] DCTCoefficients;

	return;
}

void BitStreamReader::doIDCTDecompression(uchar input[], short* short_buff,  int xSize, int ySize, int N, uchar* input2, int xSize2, int ySize2)
{
	/* Create NxN buffer for one input block */
	uchar* inBlock = new uchar[N*N];

	/* Generate DCT kernel */
	double* DCTKernel = new double[N*N];
	GenerateDCTmatrix(DCTKernel, N);

	int cnt = 0;
	for (int y = 0; y < ySize2 / N; y++)
	{
		for (int x = 0; x < xSize2 / N; x++)
		{

			performInverseDCTQuantization(&short_buff[64 * cnt], 0);

			IDCT(&short_buff[64*cnt], inBlock, N, DCTKernel);

			/* Write output values to output image */
			for (int j = 0; j<N; j++)
			{
				for (int i = 0; i<N; i++)
				{
					input2[(N*y + j)*(xSize2)+N*x + i] = inBlock[j*N + i];
				}
			}

			cnt++;
		}
	}

	cropImage(input2, xSize2, ySize2, input, xSize, ySize);

	/* Delete used memory buffers coefficients */
	delete[] inBlock;
	delete[] DCTKernel;
}

void BitStreamReader::doIDCTDecompression(char input[], short* short_buff, int xSize, int ySize, int N, char* input2, int xSize2, int ySize2)
{
	/* Create NxN buffer for one input block */
	char* inBlock = new char[N*N];

	/* Generate DCT kernel */
	double* DCTKernel = new double[N*N];
	GenerateDCTmatrix(DCTKernel, N);

	int cnt = 0;
	for (int y = 0; y < ySize2 / N; y++)
	{
		for (int x = 0; x < xSize2 / N; x++)
		{

			performInverseDCTQuantization(&short_buff[64 * cnt], 1);

			IDCT(&short_buff[64 * cnt], inBlock, N, DCTKernel);

			/* Write output values to output image */
			for (int j = 0; j<N; j++)
			{
				for (int i = 0; i<N; i++)
				{
					input2[(N*y + j)*(xSize2)+N*x + i] = inBlock[j*N + i];
				}
			}

			cnt++;
		}
	}

	cropImage(input2, xSize2, ySize2, input, xSize, ySize);

	/* Delete used memory buffers coefficients */
	delete[] inBlock;
	delete[] DCTKernel;
}

void BitStreamReader::performInverseDCTQuantization(short* dctCoeffs, int quantizationMatrix)
{
	uchar* matrix;
	if (quantizationMatrix == 0)
		matrix = LuminanceQuantizationMatrix;
	else
		matrix = ChrominanceQuantizationMatrix;

	const int N = 8;
	for (int j = 0; j<N; j++)
	{
		for (int i = 0; i<N; i++)
		{
			dctCoeffs[j*N + i] = floor(dctCoeffs[j*N + i] * (matrix[j*N + i]) + 0.5);
		}
	}
}

void BitStreamReader::zeroRunLengthDecode(std::vector<short>& v, short* output, int xSize, int ySize)
{
	int j;
	int i = 0;
	int k = 0;
	while(v[i] != 0 || v[i + 1] != 0) //eob
	{
		for (j = 0; j < v[i]; j++)
		{
			output[k++] = 0;
		}

		output[k++] = v[i + 1];

		i += 2;
	}

	for (k; k < xSize *ySize; k++)
	{
		output[k] = 0;
	}
}


void BitStreamReader::extendBorders(uchar* input, int xSize, int ySize, int N, uchar** p_output, int* newXSize, int* newYSize)
{
	int deltaX = N - xSize%N;
	int deltaY = N - ySize%N;

	*newXSize = xSize + deltaX;
	*newYSize = ySize + deltaY;

	uchar* output = new uchar[(xSize + deltaX)*(ySize + deltaY)];

	for (int i = 0; i<ySize; i++)
	{
		memcpy(&output[i*(xSize + deltaX)], &input[i*(xSize)], xSize);
		if (deltaX != 0)
		{
			memset(&output[i*(xSize + deltaX) + xSize], output[i*(xSize + deltaX) + xSize - 1], deltaX);
		}
	}

	for (int i = 0; i<deltaY; i++)
	{
		memcpy(&output[(i + ySize)*(xSize + deltaX)], &output[(ySize)*(xSize + deltaX)], xSize + deltaX);
	}

	*p_output = output;
}

void BitStreamReader::extendBorders(char* input, int xSize, int ySize, int N, char** p_output, int* newXSize, int* newYSize)
{
	int deltaX = N - xSize%N;
	int deltaY = N - ySize%N;

	*newXSize = xSize + deltaX;
	*newYSize = ySize + deltaY;

	char* output = new char[(xSize + deltaX)*(ySize + deltaY)];

	for (int i = 0; i<ySize; i++)
	{
		memcpy(&output[i*(xSize + deltaX)], &input[i*(xSize)], xSize);
		if (deltaX != 0)
		{
			memset(&output[i*(xSize + deltaX) + xSize], output[i*(xSize + deltaX) + xSize - 1], deltaX);
		}
	}

	for (int i = 0; i<deltaY; i++)
	{
		memcpy(&output[(i + ySize)*(xSize + deltaX)], &output[(ySize)*(xSize + deltaX)], xSize + deltaX);
	}

	*p_output = output;
}

void BitStreamReader::cropImage(uchar* input, int xSize, int ySize, uchar* output, int newXSize, int newYSize)
{
	for (int i = 0; i<newYSize; i++)
	{
		memcpy(&output[i*(newXSize)], &input[i*(xSize)], newXSize);
	}
}

void BitStreamReader::cropImage(char* input, int xSize, int ySize, char* output, int newXSize, int newYSize)
{
	for (int i = 0; i<newYSize; i++)
	{
		memcpy(&output[i*(newXSize)], &input[i*(xSize)], newXSize);
	}
}





// Decode image
// Expcted output is image in RGB888 format
bool BitStreamReader::decode(uchar* &output, int &xSize, int &ySize)
{
	// Read and check header
	if (!readHeader())
	{
		qCritical() << "ERROR::File Header corrupted.";
		return false;
	}

	// Get image size
	readImageInfo(xSize, ySize);

	// Create output image buffer
	output = new uchar[xSize*ySize*3];
	uchar* Y_buff = new uchar[xSize*ySize];
	char* U_buff = new char[xSize*ySize / 4];
	char* V_buff = new char[xSize*ySize / 4];

	/* Extend image borders */
	int xSize2, ySize2;
	uchar* input2;
	extendBorders(Y_buff, xSize, ySize, 8, &input2, &xSize2, &ySize2);
	short* dct_coeffs = new short[xSize2 * ySize2];

	int xSize3, ySize3;
	int xSize4, ySize4;
	char* input3;
	char* input4;
	extendBorders(U_buff, (int)(xSize/2), (int)(ySize/2), 8, &input3, &xSize3, &ySize3);
	extendBorders(V_buff, (int)(xSize/2), (int)(ySize/2), 8, &input4, &xSize4, &ySize4);
	short* dct_coeffs2 = new short[xSize3 * ySize3];
	short* dct_coeffs3 = new short[xSize4 * ySize4];


	//decoding huffman data
	std::vector<short> v_Y, v_U, v_V;
	bool ret_val = decodeHuffmanData(inputFile, v_Y, v_U, v_V);
	if (ret_val == false)
	{
		qCritical() << "Error in Huffman decoding" << endl;
		goto cleanup;
	}

	zeroRunLengthDecode(v_Y, dct_coeffs, xSize2, ySize2);
	zeroRunLengthDecode(v_U, dct_coeffs2, xSize3, ySize3);
	zeroRunLengthDecode(v_V, dct_coeffs3, xSize4, ySize4);

	doIDCTDecompression(Y_buff, dct_coeffs, xSize, ySize, 8, input2, xSize2, ySize2);
	doIDCTDecompression(U_buff, dct_coeffs2, (int)(xSize / 2), (int)(ySize / 2), 8, input3, xSize3, ySize3);
	doIDCTDecompression(V_buff, dct_coeffs3, (int)(xSize / 2), (int)(ySize / 2), 8, input4, xSize4, ySize4);

	YUV420toRGB(Y_buff, U_buff, V_buff, xSize, ySize, output);

cleanup:
	delete[] dct_coeffs;
	delete[] dct_coeffs2;
	delete[] dct_coeffs3;

	delete[] input2;
	delete[] input3;
	delete[] input4;

	delete[] Y_buff;
	delete[] U_buff;
	delete[] V_buff;

	return ret_val;
}