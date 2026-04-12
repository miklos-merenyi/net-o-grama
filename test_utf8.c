#include <stdio.h>
#include <string.h>
#include <locale.h>

// From engine.c
size_t utf8_strlen(const char *s) {
    size_t len = 0;
    for (int i = 0; s[i]; i++)
        if ((s[i] & 0xC0) != 0x80) len++;
    return len;
}

int main() {
    setlocale(LC_ALL, "");
    
    char test_words[] = "abcúgol\nabiogén\nabortál\ngarázs\nháló\nhétvége\n";
    char *p = test_words;
    char line[100];
    
    while (sscanf(p, "%99[^\n]", line) == 1) {
        printf("Word: %-15s Bytes: %zu  Chars: %zu\n", line, strlen(line), utf8_strlen(line));
        p = strchr(p, '\n');
        if (!p) break;
        p++;
    }
    
    return 0;
}
