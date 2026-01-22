#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "store.h"

int updateStore(Store *store,char *productId, int quantity){
    
    if(store->stock!=NULL){
        Stock *oldStock = store->stock;
        int found = 0;
        while(oldStock!=NULL){
            if(strcmp(productId, oldStock->productId)==0){
                if(oldStock->quantity + quantity < 0){
                    return 1;
                }
                oldStock->quantity += quantity;
                printf("update stock %d\n", oldStock->quantity);
                found = 1;
                break;
            }else{
                if(oldStock->next == NULL){
                    break;
                }else{
                    oldStock = oldStock->next;
                }
                
            }
        }
        if(found==0){
            Stock *newStock = (Stock *)malloc(sizeof(Stock));
            strcpy(newStock->productId, productId);
            newStock->quantity = quantity;
            oldStock = store->stock;
            newStock->next = oldStock;
            store->stock = newStock;
        }
    }else{
        Stock *newStock = (Stock *)malloc(sizeof(Stock));
        strcpy(newStock->productId, productId);
        newStock->quantity = quantity;
        newStock->next = NULL; 
        store->stock = newStock;
    }
    return 0;
}
int printStore(Store *store){
    Stock *stock = store->stock;
    while(stock!=NULL){
        printf("%s %d\n", stock->productId, stock->quantity);
        stock = stock->next;
    }
    return 0;
}
int saveStore(Store *store, char *fileName){
    FILE *file_ptr;

    
    file_ptr = fopen(fileName, "w");

    
    if (file_ptr == NULL) {
        printf("Error opening file!\n");        
        return 1;
    }

    Stock *stock = store->stock;
    while(stock!=NULL){
        fprintf(file_ptr, "%s %d\n", stock->productId, stock->quantity);
        stock = stock->next;
    }
    

    fclose(file_ptr);
    return 0;
}

int loadStore(Store *store,char *fileName){
    FILE *file_ptr;
    char buffer[255];
    
    file_ptr = fopen(fileName, "r");

    
    if (file_ptr == NULL) {
        printf("Error opening file!\n");        
        return 1;
    }
    
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
        char *token = strtok(buffer, " ");
        Stock *newStock = (Stock *)malloc(sizeof(Stock));
        int c = 0;    
        while(token!=NULL){
            
            if(c==0){
                strcpy(newStock->productId, token);                    
                newStock->next = store->stock;
                
            }
            if(c==1){
                newStock->quantity = atoi(token);
            }
            c++;
            token = strtok(NULL, " ");
        }
        store->stock = newStock;
    }
    
    fclose(file_ptr);
    return 0;
}

void freeStore(Store *store) {
    if (store == NULL) return;

    Stock *current = store->stock;
    Stock *next_node;

    while (current != NULL) {
        next_node = current->next; // Save reference to next
        free(current);             // Delete current node
        current = next_node;       // Move to next
    }
    
    store->stock = NULL; 
}

void getStore(Store *store, char *output, size_t outputSize) {
    // 1. Initialize the buffer properly
    output[0] = '\0'; 
    strncat(output, "--- OUR STORE ---\n", outputSize - 1);

    Stock *stock = store->stock;
    int c = 1;
    char lineBuffer[128]; // Temporary buffer for each line

    while(stock != NULL) {
        // 2. Format the line into a temporary buffer
        snprintf(lineBuffer, sizeof(lineBuffer), "%d). %s - Qty: %d\n", 
                 c, stock->productId, stock->quantity);

        // 3. Check if there is enough space left in 'output' to append
        if (strlen(output) + strlen(lineBuffer) < outputSize - 1) {
            strcat(output, lineBuffer);
        } else {
            break; // Buffer is full
        }

        stock = stock->next;
        c++;
    }
}