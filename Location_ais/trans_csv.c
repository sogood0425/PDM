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

    char line[100];
    char *token, *next_token;

    // Skip the first line (header)
    fgets(line, sizeof(line), csv_file);

    while (fgets(line, sizeof(line), csv_file)) {
        // Tokenize the line using comma as delimiter
        token = strtok(line, ",");
        while (token != NULL) {
            // Get next token before writing the current one
            next_token = strtok(NULL, ",");
            // Write token to output file
            if (next_token != NULL) {
                fprintf(output_file, "%s\n", token);
            } else {
                fprintf(output_file, "%s", token);
            }
            // Move to the next token
            token = next_token;
        }
    }

    fclose(csv_file);
    fclose(output_file);

    return 0;
}