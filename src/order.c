#include "order.h"

int updateCart(Order *head, char* username, char* productId, int quantity){
// Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (head->carts[i].isUsed && strcmp(head->carts[i].productId, productId) == 0 && strcmp(head->carts[i].username, username) == 0)
        {
            if(quantity==DELETE_QUANTITY){
                return deleteCart(head, username, productId);
            }
            
            if (head->carts[i].quantity + quantity <= 0)
                return deleteCart(head, username, productId);

            

            head->carts[i].quantity += quantity;

            printf("Update cart %s %s %d\n",head->carts[i].username,head->carts[i].productId,head->carts[i].quantity);
            return 0;
        }
    }

    return addCart(head, username, productId, quantity);
}

int saveOrder(Order *head, char *fileName)
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
    Cart *cart = head->carts;
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (cart->isUsed)
        {
            fprintf(file_ptr, "%s,%s,%d,%d\n", cart->username, cart->productId, cart->quantity, cart->checkout);
        }

        cart++;
    }
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}

int loadOrder(Order *head, char *fileName)
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
        char *username = strtok(buffer, ",");
        char *productId = strtok(NULL, ",");
        char *quantityStr = strtok(NULL, ",");
        char *checkoutStr = strtok(NULL, ",");

        // 3. Check if tokens exist before using them
        if (productId != NULL && username != NULL && quantityStr != NULL && checkoutStr != NULL)
        {
            strcpy(head->carts[i].productId, productId);
            strcpy(head->carts[i].username, username);
            head->carts[i].quantity = atoi(quantityStr);
            head->carts[i].checkout = atoi(checkoutStr);
            head->carts[i].isUsed = 1;
            i++;
        }
    }
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int addCart(Order *head,char *username,char *productId, int quantity){

    // Add new
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (!head->carts[i].isUsed)
        {
            head->carts[i].isUsed = 1;
            strcpy(head->carts[i].productId, productId);
            strcpy(head->carts[i].username, username);
            if(quantity!= DELETE_QUANTITY){
                head->carts[i].quantity = quantity;
            }
            
            return 0;
        }
    }
    
    return 1; 
}
int deleteCart(Order *head,char *username,char *productId){

    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (head->carts[i].isUsed && strcmp(head->carts[i].productId, productId) == 0 && strcmp(head->carts[i].username, username) == 0)
        {

            head->carts[i].isUsed = 0;
            head->carts[i].quantity = 0;
            head->carts[i].username[0] = '\0';
            head->carts[i].productId[0] = '\0';
            head->carts[i].checkout = 0;
            return 0;
        }
    }
    
    return 1; 
}
int clearCart(Order *orderHead,Store *storeHead,char *username){

    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (orderHead->carts[i].isUsed && strcmp(orderHead->carts[i].username, username) == 0)
        {
            int result = updateStore(storeHead, orderHead->carts[i].productId, IGNORE_UPDATE_TITLE, IGNORE_UPDATE_PRICE, orderHead->carts[i].quantity);
            printf("store update result %d\n",result);
            if(result!=0){
                return result;
            }
            orderHead->carts[i].isUsed = 0;
            orderHead->carts[i].quantity = 0;
            orderHead->carts[i].username[0] = '\0';
            orderHead->carts[i].productId[0] = '\0';
            orderHead->carts[i].checkout = 0;
            return 0;
        }
    }
    
    return 1; 
}
int printOrder(Order *head)
{
    Cart *cart = head->carts;
    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (cart->isUsed)
        {
            printf("%s %s %d %d\n", cart->username, cart->productId, cart->quantity, cart->checkout);
        }

        cart++;
    }
    return 0;
}
int checkoutCart(Order *head, char* username, char* productId){
    // Search for existing
    for (int i = 0; i < MAX_STOCK; i++)
    {
        // printf("compare : %d\n",head->carts[i].isUsed && strcmp(head->carts[i].username, username) == 0);
        if (head->carts[i].isUsed && strcmp(head->carts[i].username, username) == 0)
        {

            if(strcmp(productId, ALL_PRODUCT_ID)==0){
                head->carts[i].checkout = 1;
                return 0;
            }else if(strcmp(head->carts[i].productId, productId)==0){
                head->carts[i].checkout = 1;
                return 0;
            }
        }
    }
    return 1;
}
void getOrder(Order *order,char *username, char *output, size_t outputSize, int checkout)
{
    printf("username = %s\n",username);
    // 1. Initialize the buffer properly
    output[0] = '\0';
    strncat(output, "", outputSize - 1);

    char lineBuffer[1000]; // Temporary buffer for each line

    for (int i = 0; i < MAX_STOCK; i++)
    {
        if (order->carts[i].isUsed && order->carts[i].checkout == checkout && strcmp(order->carts[i].username,username)==0)
        {
            // 2. Format the line into a temporary buffer
            snprintf(lineBuffer, sizeof(lineBuffer), "%s-%s-%d-%d|",
                     order->carts[i].username, order->carts[i].productId, order->carts[i].quantity, order->carts[i].checkout);

            // 3. Check if there is enough space left in 'output' to append
            if (strlen(output) + strlen(lineBuffer) < outputSize - 1)
            {
                
                strcat(output, lineBuffer);
            }
            else
            {
                break; // Buffer is full
            }
        }else if(order->carts[i].isUsed && order->carts[i].checkout == checkout && strcmp(order->carts[i].username,ALL_USERS)==0){
            // 2. Format the line into a temporary buffer
            snprintf(lineBuffer, sizeof(lineBuffer), "%s-%s-%d-%d|",
                     order->carts[i].username, order->carts[i].productId, order->carts[i].quantity, order->carts[i].checkout);

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