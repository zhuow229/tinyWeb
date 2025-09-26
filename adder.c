#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *query_string = getenv("QUERY_STRING");
    int x = 0, y = 0;

    if (query_string != NULL) {
        sscanf(query_string, "x=%d&y=%d", &x, &y);
    }

    printf("Content-type: text/html\r\n\r\n");
    printf("<html><head><title>Adder Result</title></head>\n");
    printf("<body>\n");
    printf("<h1>Adder Result</h1>\n");
    printf("<p>%d + %d = %d</p>\n", x, y, x + y);
    printf("<a href=\"/home.html\">Back</a>\n");
    printf("</body></html>\n");

    return 0;
}
