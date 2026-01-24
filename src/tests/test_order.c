#include <stdio.h>
#include <stdlib.h>
#include "order.h"

int main() {
    printf("=== Starting Order Test ===\n");

    // Initialize an empty order
    Order *myOrder = (Order *)malloc(sizeof(Order));
    

    // 1. Add items to the cart
    printf("Adding items...\n");
    addCart(myOrder, "Krit","PROD001", 50);
    addCart(myOrder, "Krit","PROD002", 20);

    updateCart(myOrder, "Krit", "PROD001", -10);
    // updateCart(myOrder, "Krit", "PROD002", -999);

    checkoutCart(myOrder, "Krit", "PROD001");
    // clearCart(myOrder, "Krit");

    saveOrder(myOrder, ORDER_FILENAME);

    printf("Load carts...\n");
    Order *myOrder2 = (Order *)malloc(sizeof(Order));
    loadOrder(myOrder2, ORDER_FILENAME);
    printOrder(myOrder2);

    char orderDetails[GET_ORDER_SIZE]; 
    printf("Getting carts...\n");
    getOrder(myOrder2, orderDetails, GET_ORDER_SIZE,0);
    printf("%s\n",orderDetails);

    char orderDetails2[GET_ORDER_SIZE]; 
    printf("Getting orders...\n");
    getOrder(myOrder2, orderDetails2, GET_ORDER_SIZE,1);
    printf("%s\n",orderDetails2);
    
    printf("\n=== Order Test Complete ===\n");
    return 0;
}