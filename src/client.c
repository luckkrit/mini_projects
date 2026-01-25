#include "client.h"

int userLevel = -1;
char *sessionId = NULL;

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
    sprintf(sendMessage, "%s%s%s\n", COMMAND_SEARCH_PRODUCT, COMMAND_SEPARATOR, productId);
    int result = sendData(sendMessage, receiveMessage);

    if (result != 0)
    {
        pressEnterToContinue();
        showMenu(userLevel);
    }
    else
    {
        // receive message
        replace_char(receiveMessage, '\n', '\0');
        
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
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
            showMenu(userLevel);
    }
}

void handleViewProducts()
{
    // send message
    char sendMessage[SEND_MESSAGE_SIZE];
    char receiveMessage[RECEIVE_MESSAGE_SIZE];
    sprintf(sendMessage, "%s\n", COMMAND_VIEW_PRODUCT);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        showMenu(userLevel);
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
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
            showMenu(userLevel);
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
    sprintf(sendMessage, "%s%s%s%s%s\n", COMMAND_LOGIN,COMMAND_SEPARATOR, username, COMMAND_SEPARATOR, password);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        showMenu(userLevel);
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        sessionId = strtok(NULL, ",\n\r");
        char *isAdmin = strtok(NULL, ",\n\r");

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
        showMenu(userLevel);
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
    sprintf(sendMessage, "%s%s%s%s%s\n", COMMAND_REGISTER_MEMBER,COMMAND_SEPARATOR, username, COMMAND_SEPARATOR, password);
    int result = sendData(sendMessage, receiveMessage);
    if (result != 0)
    {
        pressEnterToContinue();
        showMenu(userLevel);
    }
    else
    {
        replace_char(receiveMessage, '\n', '\0');

        // receive message
        strtok(receiveMessage, ",\n\r");
        char *status_str = strtok(NULL, ",\n\r");
        
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
        showMenu(userLevel);
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
    }
    else if (userLevel == 1)
    { // Admin
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
            showMenu(userLevel);
        }
    }
}

int main()
{
    showMenu(userLevel);
    return 0;
}