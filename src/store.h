#define PRODUCTID_SIZE 5
#define STORE_FILENAME "store.txt"
#define STORE_SIZE 2048
typedef struct stock Stock;

struct stock {
    char productId[PRODUCTID_SIZE];
    int quantity;
    Stock *next;
};

typedef struct store Store;

struct store {
    Stock *stock;
};

int updateStore(Store *store,char *productId, int quantity);
int printStore(Store *store);
int saveStore(Store *store, char *fileName);
int loadStore(Store *store, char *fileName);
void freeStore(Store *store);
void getStore(Store *store, char *output, size_t outputSize);