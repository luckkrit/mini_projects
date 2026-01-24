#include "auth.h"
#include "store.h"

#define MAX_CART 100
#define ORDER_FILENAME "order.txt"
#define GET_ORDER_SIZE 2048

typedef struct {
    char username[USERNAME_SIZE];
    char productId[PRODUCTID_SIZE];
    int quantity;
    int checkout;
    bool isUsed;
} Cart;


typedef struct {
    Cart carts[MAX_CART];
} Order;

void getOrder(Order *head, char* output, size_t outputSize, int checkout);
int updateCart(Order *head, char* username, char* productId, int quantity);
int clearCart(Order *head, char* username);
int checkoutCart(Order *head, char* username, char* productId);
int addCart(Order *head, char* username, char* productId, int quantity);
int deleteCart(Order *head, char* username, char* productId);
int saveOrder(Order *head, char *fileName);
int loadOrder(Order *head, char *fileName);
int printOrder(Order *head);