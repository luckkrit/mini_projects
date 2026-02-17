#include "client.h"

int userLevel = -1;
char *sessionId = NULL;

void getFloatInput(char *buffer, size_t size) {
    while (1) {
        printf("Enter value (e.g., 10.50) [Press Enter to skip]: ");
        if (fgets(buffer, size, stdin)) {
            // 1. ลบ \n ที่ติดมาจาก fgets
            buffer[strcspn(buffer, "\n")] = 0;

            // 2. ถ้าเป็นค่าว่างหรือมีแต่ช่องว่าง -> ให้ผ่านได้
            if (isEmptyOrSpace(buffer)) {
                buffer[0] = '\0'; // ทำให้แน่ใจว่าเป็นสตริงว่างจริงๆ ""
                break; 
            }

            // 3. ถ้าไม่ว่าง ต้องเป็นรูปแบบทศนิยมที่ถูกต้อง
            if (isFloat(buffer)) {
                break; 
            } else {
                printf("Invalid input! Please enter numbers or leave blank to skip.\n");
            }
        }
    }
}

void getNumericInput(char *buffer, size_t size) {
    while (1) {
        printf("Enter number [Press Enter to skip]: ");
        if (fgets(buffer, size, stdin)) {
            buffer[strcspn(buffer, "\n")] = 0; // ลบ \n

            // ถ้าเป็นค่าว่าง หรือมีแต่ช่องว่าง -> อนุญาตให้ผ่าน
            if (isEmptyOrSpace(buffer)) {
                buffer[0] = '\0'; // ล้างให้เป็นสตริงว่างจริงๆ
                break;
            }

            // ถ้าไม่ว่าง ต้องเป็นตัวเลขเท่านั้น
            if (isNumeric(buffer)) {
                break;
            } else {
                printf("Invalid input! Numbers only.\n");
            }
        }
    }
}

