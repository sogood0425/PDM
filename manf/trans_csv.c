#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE *csv_file = fopen("AJT.csv", "r");
    FILE *output_file = fopen("input.data", "w");

    if (csv_file == NULL || output_file == NULL) {
        printf("Error opening files.\n");
        return 1;
    }

    char line[1024];

    // Skip the first line (header)
    fgets(line, sizeof(line), csv_file);

    while (fgets(line, sizeof(line), csv_file)) {
        char *ptr = line;
        int len = strlen(line);

        if (line[len - 1] == '\n') {
            line[len - 1] = '\0'; // Remove newline character
            len--;
        }

        int i;
        for (i = 0; i < len; i++) {
            if (line[i] == ',') {
                line[i] = '\0';
            }
        }

        ptr = line;
        while (ptr < line + len) {
            fprintf(output_file, "%s\n", ptr);
            ptr += strlen(ptr) + 1;
        }

        // Handle trailing empty fields if any
        while (ptr <= line + len) {
            fprintf(output_file, "\n");
            ptr++;
        }
    }

    fclose(csv_file);
    fclose(output_file);

    return 0;
}
