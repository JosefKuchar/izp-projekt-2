/**
 * @name IZP Projekt 2 - Prace s datovymi strukturami
 * @author Josef Kuchar - xkucha28
 * 2021
 */

#include <stdbool.h>  // Bool type
#include <stdio.h>    // IO functions
#include <stdlib.h>   // EXIT macros
#include <string.h>

// Define string length hard limits
#define MAX_STRING_LENGTH 30
#define STRING_BUFFER_SIZE MAX_STRING_LENGTH + 1  // +1 is for \0

enum store_node_type { UNIVERZUM, SET, RELATION };

// Struct to keep track of univerzum
struct univerzum {
    int size;
    char (*nodes)[STRING_BUFFER_SIZE];
};

// Struct to keep track of one set
struct set {
    int size;
    int* nodes;
};

// Struct to keep track of on node inside relation
struct relation_node {
    int a;
    int b;
};

// Struct to keep track of one relation
struct relation {
    int size;
    struct relation_node nodes[];
};

struct store_node {
    enum store_node_type type;
    void* obj;
};

/**
 * Print bool value
 * @param b bool to be printed
 */
void print_bool(bool b) {
    if (b) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}

/**
 * Function to compare two numbers - for qsort
 * @param a Pointer to first number
 * @param b Pointer to second number
 * @return Differece between two numbers
 */
