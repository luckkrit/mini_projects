#ifndef PROTOCOL_H
#define PROTOCOL_H

#define COMMAND_VIEW_PRODUCT "VIEW_PRODUCT"          // ANONYMOUS(2.1.1), MEMBER(2.2.1), ADMIN(2.3.1)
#define COMMAND_SEARCH_PRODUCT "SEARCH_PRODUCT"      // ANONYMOUS(2.1.2), MEMBER(2.2.1), ADMIN(2.3.1)
#define COMMAND_UPDATE_PRODUCT "UPDATE_PRODUCT"      // ADMIN(2.3.1)
#define COMMAND_ADD_PRODUCT "ADD_PRODUCT"      // ADMIN(2.3.1)
#define COMMAND_REMOVE_PRODUCT "REMOVE_PRODUCT"      // ADMIN(2.3.1)
#define COMMAND_UPDATE_CART "UPDATE_CART"            // MEMBER(2.2.2,2.2.4)
#define COMMAND_CLEAR_CART "CLEAR_CART"              // MEMBER(2.2.4) - ล้างตะกร้า
#define COMMAND_VIEW_CART "VIEW_CART"                // MEMBER(2.2.3)
#define COMMAND_CHECKOUT_CART "CHECKOUT_CART"          // MEMBER(2.2.5)
#define COMMAND_VIEW_ORDER "VIEW_ORDER"              // MEMBER(2.2.6), ADMIN (2.3.2)
#define COMMAND_VIEW_MEMBER "VIEW_MEMBER"            // ADMIN (2.3.3)
#define COMMAND_REGISTER_MEMBER "REGISTER_MEMBER"           // ANONYMOUS (2.1.4)
#define COMMAND_LOGOUT "LOGOUT"                      // MEMBER(2.2.7), ADMIN (2.3.7)
#define COMMAND_LOGIN "LOGIN"                        // ANONYMOUS(2.1.3)
#define COMMAND_SEPARATOR ","
#define COMMAND_UNKNOWN "UNKNOWN"

typedef enum {
    STATUS_OK,                               // 0
    STATUS_FAIL,                             // 1
    STATUS_INVALID_SESSION,                  // 2
    STATUS_UNKNOWN,                          // 3
    STATUS_INVALID_ARGUMENTS,                // 4
    STATUS_DUPLICATE_USER                    // 5
} ResponseStatus;

#endif