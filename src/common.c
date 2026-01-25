#include "common.h"

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}
int isNumeric(const char *str) {
    if (str == NULL || *str == '\0') return 0;

    int i = 0;
    // ตรวจสอบเครื่องหมายลบที่ตำแหน่งแรก
    if (str[0] == '-') {
        // ถ้ามีแค่เครื่องหมายลบตัวเดียวแบบ "-" ให้ถือว่าไม่ใช่ตัวเลข
        if (str[1] == '\0') return 0;
        i = 1; // เริ่มตรวจตัวเลขที่ตำแหน่งถัดไป
    }

    for (; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0; 
        }
    }
    return 1;
}