int compare_num_nodes(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

/**
 * Sort set
 * @param a Set
 */
void set_sort(struct set* a) {
    qsort(a->nodes, a->size, sizeof(int), compare_num_nodes);
}

/**
 * Print univerzum
 * @param u Univerzum
 */
void print_univerzum(struct univerzum* u) {
    // Indicate we are printing univerzum
    printf("U");
    // Loop around all nodes inside univerzum
    for (int i = 0; i < u->size; i++) {
        // Print each node inside univerzum
        printf(" %s", u->nodes[i]);
    }
    printf("\n");
}

/**
 * Print set
 * @param a Set
 * @param u Univerzum
 */
void print_set(struct set* a, struct univerzum* u) {
    // Indicate we are printing set
    printf("S");
    // Loop around all nodes inside set
    for (int i = 0; i < a->size; i++) {
        // Print each node inside set
        printf(" %s", u->nodes[a->nodes[i]]);
    }
    printf("\n");
}

/**
 * Print relation
 * @param r Relation
 * @param u Univerzum
 */
void print_relation(struct relation* r, struct univerzum* u) {
    // Indicate we are printing relation
    printf("R");
    // Loop around all nodes inside relation
    for (int i = 0; i < r->size; i++) {
        // Print each node inside relation
        printf(" (%s, %s)", u->nodes[r->nodes->a], u->nodes[r->nodes->b]);
    }
    printf("\n");
}

void set_empty(struct set* a) {
    print_bool(a->size == 0);
}

/**
 * Prints size of set
 * @param a Set
 */
void set_card(struct set* a) {
    printf("%d\n", a->size);
}

/*
void set_complement(struct set* a, struct univerzum* u) {
    // TODO
}

void set_union(struct set* a, struct set* b) {
    // TODO
}

void set_intersect(struct set* a, struct set* b) {
    // TODO
}

void set_minus(struct set* a, struct set* b) {
    // TODO
}

void set_subseteq(struct set* a, struct set* b) {
    // TODO
}

void set_subset(struct set* a, struct set* b) {
    // TODO
}
*/

/**
 * Compare two sets
 * @param a First set - sorted
 * @param b Second set - sorted
 * @return True if sets are equal
 */
bool set_equals(struct set* a, struct set* b) {
    // If sets have different sizes they are different
    if (a->size != b->size) {
        return false;
    }

    // Loop around all nodes
    for (int i = 0; i < a->size; i++) {
        // If any nodes differ sets are different
        if (a->nodes[i] != b->nodes[i]) {
            return false;
        }
    }

    return true;
}
/*
void relation_reflexive(struct relation* r) {
    // TODO
}

void relation_symmetric(struct relation* r) {
    // TODO
}

void relation_antisymmetric(struct relation* r) {
    // TODO
}

void relation_transitive(struct relation* r) {
    // TODO
}

void relation_function(struct relation* r) {
    // TODO
}

void relation_domain(struct relation* r) {
    // TODO
}

void relation_codomain(struct relation* r) {
    // TODO
}

void relation_injective(struct relation* r) {
    // TODO
}

void relation_surjective(struct relation* r) {
    // TODO
}

void relation_bijective(struct relation* r) {
    // TODO
}
*/

/**
 * Free univerzum struct
 * @param u Univerzum
 */
void free_univerzum(struct univerzum* u) {
    free(u->nodes);
}

/**
 * Free set struct
 * @param s Set
 */
void free_set(struct set* s) {
    free(s->nodes);
}

/**
 * Free relatino struct
 * @param r Relation
 */
void free_relation(struct relation* r) {
    free(r->nodes);
}

/**
 * Free store from memory including all children
 * @param store Pointer to store
 * @param size Size of store
 */
void free_store(struct store_node* store, int size) {
    // Free all store nodes based on their type
    for (int i = 0; i < size; i++) {
        switch (store[i].type) {
            case UNIVERZUM:
                free_univerzum(store[i].obj);
                break;
            case SET:
                free_set(store[i].obj);
                break;
            case RELATION:
                free_relation(store[i].obj);
                break;
            default:
                // TODO: Handle this, it shouldn't happen tho
                break;
        }
    }

    // Free store itself
    free(store);
}

/**
 * Parse univerzum from file stream
 * @param fp File pointer
 */
void parse_univerzum(FILE* fp) {
    struct univerzum u;
    // Allocate memory for 10 strings for now
    u.nodes = calloc(sizeof(*u.nodes), 10);
    u.size = 1;

    int index = 0;

    while (true) {
        int c = getc(fp);
        // If character is EOF or newline we can end parsing
        if (c == EOF || c == '\n') {
            // TODO: Return the actual univerzum
            break;
        }

        if (c == ' ') {
            u.size++;
            index = 0;
            continue;
        }

        u.nodes[u.size - 1][index] = c;
        index++;
    }

    print_univerzum(&u);
    free_univerzum(&u);
}

/**
 * Process all lines in file
 * @param fp File pointer
 * @return True if everything went well
 */
bool process_file(FILE* fp) {
    // Allocate enough memory for store
    struct store_node* store = malloc(sizeof(struct store_node) * 1000);
    int store_size = 0;
    if (store == NULL) {
        fprintf(stderr, "Malloc error!\n");
        return false;
    }

    int c = 0;
    // Loop around all chars
    while ((c = getc(fp)) != EOF) {
        // FIXME for blank things (univerzum for example)
        getc(fp);

        switch (c) {
            case 'U':
                // TODO parse univerzum better
                parse_univerzum(fp);
                break;
            case 'S':
                // TODO parse set
                break;
            case 'R':
                // TODO parse relation
                break;
            case 'C':
                // TODO parse command
                break;
            default:
                // TODO handle other characters
                break;
        }
    }

    // Free store from memory
    free_store(store, store_size);

    return true;
}

/**
 * Open file
 * @param filename File name
 * @return File pointer if file was opened successfully, NULL otherwise
 */
FILE* open_file(char* filename) {
    FILE* fp;
    fp = fopen(filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "Nepodarilo se otevrit soubor!\n");
        return NULL;
    }

    return fp;
}

/**
 * Close file
 * @param fp File pointer
 * @return True if file was successfully closed
 */
bool close_file(FILE* fp) {
    if (fclose(fp) == EOF) {
        fprintf(stderr, "Nepodarilo se uzavrit soubor!\n");
        return false;
    } else {
        return true;
    }
}

/**
 * Check number of arguments
 * @param argc Number of arguments
 * @return True if number of arguments is correct
 */
bool check_arguments(int argc) {
    if (argc != 2) {
        fprintf(stderr, "Nespravny pocet argumentu!\n");
        return false;
    } else {
        return true;
    }
}

int main(int argc, char* argv[]) {
    // Check number of arguments
    if (!check_arguments(argc)) {
        return EXIT_FAILURE;
    }

    // Open file
    FILE* fp = open_file(argv[1]);
    if (fp == NULL) {
        return EXIT_FAILURE;
    }

    // Process file
    if (!process_file(fp)) {
        return EXIT_FAILURE;
    }

    // Close file
    if (!fclose(fp)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
