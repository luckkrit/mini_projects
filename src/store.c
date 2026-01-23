#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "store.h"

int updateStore(Store *store, char *productId, int quantity)
{
    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed && strcmp(store->items[i].productId, productId) == 0)
        {
            if (store->items[i].quantity + quantity < 0)
                return 1;
            store->items[i].quantity += quantity;
            printf("Update stock %s %d\n",store->items[i].productId, store->items[i].quantity);
            return 0;
        }
    }
    // Add new if not found
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (!store->items[i].isUsed)
        {
            store->items[i].isUsed = 1;
            strncpy(store->items[i].productId, productId, PRODUCTID_SIZE);
            store->items[i].quantity = quantity;
            return 0;
        }
    }
    return -1; // Full
}
int printStore(Store *store)
{
    Stock *stock = store->items;
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (stock->isUsed)
        {
            printf("%s %d\n", stock->productId, stock->quantity);
        }

        stock++;
    }
    return 0;
}
int saveStore(Store *store, char *fileName)
{
    FILE *file_ptr;

    file_ptr = fopen(fileName, "w");

    if (file_ptr == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }

    Stock *stock = store->items;
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (stock->isUsed)
        {
            fprintf(file_ptr, "%s %d\n", stock->productId, stock->quantity);
        }

        stock++;
    }

    fclose(file_ptr);
    return 0;
}

int loadStore(Store *store, char *fileName)
{
    FILE *file_ptr = fopen(fileName, "r");
    if (file_ptr == NULL)
        return 1;

    char buffer[255];
    int i = 0;

    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL && i < MAX_STOCK)
    {
        // 1. Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // 2. Correct Tokenizing
        char *productId = strtok(buffer, " ");
        char *quantityStr = strtok(NULL, " ");

        // 3. Check if tokens exist before using them
        if (productId != NULL && quantityStr != NULL)
        {
            strcpy(store->items[i].productId, productId);
            store->items[i].quantity = atoi(quantityStr);
            store->items[i].isUsed = 1;
            i++;
        }
    }

    fclose(file_ptr);
    return 0;
}

void getStore(Store *store, char *output, size_t outputSize)
{
    // 1. Initialize the buffer properly
    output[0] = '\0';
    strncat(output, "--- OUR STORE ---\n", outputSize - 1);

    char lineBuffer[128]; // Temporary buffer for each line

    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed)
        {
            // 2. Format the line into a temporary buffer
            snprintf(lineBuffer, sizeof(lineBuffer), "%d). %s - Qty: %d\n",
                     i + 1, store->items[i].productId, store->items[i].quantity);

            // 3. Check if there is enough space left in 'output' to append
            if (strlen(output) + strlen(lineBuffer) < outputSize - 1)
            {
                strcat(output, lineBuffer);
            }
            else
            {
                break; // Buffer is full
            }
        }
    }
}