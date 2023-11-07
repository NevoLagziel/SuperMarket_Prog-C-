#pragma once

#include <stdio.h>
#include "Supermarket.h"

int saveSuperMarket(const SuperMarket* pMarket, const char* fileName, const char* compressedFileName,
	const char* customersFileName, const int fileType);

int loadSuperMarket(SuperMarket* pMarket, const char* fileName, const char* compressedFileName,
	const char* customersFileName, const int fileType);

int		saveSuperMarketToNFile(const SuperMarket* pMarket, const char* fileName);

int		loadSuperMarketFromNFile(SuperMarket* pMarket, const char* fileName);

//int		loadCustomerFromTextFile(SuperMarket* pMarket, const char* customersFileName);

int		loadProductFromTextFile(SuperMarket* pMarket, const char* fileName);

int		saveSuperMarketToCompressedFile(const SuperMarket* pMarket, const char* fileName);

int		SaveAllProductsToCompressedFile(const LIST* pList, FILE* fp);

int		loadSuperMarketFromCompressedFile(SuperMarket* pMarket, const char* fileName);

int		loadAllProductsFromCompressedFile(LIST* pList, const int productCount, FILE* fp);