void getInput(char *input, size_t size){
    
    fgets(input, size, stdin);
    if(input){
        replace_char(input,'\n','\0');
    }
}
void getPassword(char *password, size_t size) {
    struct termios oldt, newt;

    printf("\nEnter password: ");
    
    // 1. เก็บค่าตั้งค่าปัจจุบันของ Terminal ไว้
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 2. ปิดโหมด ECHO (ไม่ให้แสดงตัวอักษรที่พิมพ์)
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // 3. ใช้ fgets รับข้อมูลตามปกติ
    if (fgets(password, size, stdin)) {
        password[strcspn(password, "\n")] = 0; // ลบ \n ออก
    }

    // 4. คืนค่าการตั้งค่า Terminal ให้กลับเป็นปกติ
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n"); // ขึ้นบรรทัดใหม่เพราะตอนพิมพ์เราไม่ได้เห็น \n
}
void prompt(char *message, char *buffer)
{

    printf("%s", message);
    if (fgets(buffer, sizeof(buffer), stdin))
    {
        buffer[strcspn(buffer, "\n")] = 0;
        // printf("You entered: %s\n", buffer);
    }
}
void pressEnterToContinue()
{
    printf("\nPress Enter to continue...");
    char buffer[10];
    fgets(buffer, sizeof(buffer), stdin);
}
int sendData(char *sendMessage, char *receiveMessage)
{
    printf("Connecting to server...\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *peer_address;
    if (getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo() failed.\n");
        return 1;
    }

    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    if (connect(socket_peer,
                peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    printf("Connected. (Ctrl+C to quit):\n");

    // 1. ส่งข้อมูลที่รับมาจาก Parameter ทันที
    printf("Sending: %s\n", sendMessage);
    int bytes_sent = send(socket_peer, sendMessage, strlen(sendMessage), 0);
    if (bytes_sent < 1)
    {
        fprintf(stderr, "send() failed. (%d)\n", GETSOCKETERRNO());
        CLOSESOCKET(socket_peer);
        return 1;
    }

    // 2. (Optional) รอรับ Response จาก Server หลังจากส่งไปแล้ว

    int bytes_received = recv(socket_peer, receiveMessage, RECEIVE_MESSAGE_SIZE - 1, 0);
    if (bytes_received > 0)
    {
        receiveMessage[bytes_received] = '\0';
        printf("Received from server: %s\n", receiveMessage);
    }

    printf("Closing socket...\n");
    CLOSESOCKET(socket_peer);
    return 0;
}

void handleSearchProducts()
{
    // wait for input
    printf("\nSearch product by ID:");
    char productId[100];
    getInput(productId,100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_SEARCH_PRODUCT, COMMAND_SEPARATOR, productId);
    int result = sendData(sendMessage, receiveMessage);

    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        // receive message
        replace_char(receiveMessage, '\n', '\0');
        
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        char *products_str = strtok(NULL, ",\n\r");
        int status = atoi(status_str);
        if(products_str==NULL){
            printf("Product not found!\n");
            goto SHOW_MENU;
        }
        switch (status)
        {
        case STATUS_OK:
            printf("Search products success!\n");
            char *saveptr1, *saveptr2;
            // ลูปที่ 1: ตัดด้วยเครื่องหมาย '|' เพื่อแยกแต่ละรายวิชา
            char *product_token = strtok_r(products_str, "|", &saveptr1);

            while (product_token != NULL)
            {
                printf("Processing Product: %s\n", product_token);

                // ลูปที่ 2: ตัดด้วยเครื่องหมาย '-' เพื่อแยกรายละเอียดภายในวิชานั้น
                // หมายเหตุ: ต้องใช้ copy ของ product_token หรือระวังเรื่องการเปลี่ยนแปลงค่า
                char *detail = strtok_r(product_token, "-", &saveptr2);
                int field_count = 0;

                while (detail != NULL)
                {
                    if (field_count == 0)
                        printf("  ID: %s\n", detail);
                    if (field_count == 1)
                        printf("  Name: %s\n", detail);
                    if (field_count == 2)
                        printf("  Price: %s\n", detail);
                    if (field_count == 3)
                        printf("  Stock: %s\n", detail);

                    detail = strtok_r(NULL, "-", &saveptr2);
                    field_count++;
                }

                printf("-------------------\n");
                product_token = strtok_r(NULL, "|", &saveptr1);
            }
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
            
    }
}

void handleViewProducts()
{
    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s\n", COMMAND_VIEW_PRODUCT);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        char *products_str = strtok(NULL, ",\n\r");
        int status = atoi(status_str);
        if(products_str==NULL){
            printf("Empty products!\n");
            goto SHOW_MENU;
        }
        switch (status)
        {
        case STATUS_OK:
            printf("View products success!\n");
            char *saveptr1, *saveptr2;
            // ลูปที่ 1: ตัดด้วยเครื่องหมาย '|' เพื่อแยกแต่ละรายวิชา
            char *product_token = strtok_r(products_str, "|", &saveptr1);

            while (product_token != NULL)
            {
                printf("Processing Product: %s\n", product_token);

                // ลูปที่ 2: ตัดด้วยเครื่องหมาย '-' เพื่อแยกรายละเอียดภายในวิชานั้น
                // หมายเหตุ: ต้องใช้ copy ของ product_token หรือระวังเรื่องการเปลี่ยนแปลงค่า
                char *detail = strtok_r(product_token, "-", &saveptr2);
                int field_count = 0;

                while (detail != NULL)
                {
                    if (field_count == 0)
                        printf("  ID: %s\n", detail);
                    if (field_count == 1)
                        printf("  Name: %s\n", detail);
                    if (field_count == 2)
                        printf("  Price: %s\n", detail);
                    if (field_count == 3)
                        printf("  Stock: %s\n", detail);

                    detail = strtok_r(NULL, "-", &saveptr2);
                    field_count++;
                }

                printf("-------------------\n");
                product_token = strtok_r(NULL, "|", &saveptr1);
            }
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
            
    }
}
void handleLogin(){
    // wait for input
    printf("\n--- Login ---\n");
    printf("Username: ");
    char username[100];
    getInput(username, 100);

    char password[100];
    getPassword(password,100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s\n", COMMAND_LOGIN,COMMAND_SEPARATOR, username, COMMAND_SEPARATOR, password);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        // sessionId = strtok(NULL, ",\n\r");
        char *tempSession = strtok(NULL, ",\n\r");
        if (tempSession != NULL) {
            if (sessionId != NULL) free(sessionId); // Clean up old session if it exists
            sessionId = strdup(tempSession);        // Allocate permanent memory for the ID
        }
        char *isAdmin = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            userLevel = atoi(isAdmin);
            printf("Login success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }

        pressEnterToContinue();
        
    }

}

void handleRegister(){
    // wait for input
    printf("\n--- Login ---\n");
    printf("Username: ");
    char username[100];
    getInput(username, 100);

    char password[100];
    getPassword(password,100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s\n", COMMAND_REGISTER_MEMBER,COMMAND_SEPARATOR, username, COMMAND_SEPARATOR, password);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Register success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }

        pressEnterToContinue();
        
    }

}

void handleAddToCart(){

    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }

    // wait for input
    printf("\n--- Add to cart ---\n");
    printf("Product ID: ");
    char productId[100];
    getInput(productId, 100);

    printf("Quantity: ");
    char quantity[100];
    getNumericInput(quantity, 100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s%s%s\n", COMMAND_UPDATE_CART,COMMAND_SEPARATOR, sessionId, COMMAND_SEPARATOR, productId, COMMAND_SEPARATOR, quantity);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Add to cart success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleViewCart(){
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }
    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_VIEW_CART, COMMAND_SEPARATOR, sessionId);
    int result = sendData(sendMessage, receiveMessage);

    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        // receive message
        replace_char(receiveMessage, '\n', '\0');
        
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        char *carts_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        if(carts_str==NULL){
            printf("Empty cart!\n");
            goto SHOW_MENU;
        }
        switch (status)
        {
        case STATUS_OK:
            printf("View cart success!\n");
            char *saveptr1, *saveptr2;
            // ลูปที่ 1: ตัดด้วยเครื่องหมาย '|' เพื่อแยกแต่ละรายวิชา
            char *cart_token = strtok_r(carts_str, "|", &saveptr1);

            while (cart_token != NULL)
            {
                printf("Processing Cart: %s\n", cart_token);

                // ลูปที่ 2: ตัดด้วยเครื่องหมาย '-' เพื่อแยกรายละเอียดภายในวิชานั้น
                // หมายเหตุ: ต้องใช้ copy ของ cart_token หรือระวังเรื่องการเปลี่ยนแปลงค่า
                char *detail = strtok_r(cart_token, "-", &saveptr2);
                int field_count = 0;

                while (detail != NULL)
                {
                    if (field_count == 0)
                        printf("  Username: %s\n", detail);
                    if (field_count == 1)
                        printf("  Product ID: %s\n", detail);
                    if (field_count == 2)
                        printf("  Quantity: %s\n", detail);
                    if (field_count == 3)
                        printf("  Checkout: %s\n", detail);

                    detail = strtok_r(NULL, "-", &saveptr2);
                    field_count++;
                }

                printf("-------------------\n");
                cart_token = strtok_r(NULL, "|", &saveptr1);
            }
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
            
    }
}
void handleRemoveCart(){

    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }

    // wait for input
    printf("\n--- Remove cart ---\n");
    printf("Product ID: ");
    char productId[100];
    getInput(productId, 100);

    printf("Quantity: ");
    char quantity[100];
    getNumericInput(quantity, 100);

    int qty = atoi(quantity);
    if(qty>0){
        quantity[0] = '\0';
        snprintf(quantity,sizeof(quantity), "%d", -qty);
    }

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s%s%s\n", COMMAND_UPDATE_CART,COMMAND_SEPARATOR, sessionId, COMMAND_SEPARATOR, productId, COMMAND_SEPARATOR, quantity);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Remove cart success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
            
    }
}
void handleCheckoutCart()
{
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }

    // wait for input
    printf("\n--- Checkout cart ---\n");
    printf("Product ID: ");
    char productId[100];
    getInput(productId, 100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s\n", COMMAND_CHECKOUT_CART,COMMAND_SEPARATOR, sessionId, COMMAND_SEPARATOR, productId);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Checkout cart success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
            
    }
}
void handleViewOrder(){
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }
    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_VIEW_ORDER, COMMAND_SEPARATOR, sessionId);
    int result = sendData(sendMessage, receiveMessage);

    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        // receive message
        replace_char(receiveMessage, '\n', '\0');
        
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        char *orders_str = strtok(NULL, ",\n\r");
        int status = atoi(status_str);
        if(orders_str==NULL){
            printf("Empty order!\n");
            goto SHOW_MENU;
        }
        switch (status)
        {
        case STATUS_OK:
            printf("View order success!\n");
            char *saveptr1, *saveptr2;
            // ลูปที่ 1: ตัดด้วยเครื่องหมาย '|' เพื่อแยกแต่ละรายวิชา
            char *order_token = strtok_r(orders_str, "|", &saveptr1);

            while (order_token != NULL)
            {
                printf("Processing Order: %s\n", order_token);

                // ลูปที่ 2: ตัดด้วยเครื่องหมาย '-' เพื่อแยกรายละเอียดภายในวิชานั้น
                // หมายเหตุ: ต้องใช้ copy ของ order_token หรือระวังเรื่องการเปลี่ยนแปลงค่า
                char *detail = strtok_r(order_token, "-", &saveptr2);
                int field_count = 0;

                while (detail != NULL)
                {
                    if (field_count == 0)
                        printf("  Username: %s\n", detail);
                    if (field_count == 1)
                        printf("  Product ID: %s\n", detail);
                    if (field_count == 2)
                        printf("  Quantity: %s\n", detail);
                    if (field_count == 3)
                        printf("  Checkout: %s\n", detail);

                    detail = strtok_r(NULL, "-", &saveptr2);
                    field_count++;
                }

                printf("-------------------\n");
                order_token = strtok_r(NULL, "|", &saveptr1);
            }
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleMemberLogout(){

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_LOGOUT,COMMAND_SEPARATOR, sessionId);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            userLevel = -1;
            printf("Logout success!\n");
            break;
        default:
            userLevel = -1;
            sessionId[0] = '\0';
            printf("An unknown error occurred (Code: %d)\n", status);
        }

        pressEnterToContinue();
        
    }

}
void handleAddProduct(){
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }

    // wait for input
    printf("\n--- Add product ---\n");
    printf("Product ID: ");
    char productId[100];
    getInput(productId, 100);

    printf("Product Title: ");
    char productTitle[100];
    getInput(productTitle, 100);

    printf("Price: \n");
    char price[100];
    getFloatInput(price, 100);

    printf("Quantity: ");
    char quantity[100];
    getNumericInput(quantity, 100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s%s%s%s%s%s%s\n", COMMAND_ADD_PRODUCT,COMMAND_SEPARATOR, sessionId, COMMAND_SEPARATOR, productId,COMMAND_SEPARATOR,productTitle, COMMAND_SEPARATOR,price,COMMAND_SEPARATOR, quantity);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Add product success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleUpdateProduct(){
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }

    // wait for input
    printf("\n--- Update product ---\n");
    printf("Product ID: ");
    char productId[100];
    getInput(productId, 100);

    printf("Product Title: ");
    char productTitle[100];
    getInput(productTitle, 100);

    if(strlen(productTitle)==0){
        strcpy(productTitle, IGNORE_UPDATE_TITLE);
    }

    printf("Price: \n");
    char price[100];
    getFloatInput(price, 100);

    if(strlen(price)==0){
        strcpy(price, IGNORE_UPDATE_PRICE_STR);
    }

    printf("Quantity: ");
    char quantity[100];
    getNumericInput(quantity, 100);

    if(strlen(quantity)==0){
        strcpy(quantity, IGNORE_UPDATE_QUANTITY_STR);
    }

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s%s%s%s%s%s%s\n", COMMAND_UPDATE_PRODUCT,COMMAND_SEPARATOR, sessionId, COMMAND_SEPARATOR, productId,COMMAND_SEPARATOR,productTitle, COMMAND_SEPARATOR,price,COMMAND_SEPARATOR, quantity);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Update product success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleRemoveProducts(){

    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }

    // wait for input
    printf("\n--- Remove products ---\n");
    printf("Product ID: ");
    char productId[100];
    getInput(productId, 100);

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s%s%s\n", COMMAND_REMOVE_PRODUCT,COMMAND_SEPARATOR, sessionId, COMMAND_SEPARATOR, productId);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            printf("Remove product success!\n");
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleAdminViewOrder(){
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }
    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_VIEW_ORDER,COMMAND_SEPARATOR,sessionId);
    int result = sendData(sendMessage, receiveMessage);

    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        // receive message
        replace_char(receiveMessage, '\n', '\0');
        
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        char *orders_str = strtok(NULL, ",\n\r");
        int status = atoi(status_str);
        if(orders_str==NULL){
            printf("Empty order!\n");
            goto SHOW_MENU;
        }
        switch (status)
        {
        case STATUS_OK:
            printf("View order success!\n");
            char *saveptr1, *saveptr2;
            // ลูปที่ 1: ตัดด้วยเครื่องหมาย '|' เพื่อแยกแต่ละรายวิชา
            char *order_token = strtok_r(orders_str, "|", &saveptr1);

            while (order_token != NULL)
            {
                printf("Processing Order: %s\n", order_token);

                // ลูปที่ 2: ตัดด้วยเครื่องหมาย '-' เพื่อแยกรายละเอียดภายในวิชานั้น
                // หมายเหตุ: ต้องใช้ copy ของ order_token หรือระวังเรื่องการเปลี่ยนแปลงค่า
                char *detail = strtok_r(order_token, "-", &saveptr2);
                int field_count = 0;

                while (detail != NULL)
                {
                    if (field_count == 0)
                        printf("  Username: %s\n", detail);
                    if (field_count == 1)
                        printf("  Product ID: %s\n", detail);
                    if (field_count == 2)
                        printf("  Quantity: %s\n", detail);
                    if (field_count == 3)
                        printf("  Checkout: %s\n", detail);

                    detail = strtok_r(NULL, "-", &saveptr2);
                    field_count++;
                }

                printf("-------------------\n");
                order_token = strtok_r(NULL, "|", &saveptr1);
            }
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleAdminViewMember(){
    if(sessionId==NULL){
        printf("You are not login! Please login\n");
        goto SHOW_MENU;
    }
    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_VIEW_MEMBER, COMMAND_SEPARATOR, sessionId);
    int result = sendData(sendMessage, receiveMessage);

    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        // receive message
        replace_char(receiveMessage, '\n', '\0');
        
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        char *members_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        if(members_str==NULL){
            printf("Empty member!\n");
            goto SHOW_MENU;
        }
        switch (status)
        {
        case STATUS_OK:
            printf("View member success!\n");
            char *saveptr1, *saveptr2;
            // ลูปที่ 1: ตัดด้วยเครื่องหมาย '|' เพื่อแยกแต่ละรายวิชา
            char *member_token = strtok_r(members_str, "|", &saveptr1);

            while (member_token != NULL)
            {
                printf("Processing Member: %s\n", member_token);

                // ลูปที่ 2: ตัดด้วยเครื่องหมาย '-' เพื่อแยกรายละเอียดภายในวิชานั้น
                // หมายเหตุ: ต้องใช้ copy ของ order_token หรือระวังเรื่องการเปลี่ยนแปลงค่า
                char *detail = strtok_r(member_token, "-", &saveptr2);
                int field_count = 0;

                while (detail != NULL)
                {
                    if (field_count == 0)
                        printf("  Username: %s\n", detail);
                

                    detail = strtok_r(NULL, "-", &saveptr2);
                    field_count++;
                }

                printf("-------------------\n");
                member_token = strtok_r(NULL, "|", &saveptr1);
            }
            break;
        default:
            printf("An unknown error occurred (Code: %d)\n", status);
        }
        SHOW_MENU:
            pressEnterToContinue();
        
    }
}
void handleAdminLogout(){

    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    printf("sessionid = %s\n", sessionId);
    snprintf(sendMessage,sizeof(sendMessage), "%s%s%s\n", COMMAND_LOGOUT,COMMAND_SEPARATOR, sessionId);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        if (status_str == NULL) {
            printf("Error: Server sent an invalid response format.\n");
            pressEnterToContinue();
            return; // ออกจากฟังก์ชันทันที ไม่ทำ atoi ต่อ
        }
        int status = atoi(status_str);
        switch (status)
        {
        case STATUS_OK:
            userLevel = -1;
            printf("Logout success!\n");
            break;
        default:
            userLevel = -1;
            sessionId[0] = '\0';
            printf("An unknown error occurred (Code: %d)\n", status);
        }

        pressEnterToContinue();
        
    }

}
void exitApp()
{
    printf("\nBye!\n");
    exit(0);
}
void showMenu(int userLevel)
{
    if (userLevel == 0)
    { // Member
        MenuEntry memberMenuTable[] = {
            {MENU_VIEW_PRODUCTS, handleViewProducts},
            {MENU_ADD_TO_CART, handleAddToCart},
            {MENU_VIEW_CART, handleViewCart},
            {MENU_REMOVE_CART, handleRemoveCart},
            {MENU_CHECKOUT_CART, handleCheckoutCart},
            {MENU_VIEW_ORDER, handleViewOrder},
            {MENU_MEMBER_LOGOUT, handleMemberLogout},
            {MENU_EXIT, exitApp},
            {NULL, NULL} // Sentinel to mark the end
        };

        char *memberMenu = "\nWelcome to our Shop:\n"
                              "\n--- Member Menu ---\n"
                              "[1] View Products\n"
                              "[2] Add to cart\n"
                              "[3] View cart\n"
                              "[4] Remove cart\n"
                              "[5] Checkout\n"
                              "[6] View order\n"
                              "[7] Logout\n"
                              "[0] Exit\n"
                              "Please select [0] - [4]: ";
        char menuName[ANSWER_SIZE];
        prompt(memberMenu, menuName);
        int found = 0;
        for (int i = 0; memberMenuTable[i].menuName != NULL; i++)
        {
            if (strcmp(menuName, memberMenuTable[i].menuName) == 0)
            {
                // Found the command! Execute its function.
                printf("Found menu: %s\n", menuName);
                found = 1;
                memberMenuTable[i].handler();
            }
        }
        if (!found)
        {
            return;
        }
    }
    else if (userLevel == 1)
    { // Admin
        MenuEntry adminMenuTable[] = {
            {MENU_ADMIN_ADD_PRODUCTS, handleAddProduct},
            {MENU_ADMIN_UPDATE_PRODUCTS, handleUpdateProduct},
            {MENU_ADMIN_REMOVE_PRODUCTS, handleUpdateProduct},
            {MENU_ADMIN_VIEW_ORDER, handleAdminViewOrder},
            {MENU_ADMIN_VIEW_MEMBER, handleAdminViewMember},
            {MENU_ADMIN_LOGOUT, handleAdminLogout},
            {MENU_EXIT, exitApp},
            {NULL, NULL} // Sentinel to mark the end
        };

        char *adminMenu = "\nWelcome to our Shop:\n"
                              "\n--- Admin Menu ---\n"
                              "[1] Add Products\n"
                              "[2] Update Products\n"
                              "[3] Remove Products\n"
                              "[4] View Order\n"
                              "[5] View Member\n"
                              "[6] Logout\n"
                              "[0] Exit\n"
                              "Please select [0] - [6]: ";
        char menuName[ANSWER_SIZE];
        prompt(adminMenu, menuName);
        int found = 0;
        for (int i = 0; adminMenuTable[i].menuName != NULL; i++)
        {
            if (strcmp(menuName, adminMenuTable[i].menuName) == 0)
            {
                // Found the command! Execute its function.
                printf("Found menu: %s\n", menuName);
                found = 1;
                adminMenuTable[i].handler();
            }
        }
        if (!found)
        {
            return;
        }
    }
    else
    { // Anonymous

        MenuEntry anonymousMenuTable[] = {
            {MENU_VIEW_PRODUCTS, handleViewProducts},
            {MENU_SEARCH_PRODUCTS, handleSearchProducts},
            {MENU_LOGIN, handleLogin},
            {MENU_REGISTER, handleRegister},
            {MENU_EXIT, exitApp},
            {NULL, NULL} // Sentinel to mark the end
        };

        char *anonymousMenu = "\nWelcome to our Shop:\n"
                              "\n--- Anonymous Menu ---\n"
                              "[1] View Products\n"
                              "[2] Search products\n"
                              "[3] Login\n"
                              "[4] Register\n"
                              "[0] Exit\n"
                              "Please select [0] - [4]: ";
        char menuName[ANSWER_SIZE];
        prompt(anonymousMenu, menuName);
        int found = 0;
        for (int i = 0; anonymousMenuTable[i].menuName != NULL; i++)
        {
            if (strcmp(menuName, anonymousMenuTable[i].menuName) == 0)
            {
                // Found the command! Execute its function.
                printf("Found menu: %s\n", menuName);
                found = 1;
                anonymousMenuTable[i].handler();
            }
        }
        if (!found)
        {
            return;
        }
    }
}

int main()
{
    while(1){
        showMenu(userLevel);
    }
    
    return 0;
}