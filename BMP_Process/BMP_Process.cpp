#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define file_source "test0.bmp"
#define file_copy "test_copy0.bmp"
#define file_upside_down "test_upside_down0.bmp"
#define file_left_to_right "test_left_to_right0.bmp"

#pragma pack(2)
typedef struct  _FILEHEADER{
	unsigned __int16 bf_Type;
	unsigned __int32 bf_FileSize;
	unsigned __int16 bf_Reserved1;
	unsigned __int16 bf_Reserved2;
	unsigned __int32 bf_OffBits;
}FILEHEADER;

typedef struct _INFOHEADER {
	unsigned __int32 bi_InfoSize;
	unsigned __int32 bi_Width;
	unsigned __int32 bi_Height;
	unsigned __int16 bi_Planes;
	unsigned __int16 bi_BitCount;
	unsigned __int32 bi_Compression;
	unsigned __int32 bi_SizeImage;
	unsigned __int32 bi_XpelsPerMeter;
	unsigned __int32 bi_YPelsPerMeter;
	unsigned __int32 bi_ClrUsed;
	unsigned __int32 bi_ClrImportant;
}INFOHEADER;
#pragma pack()

int main() {
	FILE* fptr_source = NULL;
	FILE* fptr_target = NULL;
	FILEHEADER fileheader;
	INFOHEADER infoheader;
	unsigned __int8* palette_1bit_depth = NULL;
	unsigned __int8* pixel_array = NULL;
	unsigned __int8* pixel_array_reverse = NULL;
	unsigned __int8* ptr_pixel_array = NULL;
	unsigned __int8 rowsize = 0;
	unsigned __int32 dataNum = 0;
	unsigned __int8 pad = 0;
	unsigned __int8 temp = 0;
	unsigned __int8 buffer[8] = {0};

	if ((fptr_source = fopen(file_source, "rb")) != NULL) {
		printf("file opened\n");
		/*read*/
		fread(&fileheader, sizeof(FILEHEADER), 1, fptr_source);
		fread(&infoheader, sizeof(INFOHEADER), 1, fptr_source);

		palette_1bit_depth = (unsigned __int8*)malloc(sizeof(unsigned __int8) * 8);
		fread(palette_1bit_depth, sizeof(unsigned __int8), 8, fptr_source);

		rowsize = (infoheader.bi_BitCount * infoheader.bi_Width + 31) / 32 * 4;
		pixel_array = (unsigned char*)malloc(sizeof(unsigned char) * rowsize * infoheader.bi_Height);
		fread(pixel_array, sizeof(unsigned __int8), rowsize * infoheader.bi_Height, fptr_source);

		/*copy*/
		fptr_target = fopen(file_copy, "wb");
		fwrite(&fileheader, sizeof(FILEHEADER), 1, fptr_target);
		fwrite(&infoheader, sizeof(INFOHEADER), 1, fptr_target);
		fwrite(palette_1bit_depth, sizeof(unsigned __int8), 8, fptr_target);
		fwrite(pixel_array, sizeof(unsigned __int8), rowsize * infoheader.bi_Height, fptr_target);
		fclose(fptr_target);
		printf("file is copyed\n");

		/*copy: upside down*/
		fptr_target = fopen(file_upside_down, "wb");
		fwrite(&fileheader, sizeof(FILEHEADER), 1, fptr_target);
		fwrite(&infoheader, sizeof(INFOHEADER), 1, fptr_target);
		fwrite(palette_1bit_depth, sizeof(unsigned __int8), 8, fptr_target);
		ptr_pixel_array = pixel_array + rowsize * infoheader.bi_Height;
		for (unsigned __int8 i = 0; i < infoheader.bi_Height; i++) {
			ptr_pixel_array -= rowsize;
			fwrite(ptr_pixel_array, rowsize, 1, fptr_target);
		}
		fclose(fptr_target);
		printf("file is upsidedowned\n");

		/*left right to right*/
		pixel_array_reverse = (unsigned __int8*)malloc(sizeof(unsigned __int8) * rowsize * infoheader.bi_Height);
		memcpy(pixel_array_reverse, pixel_array, rowsize * infoheader.bi_Height);
		if (infoheader.bi_Width % 32 == 0) {	//寬為4個Byte倍數
			pad = 0;							//位元填充數量為0
			dataNum = infoheader.bi_Width /8;	//有效資料數量
			ptr_pixel_array = pixel_array_reverse;
			for (unsigned __int32 i = 0; i < infoheader.bi_Height; i++) {
				for (unsigned __int32 j = 0; j < dataNum / 2; j++) {		//交換Byte
					temp = *(ptr_pixel_array + j);
					*(ptr_pixel_array + j) = *(ptr_pixel_array + (dataNum - 1 - j));
					*(ptr_pixel_array + (dataNum - 1 - j)) = temp;
				}
				for (unsigned int j = 0; j < dataNum; j++) {				//交換bit
					for (unsigned int k = 0; k < 8; k++) {
						buffer[k] = (*(ptr_pixel_array + j) & 0x01) << (7 - k);
						*(ptr_pixel_array + j) = *(ptr_pixel_array + j) >> 1;
					}
					*(ptr_pixel_array + j) = (buffer[0] | buffer[1] | buffer[2] | buffer[3] | buffer[4] | buffer[5] | buffer[6] | buffer[7]);
				}
				ptr_pixel_array += rowsize;
			}
			fptr_target = fopen(file_left_to_right, "wb");
			fwrite(&fileheader, sizeof(FILEHEADER), 1, fptr_target);
			fwrite(&infoheader, sizeof(INFOHEADER), 1, fptr_target);
			fwrite(palette_1bit_depth, sizeof(unsigned __int8), 8, fptr_target);
			fwrite(pixel_array_reverse, sizeof(unsigned __int8), rowsize * infoheader.bi_Height, fptr_target);
			fclose(fptr_target);
			printf("file left to right\n");
		}
		else{									   //寬不為4個Byte倍數
			pad = (8 - infoheader.bi_Width % 8);   //位元填充數量為 (32-寬%32)
			dataNum = infoheader.bi_Width /8 + 1;  //有效資料數量
			ptr_pixel_array = pixel_array_reverse;

			printf("像素寬度為%d,不為32的倍數", infoheader.bi_Width);
			for (unsigned __int32 i = 0; i < infoheader.bi_Height; i++) {
				for (unsigned __int32 j = 0; j < dataNum / 2; j++) {		//交換Byte
					temp = *(ptr_pixel_array + j);
					*(ptr_pixel_array + j) = *(ptr_pixel_array + (dataNum - 1 - j));
					*(ptr_pixel_array + (dataNum - 1 - j)) = temp;
				}
				for (unsigned int j = 0; j < dataNum; j++) {				//交換bit
					if (j == 0) {	//首筆資料位移pad個位元
						*(ptr_pixel_array + j) = *(ptr_pixel_array + j) >> pad;
					}

					//擷取8-pad個位元資料
					for (unsigned int k = 0; k < 8-pad; k++) {
						buffer[k] = (*(ptr_pixel_array + j) & 0x01) << (7 - k);
						*(ptr_pixel_array + j) = *(ptr_pixel_array + j) >> 1;
					}

					//缺pad個bit，從j+1找
					if (j != dataNum - 1) {	//非最後一筆資料
						for (unsigned int k = 0; k < pad; k++) {
							buffer[8 - pad + k] = (*(ptr_pixel_array + j + 1) & 0x01) << (pad - 1 - k);
							*(ptr_pixel_array + j + 1) = *(ptr_pixel_array + j + 1) >> 1;
						}
					}
					else {	//最後一筆資料，補pad個0
						for (unsigned int k = 0; k < pad; k++) {
							buffer[8 - pad + k] = 0x00 << (pad - 1 - k);
						}
					}
					
					//use "or" to combine 8 bit into a __int8
					*(ptr_pixel_array + j) = (buffer[0] | buffer[1] | buffer[2] | buffer[3] | buffer[4] | buffer[5] | buffer[6] | buffer[7]);
				}
				ptr_pixel_array += rowsize;
			}
			fptr_target = fopen(file_left_to_right, "wb");
			fwrite(&fileheader, sizeof(FILEHEADER), 1, fptr_target);
			fwrite(&infoheader, sizeof(INFOHEADER), 1, fptr_target);
			fwrite(palette_1bit_depth, sizeof(unsigned __int8), 8, fptr_target);
			fwrite(pixel_array_reverse, sizeof(unsigned __int8), rowsize * infoheader.bi_Height, fptr_target);
			fclose(fptr_target);
			printf("file left to right\n");
		}

		free(palette_1bit_depth);
		free(pixel_array);
		free(pixel_array_reverse);
	}
	else {
		printf("file not found\n");
	}

	return 0;
}
