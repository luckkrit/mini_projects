#include "store_controller.h"

// ฟังก์ชันภายใน (Helper) สำหรับแปลง String เป็น ID ตัวเลข
// ใช้ static เพื่อให้เรียกได้เฉพาะภายในไฟล์นี้ ไม่รกข้างนอก
static unsigned long parse_sid(char *sidStr) {
    if (sidStr == NULL) return 0;
    char *endptr;
    unsigned long sid = strtoul(sidStr, &endptr, 10);
    if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r') return 0;
    return sid;
}

User* ctrl_Authorize(UserSessions *sessions, char *sidStr, int requireAdmin, int *outStatus) {
    unsigned long sid = parse_sid(sidStr);
    if (sid == 0) {
        if (outStatus) *outStatus = STATUS_INVALID_SESSION;
        return NULL;
    }

    User *u = getUserBySession(sessions, sid);
    if (u == NULL) {
        if (outStatus) *outStatus = STATUS_INVALID_SESSION;
        return NULL;
    }

    if (requireAdmin && !u->isAdmin) {
        if (outStatus) *outStatus = STATUS_PERMISSION_DENIED;
        return NULL;
    }

    if (outStatus) *outStatus = STATUS_OK;
    return u;
}

// 1. สำหรับ Admin: เพิ่มสินค้าใหม่เท่านั้น
int ctrl_AddProduct(Store *s, char *id, char *title, float price, int qty) {
    if (id == NULL) return STATUS_INVALID_ARGUMENTS;
    // เราใช้ updateStore ของคุณได้เลย เพราะมันจะไปจบที่ addProduct ให้อัตโนมัติถ้าไม่มี ID เดิม
    // แต่เราเพิ่มกฎ: ถ้ามีอยู่แล้ว ห้ามแอดซ้ำ
    if (findProductById(s, id) != NULL) return STATUS_DUPLICATE_PRODUCT; 

    int res = updateStore(s, id, title, price, qty);
    if (res == 0) {
        int res2 = saveStore(s, STORE_FILENAME);
        if(res2 == 0){
            return STATUS_OK;
        }else{
            return STATUS_SAVE_STORE_FAIL;
        }

    }
    return STATUS_ADD_PRODUCT_FAIL;
}

// 2. สำหรับ Admin: แก้ไขข้อมูลสินค้า (ไม่ใช่การบวกเพิ่มสต็อก แต่เป็นการ Set ค่าใหม่)
int ctrl_UpdateProduct(Store *s, char *id, char *title, float price, int qty) {
    if (id == NULL) return STATUS_INVALID_ARGUMENTS;
    if (findProductById(s, id) == NULL) return STATUS_PRODUCT_NOT_FOUND; 
    // เราเลือกใช้ updateProduct (ฟังก์ชันแรกใน store.c) เพราะมันเป็นการ Set ค่าทับ
    int res = updateProduct(s, id, title, price, qty);
    if (res == 0){
        int res2 = saveStore(s, STORE_FILENAME);
        if(res2==0){
            return STATUS_OK;
        }else{
            return STATUS_SAVE_STORE_FAIL;
        }
    } 
    return STATUS_UPDATE_PRODUCT_FAIL;
}

int ctrl_DeleteProduct(Store *s, char *id){
    if (id == NULL) return STATUS_INVALID_ARGUMENTS;
    if (findProductById(s, id) == NULL) return STATUS_PRODUCT_NOT_FOUND; 
    int res = deleteProduct(s, id);
    if (res == 0){
        int res2 = saveStore(s, STORE_FILENAME);
        if(res2==0){
            return STATUS_OK;
        }else{
            return STATUS_SAVE_STORE_FAIL;
        }
    } 
    return STATUS_DELETE_PRODUCT_FAIL;
}



// 3. สำหรับ Member: เพิ่มของลงตะกร้า (หักสต็อก)
int ctrl_ReserveStock(Store *s, char *id, int qty) {
    // ใช้ updateStore โดยส่งค่า qty ติดลบ เพื่อให้บรรทัด store->items[i].quantity += quantity ทำงาน
    // และฟังก์ชันคุณมีเช็ค if (quantity + quantity < 0) return 1; อยู่แล้ว... ปลอดภัยมาก!
    int res = updateStore(s, id, IGNORE_UPDATE_TITLE, IGNORE_UPDATE_PRICE, -qty);
    if (res == 0) saveStore(s, STORE_FILENAME);
    return res;
}

// 4. สำหรับ Member: คืนของเข้า Store (บวกสต็อกกลับ)
int ctrl_ReturnStock(Store *s, char *id, int qty) {
    // ส่งค่าบวกปกติเข้าไปที่ updateStore
    int res = updateStore(s, id, IGNORE_UPDATE_TITLE, IGNORE_UPDATE_PRICE, qty);
    if (res == 0) saveStore(s, STORE_FILENAME);
    return res;
}

// ตัวอย่างแบบง่าย ถ้าไม่อยากแก้ store.c
static Stock* findProductById(Store *s, char *id) {
    for (int i = 0; i < MAX_STOCK; i++) {
        if (s->items[i].isUsed && strcmp(s->items[i].productId, id) == 0) {
            return &s->items[i];
        }
    }
    return NULL;
}