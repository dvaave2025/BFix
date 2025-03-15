#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a struct for a book with a flexible array member for authors
struct Book {
    char title[100];
    int year;
    char *authors[]; // Flexible array member for authors
};

int main(int argc) {
    // Allocate memory for a single struct Book with space for 2 authors
    struct Book *book = (struct Book *)malloc(sizeof(struct Book) + argc * sizeof(char *));

    if (book == NULL) {
        printf("Memory allocation failed. Exiting...\n");
        return 1;
    }

    // Input book details
    strcpy(book->title, "XXX");
	char *name = "QQQ";
	book->year = 1002;
    book->authors[0] = (char *)malloc((strlen(name) + 1) * sizeof(char));
    strcpy(book->authors[0], name);

    // Print book details
    printf("\nBook Details:\n");
    printf("Title: %s\n", book->title);
    printf("Year of publication: %d\n", book->year);
    printf("Authors: %s\n", book->authors[0]);
    // Free dynamically allocated memory
    //free(book->authors[0]);
    free(book);

    return 0;
}

