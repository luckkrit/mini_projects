#include <stdio.h>
#include <stdlib.h>
#include "store.h"

int main() {
    printf("=== Starting Inventory Store Test ===\n");

    // Initialize an empty store
    Store *myStore = (Store *)malloc(sizeof(Store));
    myStore->stock = NULL;

    // 1. Add items to the store
    printf("Adding items...\n");
    updateStore(myStore, "PROD001", 50);
    updateStore(myStore, "PROD002", 20);
    
    // Test updating existing item (adding 10 more)
    updateStore(myStore, "PROD001", 10); 

    // 2. Print current state
    printf("\nCurrent Store Contents:\n");
    printStore(myStore);

    // 3. Save to file
    printf("\nSaving to test_inventory.txt...\n");
    saveStore(myStore, "test_inventory.txt");

    // 4. Create a second store to test loading
    Store *loadedStore = (Store *)malloc(sizeof(Store));
    loadedStore->stock = NULL;

    printf("Loading into a new store object...\n");
    if (loadStore(loadedStore, "test_inventory.txt") == 0) {
        printf("Loaded Store Contents:\n");
        printStore(loadedStore);
    } else {
        printf("Failed to load store.\n");
    }


    // 5. Get store details
    printf("\nGet store...\n");
    char storeDetails[STORE_SIZE]; 
    getStore(myStore, storeDetails, STORE_SIZE);
    printf(storeDetails);

    // Clean up memory (In a real app, you'd have a freeStore function)
    printf("\n=== Store Test Complete ===\n");
    return 0;
}