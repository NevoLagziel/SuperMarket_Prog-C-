#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Address.h"
#include "General.h"
#include "fileHelper.h"
#include "SuperFile.h"
#include "myMacros.h"



int saveSuperMarket(const SuperMarket* pMarket, const char* fileName, const char* compressedFileName, 
					const char* customersFileName, const int fileType)
{
	if (fileType == 1)
	{
		if (!saveSuperMarketToCompressedFile(pMarket,compressedFileName))
			return 0;
	}
	else 
	{
		if (!saveSuperMarketToNFile(pMarket, fileName))
			return 0;
	}
	if (!saveCustomerToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName))
		return 0;
	return 1;
}

int loadSuperMarket(SuperMarket* pMarket, const char* fileName, const char* compressedFileName,
					const char* customersFileName, const int fileType)
{
	if (fileType == 1)
	{
		if (!loadSuperMarketFromCompressedFile(pMarket, compressedFileName))
			return 0;
	}
	else
	{
		if (!loadSuperMarketFromNFile(pMarket, fileName))
			return 0;
	}
	pMarket->customerArr = loadCustomerFromTextFile(customersFileName, &pMarket->customerCount);
	CHECK_RETURN_0(pMarket->customerArr);
	return 1;
}

int	saveSuperMarketToNFile(const SuperMarket* pMarket, const char* fileName)
{
	FILE* fp;
	fp = fopen(fileName, "wb");
	CHECK_MSG_RETURN_0(fp, "Error open supermarket file to write\n");

	if (!writeStringToFile(pMarket->name, fp, "Error write supermarket name\n"))
		CLOSE_RETURN_0(fp);
	if (!saveAddressToFile(&pMarket->location, fp))
		CLOSE_RETURN_0(fp);
	int count = getNumOfProductsInList(pMarket);

	if (!writeIntToFile(count, fp, "Error write product count\n"))
		CLOSE_RETURN_0(fp);

	Product* pTemp;
	NODE* pN = pMarket->productList.head.next; //first Node
	while (pN != NULL)
	{
		pTemp = (Product*)pN->key;
		if (!saveProductToFile(pTemp, fp))
			CLOSE_RETURN_0(fp);
		pN = pN->next;
	}
	fclose(fp);
	return 1;
}
#define BIN
#ifdef BIN
int	loadSuperMarketFromNFile(SuperMarket* pMarket, const char* fileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Error open company file\n");

	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	if (!pMarket->name)
		CLOSE_RETURN_0(fp);
	if (!loadAddressFromFile(&pMarket->location, fp))
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);

	int count;
	if (!readIntFromFile(&count, fp, "Error reading product count\n"))
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);

	if (count > 0)
	{
		Product* pTemp;
		for (int i = 0; i < count; i++)
		{
			pTemp = (Product*)calloc(1, sizeof(Product));
			if (!pTemp)
			{
				printf("Allocation error\n");
				L_free(&pMarket->productList, freeProduct);
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}
			if (!loadProductFromFile(pTemp, fp))
			{
				L_free(&pMarket->productList, freeProduct);
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}
			if (!insertNewProductToList(&pMarket->productList, pTemp))
			{
				L_free(&pMarket->productList, freeProduct);
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}
		}
	}
	fclose(fp);

	return	1;

}
#else
int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Error open company file\n");
		return 0;
	}

	//L_init(&pMarket->productList);


	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	if (!pMarket->name)
	{
		fclose(fp);
		return 0;
	}

	if (!loadAddressFromFile(&pMarket->location, fp))
	{
		free(pMarket->name);
		fclose(fp);
		return 0;
	}

	fclose(fp);

	loadProductFromTextFile(pMarket, "Products.txt");


	return	1;

}
#endif

int		loadProductFromTextFile(SuperMarket* pMarket, const char* fileName)
{
	FILE* fp;
	//L_init(&pMarket->productList);
	fp = fopen(fileName, "r");
	int count;
	fscanf(fp, "%d\n", &count);


	//Product p;
	Product* pTemp;
	for (int i = 0; i < count; i++)
	{
		pTemp = (Product*)calloc(1, sizeof(Product));
		myGets(pTemp->name, sizeof(pTemp->name), fp);
		myGets(pTemp->barcode, sizeof(pTemp->barcode), fp);
		fscanf(fp, "%d %f %d\n", &pTemp->type, &pTemp->price, &pTemp->count);
		insertNewProductToList(&pMarket->productList, pTemp);
	}

	fclose(fp);
	return 1;
}


int		saveSuperMarketToCompressedFile(const SuperMarket* pMarket, const char* fileName)
{
	FILE* fp = fopen(fileName, "wb");
	CHECK_MSG_RETURN_0(fp, "Error opening supermarket file\n");

	BYTE byte[2] = { 0 };
	int productCount = getNumOfProductsInList(pMarket);
	int nameLen = (int)strlen(pMarket->name);
	byte[0] = (productCount >> 2);
	byte[1] = (productCount << 6) | nameLen;
	
	if (fwrite(byte, sizeof(BYTE), 2, fp) != 2)
		CLOSE_RETURN_0(fp);

	if (fwrite(pMarket->name, sizeof(char), nameLen, fp) != nameLen)
		CLOSE_RETURN_0(fp);

	if (!saveAddressToCompressedFile(&pMarket->location, fp))
		CLOSE_RETURN_0(fp);
	
	if (!SaveAllProductsToCompressedFile(&pMarket->productList, fp))
		CLOSE_RETURN_0(fp);

	fclose(fp);
	return 1;
}

int		SaveAllProductsToCompressedFile(const LIST* pList, FILE* fp)
{
	NODE* node = pList->head.next;
	while (node != NULL)
	{
		if (!saveProductToCompressedFile((Product*)node->key, fp))
			return 0;
		node = node->next;
	}
	return 1;
}

int		loadSuperMarketFromCompressedFile(SuperMarket* pMarket, const char* fileName)
{
	FILE* fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Error oppening supermarket file\n");
	BYTE byte[2] = { 0 };
	int productCount, nameLen;
	if (fread(byte, sizeof(BYTE), 2, fp) != 2)
		CLOSE_RETURN_0(fp);
	productCount = (byte[0] & 0xff);
	productCount = (productCount << 2);
	productCount = productCount | ((byte[1] >> 6) & 0x3);
	nameLen = (byte[1] & 0x3f);
	pMarket->name = (char*)malloc((nameLen + 1) * sizeof(char));
	if (!pMarket->name)
		CLOSE_RETURN_0(fp);
	if (fread(pMarket->name, sizeof(char), nameLen, fp) != nameLen)
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	pMarket->name[nameLen] = '\0';
	if (!loadAddressFromCompressedFile(&pMarket->location, fp))
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);	
	if (!loadAllProductsFromCompressedFile(&pMarket->productList, productCount, fp))
	{
		freeAddress(&pMarket->location);
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	}
	fclose(fp);
	return 1;
}

int		loadAllProductsFromCompressedFile(LIST* pList,const int productCount, FILE* fp)
{
	NODE* node = pList->head.next;
	for (int i = 0; i < productCount; i++)
	{
		Product* temp = (Product*)malloc(1 * sizeof(Product));
		CHECK_RETURN_0(temp);

		if (!loadProductFromCompressedFile(temp, fp))
		{
			L_free(pList, freeProduct);
			free(temp);
			return 0;
		}
		if (!insertNewProductToList(pList, temp))
		{
			L_free(pList, freeProduct);
			free(temp);
			return 0;
		}
	}
	return 1;
}



