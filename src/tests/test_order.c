#include <stdio.h>
#include <stdlib.h>
#include "order.h"


int main() {
    printf("=== Starting Order Test ===\n");

    // Initialize an empty order
    Order *myOrder = (Order *)malloc(sizeof(Order));
    Store *myStore = (Store *)malloc(sizeof(Store));
    loadStore(myStore, "test_store.txt");
    

    // 1. Add items to the cart
    // printf("Adding items...\n");
    addCart(myOrder, "Krit","PROD001", 1);
    addCart(myOrder, "Krit2","PROD002", 1);

    // updateCart(myOrder, "Krit", "PROD001", -10);
    // updateCart(myOrder, "Krit", "PROD002", -999);

    // checkoutCart(myOrder, "Krit", "PROD001");
    int result = clearCart(myOrder, myStore, "Krit");
    printf("Clear cart status: %d\n", result);
    saveOrder(myOrder, "test_order.txt");
    saveStore(myStore, "test_store.txt");

    // printf("Load carts...\n");
    // Order *myOrder2 = (Order *)malloc(sizeof(Order));
    // loadOrder(myOrder2, "test_order.txt");
    // printOrder(myOrder2);

    // char orderDetails[GET_CART_ORDER_SIZE]; 
    // printf("Getting carts...\n");
    // getOrder(myOrder2, orderDetails, GET_CART_ORDER_SIZE,0);
    // printf("%s\n",orderDetails);

    // char orderDetails2[GET_CART_ORDER_SIZE]; 
    // printf("Getting orders...\n");
    // getOrder(myOrder2, orderDetails2, GET_CART_ORDER_SIZE,1);
    // printf("%s\n",orderDetails2);
    
    printf("\n=== Order Test Complete ===\n");
    return 0;
}