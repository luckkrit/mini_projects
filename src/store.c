
#include "store.h"

int updateProduct(Store *store, char *productId,char *productTitle,float price, int quantity)
{
    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed && strcmp(store->items[i].productId, productId) == 0)
        {
            if(strcmp(productTitle, IGNORE_UPDATE_TITLE)!=0){
                strcpy(store->items[i].productTitle, productTitle);
            }
            if(quantity != IGNORE_UPDATE_QUANTITY){
                store->items[i].quantity = quantity;
            }
            
            if(price != IGNORE_UPDATE_PRICE){
                store->items[i].price = price;
            }
            
            printf("Update product %s %s %f %d\n",store->items[i].productId,store->items[i].productTitle,store->items[i].price, store->items[i].quantity);
            return 0;
        }
    }

    return 1;
}
int updateStore(Store *store, char *productId,char *productTitle,float price, int quantity)
{
    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed && strcmp(store->items[i].productId, productId) == 0)
        {
            if(quantity==DELETE_QUANTITY){
                return deleteProduct(store, productId);
            }
            
            if (store->items[i].quantity + quantity < 0)
                return 1;

            

            store->items[i].quantity += quantity;
            if(price!=IGNORE_UPDATE_PRICE){
                store->items[i].price = price;
            }

            if(strcmp(productTitle, IGNORE_UPDATE_TITLE)!=0){
                productTitle = replace_char(productTitle,'-','_');
                strcpy(store->items[i].productTitle, productTitle);
            }

            printf("Update stock %s %s %f %d\n",store->items[i].productId,store->items[i].productTitle,store->items[i].price, store->items[i].quantity);
            return 0;
        }
    }

    return addProduct(store, productId,productTitle, price, quantity);
}
int printStore(Store *store)
{
    Stock *stock = store->items;
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (stock->isUsed)
        {
            printf("%s %s %f %d\n", stock->productId, stock->productTitle,stock->price, stock->quantity);
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
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);
    Stock *stock = store->items;
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (stock->isUsed)
        {
            fprintf(file_ptr, "%s,%s,%f,%d\n", stock->productId, stock->productTitle, stock->price, stock->quantity);
        }

        stock++;
    }
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}

int loadStore(Store *store, char *fileName)
{
    FILE *file_ptr = fopen(fileName, "r");

    if (file_ptr == NULL) {
        // File doesn't exist, let's create it!
        printf("File not found. Creating a new one...\n");
        file_ptr = fopen(fileName, "w+"); // w+ creates it and allows reading
    }

    if (file_ptr == NULL)
        return 1;

    char buffer[255];
    int i = 0;
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL && i < MAX_STOCK)
    {
        // 1. Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // 2. Correct Tokenizing
        char *productId = strtok(buffer, ",");
        char *productTitle = strtok(NULL, ",");
        char *priceStr = strtok(NULL, ",");
        char *quantityStr = strtok(NULL, ",");

        // 3. Check if tokens exist before using them
        if (productId != NULL && productTitle != NULL && quantityStr != NULL && priceStr != NULL)
        {
            strcpy(store->items[i].productId, productId);
            strcpy(store->items[i].productTitle, productTitle);
            store->items[i].quantity = atoi(quantityStr);
            store->items[i].price = atof(priceStr);
            store->items[i].isUsed = 1;
            i++;
        }
    }
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int addProduct(Store *store,char *productId, char *productTitle,float price, int quantity){

    // Add new
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (!store->items[i].isUsed)
        {
            store->items[i].isUsed = 1;
            strcpy(store->items[i].productId, productId);
            if(strcmp(productTitle, IGNORE_UPDATE_TITLE)==0){
                strcpy(store->items[i].productTitle, "Unknown");
            }else{
                productTitle = replace_char(productTitle,'-','_');
                strcpy(store->items[i].productTitle, productTitle);
            }
            if(quantity!= DELETE_QUANTITY){
                store->items[i].quantity = quantity;
            }
            if(price != IGNORE_UPDATE_PRICE){
                store->items[i].price = price;
            }
            return 0;
        }
    }
    
    return 1; 
}
int deleteProduct(Store *store,char *productId){

    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed && strcmp(store->items[i].productId, productId) == 0)
        {

            store->items[i].isUsed = 0;
            store->items[i].quantity = 0;
            store->items[i].price = 0;
            store->items[i].productId[0] = '\0';
            return 0;
        }
    }
    
    return 1; 
}
void getStore(Store *store, char *output, size_t outputSize)
{
    // 1. Initialize the buffer properly
    output[0] = '\0';
    strncat(output, "", outputSize - 1);

    char lineBuffer[1000]; // Temporary buffer for each line

    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed)
        {
            // 2. Format the line into a temporary buffer
            snprintf(lineBuffer, sizeof(lineBuffer), "%s-%s-%f-%d|",
                     store->items[i].productId, store->items[i].productTitle,store->items[i].price, store->items[i].quantity);

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
void searchStore(Store *store,char *productId,char *output, size_t outputSize){
    output[0] = '\0';
    strncat(output, "", outputSize - 1);

    char lineBuffer[1000]; // Temporary buffer for each line
    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (store->items[i].isUsed && strcmp(store->items[i].productId, productId) == 0)
        {

            // 2. Format the line into a temporary buffer
            snprintf(lineBuffer, sizeof(lineBuffer), "%s-%s-%f-%d|",
                     store->items[i].productId, store->items[i].productTitle,store->items[i].price, store->items[i].quantity);

            // 3. Check if there is enough space left in 'output' to append
            if (strlen(output) + strlen(lineBuffer) < outputSize - 1)
            {
                strcat(output, lineBuffer);
            }else{
                strcat(output, "Exceed buffer limit");
            }
            break;
        }
    }

}