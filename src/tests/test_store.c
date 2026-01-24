#include <stdio.h>
#include <stdlib.h>
#include "store.h"

int main() {
    printf("=== Starting Inventory Store Test ===\n");

    // Initialize an empty store
    Store *myStore = (Store *)malloc(sizeof(Store));
    

    // 1. Add items to the store
    printf("Adding items...\n");
    addStore(myStore, "PROD001","APPLE IPHONE15",10.0, 50);
    addStore(myStore, "PROD002","ASUS VIVO BOOK15",11.0, 20);
    
    // Test updating existing item (adding 10 more)
    updateStore(myStore, "PROD001",IGNORE_UPDATE_TITLE,IGNORE_UPDATE_PRICE, 10); 
    
    // Test updating existing item (remove 1)
    updateStore(myStore, "PROD001",IGNORE_UPDATE_TITLE,IGNORE_UPDATE_PRICE, -1); 

    // Test delete existing item
    deleteStore(myStore, "PROD002");

    // 2. Print current state
    printf("\nCurrent Store Contents:\n");
    printStore(myStore);

    // // 3. Save to file
    printf("\nSaving to test_store.txt...\n");
    saveStore(myStore, "test_store.txt");

    // // 4. Create a second store to test loading
    Store *loadedStore = (Store *)malloc(sizeof(Store));
    

    printf("Loading into a new store object...\n");
    if (loadStore(loadedStore, "test_store.txt") == 0) {
        printf("Loaded Store Contents:\n");
        printStore(loadedStore);
    } else {
        printf("Failed to load store.\n");
    }


    // 5. Get store details
    printf("\nGet store...\n");
    char storeDetails[GET_STORE_SIZE]; 
    getStore(myStore, storeDetails, GET_STORE_SIZE);
    printf("%s\n",storeDetails);
    
    // add more 10 items
    updateStore(myStore, "PROD001",IGNORE_UPDATE_TITLE,IGNORE_UPDATE_PRICE, 10);

    char searchDetails[GET_STORE_SIZE]; 
    searchStore(myStore,"PROD002", searchDetails, GET_STORE_SIZE);
    printf("Search result: %s\n", searchDetails);
    
    printf("\n=== Store Test Complete ===\n");
    return 0;
}