#define PRODUCTID_SIZE 10
#define MAX_STOCK 100
#define STORE_FILENAME "store.txt"
#define STORE_SIZE 2048
typedef struct stock Stock;

struct stock {
    char productId[PRODUCTID_SIZE];
    int quantity;
    int isUsed; // Flag: 1 if this slot has a product, 0 if empty
};

typedef struct store Store;

struct store {
    struct stock items[MAX_STOCK]; 
};

int updateStore(Store *store,char *productId, int quantity);
int printStore(Store *store);
int saveStore(Store *store, char *fileName);
int loadStore(Store *store, char *fileName);
void getStore(Store *store, char *output, size_t outputSize);