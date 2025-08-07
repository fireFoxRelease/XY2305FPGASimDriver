#include<stdio.h>
#include <windows.h>
#include "HiXY2305DriverPublic.h"
unsigned char flashData[100 * 1024 * 1024];
int main()
{
	HR_DEVICE pHandle;
	int ret = 0;
	TY_CARD_INFO pCardInfo;
	unsigned int flashId;
	unsigned int length = 0;
	unsigned int fileSize = 0;
	char data[4096];
	memset(flashData,0, 100 * 1024 * 1024);
	ret = hiMasterOpenCard(&pHandle, 0);
	if (ret != 0)
	{
		return 0;
	}
	ret = hiMasterGetCardVersion(pHandle, &pCardInfo);
	for (int i = 0; i < 6; i++)
	{
		hiPWMEnable(pHandle, i,1);

		hiPWMSetSpeed(pHandle, i, 0x7D0);

		hiPWMLow(pHandle, i, 0x7D0);

		hiPWMHigh(pHandle, i, 0x7D0);

		hiPWMDied(pHandle, i, 0x7D0);

		hiPWMSel(pHandle, i, 1);
	}
	float value = 0.0;
	for (int i = 0; i < 8; i++)
	{

		//hiAnalogAjustCalEnable(pHandle, i, 1);

		//hiAnalogAjustCalData(pHandle, i, (float)0.7,1);

		hiAnalogOutEnable(pHandle, i, 1);

		hiAnalogOutSel(pHandle, i, 1);

		hiAnalogOutExcSel(pHandle, i, 0);

		hiAnalogOutVpp(pHandle, i,0x32);

		hiAnalogOutFreq(pHandle, i, 0x3E8);
		 
		hiAnalogOutData(pHandle, i, (float)-9.3);
		
		//hiAnalogInEnable(pHandle, i + 1,1);

		//hiAnalogInData(pHandle, i + 1, &value);

	}
	for (int i = 0; i < 8; i++)
	{
		hiAnalogInEnable(pHandle, i, 1);

		//hiAnalogInData(pHandle, i + 1, &value);
	}
	while (1)
	{
		for (int i = 0; i < 8; i++)
		{
			hiAnalogInData(pHandle, i, &value);
			printf("chn: %d   value: %lf\r\n",i, value);
		}
		getchar();
	}



	unsigned int monitor = 0;
	for (int i = 0; i <1; i++)
	{
		 hiIOChnEnable(pHandle, i, 1);

		 hiIOChnDir(pHandle, i, 0);

		// hiIOChnOut(pHandle, i, 1);

		// hiIOChnSetInSamp(pHandle, i, 1);

		// hiIOChnSetInData(pHandle, i, 1);

		// hiIOChnMon(pHandle, i, &monitor);
	}
	hiIOChnEnable(pHandle, 1, 1);

	hiIOChnDir(pHandle, 1, 1);
	while (1)
	{

		hiIOChnOut(pHandle, 1, 1);

		for (int i = 0; i < 1; i++)
		{
			hiIOChnGetInData(pHandle, i, &monitor);

			printf("chn: %d monitor:%d\r\n",i, monitor);
		}

		Sleep(10);

		hiIOChnOut(pHandle, 1, 0);

		for (int i = 0; i < 1; i++)
		{
			hiIOChnGetInData(pHandle, i, &monitor);

			printf("chn: %d monitor:%d\r\n", i, monitor);
		}

	}

	hiLoadBit(pHandle, "D:/bit/fpga_sim_top.bin");
	hiWriteDMA(pHandle, data, 512);
	hiReadDMA(pHandle, data, 512);
	FILE* fp = fopen("D:/0420/XY2305FPGASim/lib/fpga_sim_top.bin", "ab+");
	if (fp == NULL)
	{
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET); 
	if (fileSize % 512 == 0)
	{
		length = fileSize;
	}
	else
	{
		length = (fileSize / 512 + 1) * 512;
	}
	hiLoadFlash(pHandle, "D:/0420/XY2305FPGASim/lib/fpga_sim_top.bin");
	//hiReadFlash(pHandle, flashData, 0xAE9E00);
	hiReadFlash(pHandle, flashData, 0xAE9E00);
	fclose(fp);
	/*for (int i = 0; i < 0xAE9E00; i++)
	{
		flashData[i] = i;
	}*/
	FILE* fpFlash = fopen("text_1.bin", "ab+");
	fwrite(flashData, 1, 0xAE9E00, fpFlash);
	//fwrite(flashData,1, 0xAE9E00, fpFlash);
	fflush(fpFlash);
	fclose(fpFlash);
	//hiLoadFlash(pHandle, "D:/0420/XY2305FPGASim/lib/fpga_sim_top.bin");
	hiGetFlashId(pHandle, &flashId);
	memset(data, 0xFF, 4096);
	ret = hiMasterGetCardVersion(pHandle, &pCardInfo);
	if (ret != 0)
	{
		return 0;
	}
	hiLoadBit(pHandle, "D:/0420/XY2305FPGASim/lib/fpga_sim_top.bin");
	ret =  hiGetFlashId(pHandle, &flashId);

	ret = hiMasterCloseCard(pHandle);
	if (ret != 0)
	{
		return 0;
	}
	return 0;
}