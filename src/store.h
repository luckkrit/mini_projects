#ifndef STORE_H // Fix Store is undefined in server.c
#define STORE_H

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#define PRODUCTID_SIZE 10
#define PRODUCT_TITLE_SIZE 500
#define MAX_STOCK 100
#define STORE_FILENAME "store.txt"
#define GET_STORE_SIZE 2048
#define DELETE_QUANTITY -999
#define IGNORE_UPDATE_PRICE -999
#define IGNORE_UPDATE_TITLE "-999"

typedef struct stock Stock;

struct stock {
    char productId[PRODUCTID_SIZE];
    char productTitle[PRODUCT_TITLE_SIZE];
    int quantity;
    float price;
    int isUsed; // Flag: 1 if this slot has a product, 0 if empty
};

typedef struct store Store;

struct store {
    struct stock items[MAX_STOCK]; 
};

int updateStore(Store *store,char *productId, char *productTitle,float price, int quantity);
int deleteStore(Store *store,char *productId);
int addStore(Store *store,char *productId,char *productTitle,float price, int quantity);
int printStore(Store *store);
int saveStore(Store *store, char *fileName);
int loadStore(Store *store, char *fileName);
void getStore(Store *store,char *output, size_t outputSize);
void searchStore(Store *store,char *productId,char *output, size_t outputSize);


#endif