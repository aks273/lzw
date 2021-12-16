#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************************
  
                                LZW DECOMPRESSION

 Algorithm:
     -  Read in the compressed file, which consists of 12 bit codes, three bits
        (two codes) at a time
     -  When we read a code, add the previous code plus the first character of
        this code to the dictionary
     -  When the dictionary gets full, reset it

This program prints the decompressed output to stdout. This can be piped into
an output file if needs be.

**********************************************************************************/

void reset_dictionary(char ** dict) {
    for (int i = 256; i < 4096; i++) {
        free(dict[i]);
    }
}

// Find the size of the file by seeking to the end. Reuse the file pointer
// for the compression algorithm by seeking back to the beginning again.
int get_file_size(FILE* file) {
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file_size;
}

int add_to_dictionary(char** twelve_bit_dictionary, int* file_buffer, int file_buffer_index, int dictionary_index) {
    if (file_buffer_index > 1) {
        int prev_code = file_buffer[file_buffer_index - 2];
        int curr_code = file_buffer[file_buffer_index - 1];

        int prev_code_len = strlen(twelve_bit_dictionary[prev_code]);
        twelve_bit_dictionary[dictionary_index] 
            = malloc((prev_code_len + 2) * sizeof(char));

        // Next dictionary element = previous code + first char of the current
        strcpy(
            twelve_bit_dictionary[dictionary_index],
            twelve_bit_dictionary[prev_code]
        );
        twelve_bit_dictionary[dictionary_index][prev_code_len] 
            = twelve_bit_dictionary[curr_code][0];
        twelve_bit_dictionary[dictionary_index][prev_code_len + 1] = '\0';

        dictionary_index++;
        if (dictionary_index == 4096) {
            reset_dictionary(twelve_bit_dictionary);
            dictionary_index = 256;
        }
    }
    return dictionary_index;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Incorrect usage!\n");
        printf("Please enter the path of the file to decompress\n");
        return 1;
    }

    FILE* input_file;
    if ((input_file = fopen(argv[1], "r")) == NULL) {
        printf("Incorrect usage!");
        printf("Please enter a valid file path\n");
        return 1;
    }

    int file_size = get_file_size(input_file);

    printf("Size: %d\n", file_size);

    // Initial setup of the dictionary
    // We use 4096 because this is the max number (2^12) of dictionary elements
    char** twelve_bit_dictionary = malloc(4096 * sizeof(char*));

    // Prefill the regular 8 bit codes into the dictionary.
    for (int i = 0; i < 256; i++) {
        // 2 because each of these twelve bit codes represent a string of length 1
        twelve_bit_dictionary[i] = malloc(2 * sizeof(char));
        twelve_bit_dictionary[i][0] = i;
        twelve_bit_dictionary[i][1] = '\0';
    }

    // Reading of the file
    // We parse the file three bits at a time, and try to update the dictionary
    // after each 12 bit code that we parse.
    int next_bit;
    int bit_count = 0;
    int next_three_bits[3];

    // We store the input file in a buffer for ease of adding the codes into the
    // dictionary (so we can look backwards into the file). We could probably
    // just store the dictionary index of the last 12 bit code here instead.
    // Allocating 2 * the file size means that this will definitely be big enough.
    int* file_buffer = malloc(file_size * 2 * sizeof(int));
    int file_buffer_index = 0;

    // Store a dictionary index so we know when to reset the dictionary (when
    // it gets to 4096).
    int dictionary_index = 256;
    
    while ((next_bit = fgetc(input_file)) != EOF) {
        next_three_bits[bit_count % 3] = next_bit;
        bit_count++;

        if (bit_count % 3 == 0) {
            // Extract and print the next 8 bit character from the twelve bit code
            file_buffer[file_buffer_index] 
                = (next_three_bits[0] << 4) | (next_three_bits[1] >> 4);
            printf("%s", twelve_bit_dictionary[file_buffer[file_buffer_index]]);
            file_buffer_index++;

            // Try to add a new element to the dictionary
            dictionary_index = add_to_dictionary(
                twelve_bit_dictionary, file_buffer, file_buffer_index, dictionary_index
            );

            // We repeat here because we are examining two twelve bit codes
            file_buffer[file_buffer_index] 
                = ((next_three_bits[1] & 0x0f) << 8) | next_three_bits[2];
            printf("%s", twelve_bit_dictionary[file_buffer[file_buffer_index]]);
            file_buffer_index++;

            dictionary_index = add_to_dictionary(
                twelve_bit_dictionary, file_buffer, file_buffer_index, dictionary_index
            );

            // Reset the next_three_bits store for the next two codes
            next_three_bits[0] = EOF;
            next_three_bits[1] = EOF;
            next_three_bits[2] = EOF;
        }
    }

    // Handle edge case if we end with only one twelve bit code
    if (next_three_bits[0] != EOF) {
            file_buffer[file_buffer_index] 
                = (next_three_bits[0] << 4) | (next_three_bits[1] >> 4);
            printf("%s", twelve_bit_dictionary[file_buffer[file_buffer_index]]);
            file_buffer_index++;
    }

    // Cleanup
    fclose(input_file);
    free(file_buffer);
    for (int i = 0; i < dictionary_index; i++) {
        free(twelve_bit_dictionary[i]);
    }
    free(twelve_bit_dictionary);
}