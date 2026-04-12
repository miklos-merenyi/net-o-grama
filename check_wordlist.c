#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

// From engine.c
size_t utf8_strlen(const char *s) {
    size_t len = 0;
    for (int i = 0; s[i]; i++)
        if ((s[i] & 0xC0) != 0x80) len++;
    return len;
}

int has_accent(const char *s) {
    unsigned char c;
    for (int i = 0; s[i]; i++) {
        c = (unsigned char)s[i];
        if (c >= 0xC0) return 1;  // Multi-byte UTF-8 char
    }
    return 0;
}

int main() {
    FILE *f = fopen("wordlist.hu", "r");
    if (!f) { perror("open"); return 1; }
    
    setlocale(LC_ALL, "");
    
    char line[200];
    int total = 0, len7 = 0, len7_accent = 0;
    
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        total++;
        if (utf8_strlen(line) == 7) {
            len7++;
            if (has_accent(line)) {
                len7_accent++;
                if (len7_accent <= 20) printf("%-20s (accented)\n", line);
            }
        }
    }
    
    printf("\nStats: Total words: %d, 7-char: %d, 7-char with accents: %d\n", 
           total, len7, len7_accent);
    
    fclose(f);
    return 0;
}
