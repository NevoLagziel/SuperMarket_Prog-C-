#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Product.h"
#include "General.h"
#include "fileHelper.h"


#define MIN_DIG 3
#define MAX_DIG 5

void	initProduct(Product* pProduct)
{
	initProductNoBarcode(pProduct);
	getBorcdeCode(pProduct->barcode);
}

void	initProductNoBarcode(Product* pProduct)
{
	initProductName(pProduct);
	pProduct->type = getProductType();
	pProduct->price = getPositiveFloat("Enter product price\t");
	pProduct->count = getPositiveInt("Enter product number of items\t");
}

void initProductName(Product* pProduct)
{
	do {
		printf("enter product name up to %d chars\n", NAME_LENGTH );
		myGets(pProduct->name, sizeof(pProduct->name),stdin);
	} while (checkEmptyString(pProduct->name));
}

void	printProduct(const Product* pProduct)
{
	printf("%-20s %-10s\t", pProduct->name, pProduct->barcode);
	printf("%-20s %5.2f %10d\n", typeStr[pProduct->type], pProduct->price, pProduct->count);
}

int		saveProductToFile(const Product* pProduct, FILE* fp)
{
	if (fwrite(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error saving product to file\n");
		return 0;
	}
	return 1;
}

int		loadProductFromFile(Product* pProduct, FILE* fp)
{
	if (fread(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error reading product from file\n");
		return 0;
	}
	return 1;
}

int		saveProductToCompressedFile(const Product* pProd, FILE* fp)
{
	int nameLen = (int)strlen(pProd->name);
	if (!compressProductToBytes(pProd, nameLen, fp))
		return 0;
	if (fwrite(pProd->name, sizeof(char), nameLen, fp) != nameLen)
		return 0;
	if (!compressCountPriceToBytes(pProd, fp))
		return 0;
	return 1;
}

int		loadProductFromCompressedFile(Product* pProd, FILE* fp)
{
	BYTE data[6] = { 0 };
	BYTE priceData[3] = { 0 };

	if (fread(data, sizeof(BYTE), 6, fp) != 6)
		return 0;
	int nameLen = changeProductFromBytes(data, pProd);

	if (fread(pProd->name, sizeof(char), nameLen, fp) != nameLen)
		return 0;
	pProd->name[nameLen] = '\0';

	if (fread(priceData, sizeof(BYTE), 3, fp) != 3)
		return 0;

	changePriceDataFromBytes(priceData, pProd);
	return 1;
}

void getBorcdeCode(char* code)
{
	char temp[MAX_STR_LEN];
	char msg[MAX_STR_LEN];
	sprintf(msg,"Code should be of %d length exactly\n"
				"UPPER CASE letter and digits\n"
				"Must have %d to %d digits\n"
				"First and last chars must be UPPER CASE letter\n"
				"For example A12B40C\n",
				BARCODE_LENGTH, MIN_DIG, MAX_DIG);
	int ok = 1;
	int digCount = 0;
	do {
		ok = 1;
		digCount = 0;
		printf("Enter product barcode "); 
		getsStrFixSize(temp, MAX_STR_LEN, msg);
		if (strlen(temp) != BARCODE_LENGTH)
		{
			puts(msg);
			ok = 0;
		}
		else {
			//check and first upper letters
			if(!isupper(temp[0]) || !isupper(temp[BARCODE_LENGTH-1]))
			{
				puts("First and last must be upper case letters\n");
				ok = 0;
			} else {
				for (int i = 1; i < BARCODE_LENGTH - 1; i++)
				{
					if (!isupper(temp[i]) && !isdigit(temp[i]))
					{
						puts("Only upper letters and digits\n");
						ok = 0;
						break;
					}
					if (isdigit(temp[i]))
						digCount++;
				}
				if (digCount < MIN_DIG || digCount > MAX_DIG)
				{
					puts("Incorrect number of digits\n");
					ok = 0;
				}
			}
		}
		
	} while (!ok);

	strcpy(code, temp);
}


eProductType getProductType()
{
	int option;
	printf("\n\n");
	do {
		printf("Please enter one of the following types\n");
		for (int i = 0; i < eNofProductType; i++)
			printf("%d for %s\n", i, typeStr[i]);
		scanf("%d", &option);
	} while (option < 0 || option >= eNofProductType);
	getchar();
	return (eProductType)option;
}

const char* getProductTypeStr(eProductType type)
{
	if (type < 0 || type >= eNofProductType)
		return NULL;
	return typeStr[type];
}

int		isProduct(const Product* pProduct, const char* barcode)
{
	if (strcmp(pProduct->barcode, barcode) == 0)
		return 1;
	return 0;
}

int		compareProductByBarcode(const void* var1, const void* var2)
{
	const Product* pProd1 = (const Product*)var1;
	const Product* pProd2 = (const Product*)var2;

	return strcmp(pProd1->barcode, pProd2->barcode);
}


void	updateProductCount(Product* pProduct)
{
	int count;
	do {
		printf("How many items to add to stock?");
		scanf("%d", &count);
	} while (count < 1);
	pProduct->count += count;
}


void	freeProduct(Product* pProduct)
{
	//nothing to free!!!!
}

int* changeBarcodeToNumbers(const char* barcode)
{
	int bar[BARCODE_LENGTH];
	for (int i = 0; i < BARCODE_LENGTH; i++)
	{
		bar[i] = convertLetterToNum(barcode[i]);
	}
	return bar;
}

void changeBarcodeFromNumbers(Product* pProd, int* barcode)
{
	for (int i = 0; i < BARCODE_LENGTH; i++)
	{
		pProd->barcode[i] = convertNumToLetter(barcode[i]);
	}
	pProd->barcode[BARCODE_LENGTH] = '\0';
}

int compressProductToBytes(const Product* pProd, int nameLen, FILE* fp)
{
	BYTE byte[6] = { 0 };
	int* num = changeBarcodeToNumbers(pProd->barcode);

	byte[0] = (num[0] << 2) | (num[1] >> 4);
	byte[1] = (num[1] << 4) | (num[2] >> 2);
	byte[2] = (num[2] << 6) | (num[3]);
	byte[3] = (num[4] << 2) | (num[5] >> 4);
	byte[4] = (num[5] << 4) | (num[6] >> 2);
	byte[5] = (num[6] << 6) | (nameLen << 2) | (pProd->type);

	if (fwrite(byte, sizeof(BYTE), 6, fp) != 6)
		return 0;
	return 1;
}

int compressCountPriceToBytes(const Product* pProd, FILE* fp)
{
	int agorot = (int)(pProd->price * 100) % 100;
	int shkalim = (int)pProd->price;
	BYTE priceData[3] = { 0 };
	priceData[0] = (pProd->count);
	priceData[1] = (agorot << 1) | (shkalim >> 8);
	priceData[2] = (shkalim);

	if (fwrite(priceData, sizeof(BYTE), 3, fp) != 3)
		return 0;
	return 1;
}

int		changeProductFromBytes(BYTE* data, Product* pProd)
{
	int barcode[BARCODE_LENGTH];
	int nameLen = 0;
	barcode[0] = ((data[0] >> 2) & 0x3f);
	barcode[1] = ((data[0] & 0x3) << 4) | ((data[1] >> 4) & 0xf);
	barcode[2] = ((data[1] & 0xf) << 2) | ((data[2] >> 6) & 0x3);
	barcode[3] = (data[2] & 0x3f);
	barcode[4] = ((data[3] >> 2) & 0x3f);
	barcode[5] = ((data[3] & 0x3) << 4) | ((data[4] >> 4) & 0xf);
	barcode[6] = ((data[4] & 0xf) << 2) | ((data[5] >> 6) & 0x3);
	nameLen = ((data[5] >> 2) & 0xf);
	pProd->type = (data[5] & 0x3);

	changeBarcodeFromNumbers(pProd, barcode);

	return nameLen;
}

void	changePriceDataFromBytes(BYTE* priceData, Product* pProd)
{
	int agorot, shkalim;
	pProd->count = priceData[0];
	agorot = ((priceData[1] >> 1) & 0x7f);
	shkalim = (priceData[1] & 0x1);
	shkalim = shkalim << 8;
	shkalim = (shkalim & 0x100) | (priceData[2] & 0xff);
	pProd->price = ((float)agorot / 100) + ((float)shkalim);
}