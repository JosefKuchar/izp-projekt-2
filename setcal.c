/**
 * @name IZP Projekt 2 - Prace s datovymi strukturami
 * @author Josef Kuchar - xkucha28
 * @author Martin Hemza - xhemza05
 * 2021
 */

#include <ctype.h>    // Char functions
#include <stdbool.h>  // Bool type
#include <stdio.h>    // IO functions
#include <stdlib.h>   // EXIT macros
#include <string.h>   // String manipulation functions

// Define string length hard limits
#define MAX_STRING_LENGTH 30
#define STRING_BUFFER_SIZE MAX_STRING_LENGTH + 1  // +1 is for \0

enum store_node_type { UNIVERZUM, SET, RELATION, COMMAND };

enum command_type { EMPTY };

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
    struct relation_node* nodes;
};

// Struct to keep track of one command
struct command {
    enum command_type type;  // Command type
    int a;                   // First operand (set or relation)
    int b;                   // Second operand
    int n;                   // Line number for bonus
};

// Struct to keep track of one node inside relation
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
 * Function to compare two relation nodes
 * @param a Pointer to first relation node
 * @param b Pointer to second relation node
 * @return Difference between two relation nodes
 */
int compare_relation_nodes(const void* a, const void* b) {
    // Cast void pointers to struct pointers
    const struct relation_node* x = a;
    const struct relation_node* y = b;
    // Find difference between first numbers
    int diff_a = x->a - y->a;
    // If difference is not 0 we can return
    if (diff_a != 0) {
        return diff_a;
    }
    // If diff between first numbers was 0 return diff between second numbers
    return x->b - y->b;
}

/**
 * Sort set
 * @param s Set
 */
void set_sort(struct set* s) {
    qsort(s->nodes, s->size, sizeof(int), compare_num_nodes);
}

/**
 * Sort relation
 * @param r Relation
 */
void relation_sort(struct relation* r) {
    qsort(r->nodes, r->size, sizeof(struct relation_node),
          compare_relation_nodes);
}

/**
 * Check if univerzum is valid (doesn't contain reserved words, same words)
 * @param u Univerzum
 * @return True if univerzum is valid
 */
bool univerzum_valid(struct univerzum* u) {
    // Define all illegal words inside univerzum
    const char* illegal[] = {
        "empty",       "card",          "complement", "union",
        "intersect",   "minus",         "subseteq",   "subset",
        "equals",      "reflexive",     "symmetric",  "antisymmetric",
        "transitive",  "function",      "domain",     "codomain",
        "injective",   "surjective",    "bijective",  "closure_ref",
        "closure_sym", "closure_trans", "select",     "true",
        "false"};

    // Loop around all elements inside univerzum
    for (int i = 0; i < u->size; i++) {
        // Loop around all illegal words
        // TODO: Remove hardcoded 25
        for (int j = 0; j < 25; j++) {
            if (strcmp(u->nodes[i], illegal[j]) == 0) {
                return false;
            }
        }
        // Check for repeated word
        for (int j = i + 1; j < u->size; j++) {
            if (strcmp(u->nodes[i], u->nodes[j]) == 0) {
                return false;
            }
        }
    }
    // Univerzum is valid if everything went well
    return true;
}

/**
 * Check if set is valid
 * @param s Set - sorted
 * @return True if set is valid
 */
bool set_valid(struct set* a) {
    // Loop around all elements inside set
    for (int i = 1; i < a->size; i++) {
        // If last item is same as current then this set is invalid
        if (a->nodes[i] == a->nodes[i - 1]) {
            return false;
        }
    }
    // If we didn't find two same elements then this set is valid
    return true;
}

/**
 * Check if relation is valid
 * @param r Relation - sorted
 * @return True if relation is valid
 */
bool relation_valid(struct relation* r) {
    // Loop around all elements inside relation
    for (int i = 1; i < r->size; i++) {
        // If last item is same as current then this relation is invalid
        if (r->nodes[i].a == r->nodes[i - 1].a &&
            r->nodes[i].b == r->nodes[i - 1].b) {
            return false;
        }
    }
    // If we didn't find two same relation nodes then this set is valid
    return true;
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
        printf(" (%s %s)", u->nodes[r->nodes[i].a], u->nodes[r->nodes[i].b]);
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

struct set* set_complement(struct set* a, struct univerzum* u) {
    // Allocate memory complement set
    struct set* complement = malloc(sizeof(struct set));
    complement->nodes = malloc(sizeof(u) - sizeof(a));
    complement->size = 0;

    int k = 0;
    for (int i = 0; i <= a->size; i++) {
        while (k < u->size) {
            if (a->nodes[i] == k) {
                k = a->nodes[i] + 1;
                break;
            }
            complement->nodes[complement->size++] = k;
            k++;
        }
    }
    return complement;
}

struct set* set_union(struct set* a, struct set* b) {
    // Allocate memory union set
    struct set* s_union = malloc(sizeof(struct set));
    s_union->nodes = malloc(sizeof(int));
    s_union->size = 0;

    int i = 0, k = 0;
    while (i < a->size || k < b->size) {
        s_union->nodes =
            realloc(s_union->nodes, sizeof(int) * (s_union->size + 1));
        if (i > a->size) {
            s_union->nodes[s_union->size] = b->nodes[k++];
        } else if (k > b->size) {
            s_union->nodes[s_union->size] = a->nodes[i++];
        } else if (a->nodes[i] < b->nodes[k]) {
            s_union->nodes[s_union->size] = a->nodes[i++];
        } else if (a->nodes[i] == b->nodes[k]) {
            s_union->nodes[s_union->size] = a->nodes[i++];
            k++;
        } else {
            s_union->nodes[s_union->size] = b->nodes[k++];
        }
        s_union->size++;
    }
    return s_union;
}

struct set* set_intersect(struct set* a, struct set* b) {
    // TODO
    struct set* intersect = malloc(sizeof(struct set));
    intersect->nodes = malloc(sizeof(int));
    intersect->size = 0;

    int i = 0, k = 0;
    while (i < a->size && k < b->size) {
        if (a->nodes[i] == b->nodes[k]) {
            intersect->nodes =
                realloc(intersect->nodes, sizeof(int) * (intersect->size + 1));
            intersect->nodes[intersect->size++] = a->nodes[i++];
            k++;
        } else if (a->nodes[i] < b->nodes[k]) {
            i++;
        } else {
            k++;
        }
    }
    return intersect;
}
/*

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
    free(u);
}

/**
 * Free set struct
 * @param s Set
 */
void free_set(struct set* s) {
    free(s->nodes);
    free(s);
}

/**
 * Free relatino struct
 * @param r Relation
 */
void free_relation(struct relation* r) {
    free(r->nodes);
    free(r);
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
 * @param u Univerzum
 * @return True if everything went well
 */
bool parse_univerzum(FILE* fp, struct univerzum* u) {
    // Allocate memory for 1 node
    u->nodes = calloc(sizeof(u->nodes), 1);
    u->size = 1;

    int index = 0;

    while (true) {
        int c = getc(fp);
        // If character is EOF or newline we can end parsing
        if (c == EOF || c == '\n') {
            // TODO: Return the actual univerzum
            break;
        } else if (c == ' ') {
            u->size++;
            index = 0;
            u->nodes = realloc(u->nodes, sizeof(*u->nodes) * u->size);
            memset(u->nodes[u->size - 1], 0, STRING_BUFFER_SIZE);
            continue;
        } else if (!isalpha(c)) {
            // TODO: Handle when character is not alphanumeric
        }

        u->nodes[u->size - 1][index] = c;
        index++;
    }

    print_univerzum(u);
    return true;
}

/**
 * Parse set from file stream
 * @param fp File pointer
 * @param u Univerzum
 * @return True if everything went well
 * */
bool parse_set(FILE* fp, struct set* s, struct univerzum* u) {
    // Allocate memory for one node
    s->nodes = malloc(sizeof(int));
    s->size = 0;

    char node[STRING_BUFFER_SIZE] = {0};
    int index = 0;

    while (true) {
        int c = getc(fp);

        if (c == ' ' || c == EOF || c == '\n') {
            s->size++;
            node[index] = '\0';
            index = 0;
            // Allocate memory for next node
            s->nodes = realloc(s->nodes, sizeof(int) * s->size + 1);

            // Compares set node to univerzum node
            int max = u->size;
            for (int i = 0; i < max; i++) {
                if (!(strcmp(node, u->nodes[i]))) {
                    s->nodes[s->size - 1] = i;
                    break;
                }
                // If the iteration is the last one => set node wasn't found in
                // univerzum
                if (i == max - 1) {
                    fprintf(stderr, "S Set node is not in univerzum.\n");
                    return false;
                }
            }
            // If character is EOF or newline we can end parsing
            if (c == EOF || c == '\n') {
                break;
            } else {
                continue;
            }
        }

        node[index] = c;
        index++;
    }
    print_set(s, u);
    return true;
}

/**
 * Parse relation from file stream
 * @param fp File pointer
 * @param r Relation
 * @param u Univerzum
 * @return True if everything went well
 * */
bool parse_relation(FILE* fp, struct relation* r, struct univerzum* u) {
    // Allocate memory for one node
    r->nodes = malloc(sizeof(struct relation_node));
    r->size = 0;

    char node[STRING_BUFFER_SIZE] = {0};
    int index = 0;

    while (true) {
        int c = getc(fp);
        if (c == '(') {
            while (c != ')') {
                c = getc(fp);

                if (c == ' ' || c == ')') {
                    node[index] = '\0';
                    index = 0;

                    // Compares relation node to univerzum node
                    int max = u->size;
                    for (int i = 0; i < max; i++) {
                        if (!(strcmp(node, u->nodes[i]))) {
                            if (c != ')') {
                                r->nodes[r->size].a = i;
                            } else {
                                r->nodes[r->size].b = i;
                            }
                            break;
                        }
                        // If the iteration is the last one => relation node
                        // wasn't found in univerzum
                        if (i == max - 1) {
                            fprintf(stderr,
                                    "S Relation node is not in univerzum.\n");
                            return false;
                        }
                    }
                    continue;
                }
                node[index] = c;
                index++;
            }
            r->size++;
            // Allocate memory for next node
            r->nodes =
                realloc(r->nodes, sizeof(struct relation_node) * (r->size + 1));
        }
        // If character is EOF or newline we can end parsing
        if (c == EOF || c == '\n') {
            break;
        }
    }
    print_relation(r, u);
    return true;
}

// bool parse_command(FILE* fp, struct command* c) {

// }

/**
 * Process univerzum
 * @param fp File pointer
 * @param store Store
 * @param store_size Store size
 * @return True if everything went well
 */
bool process_univerzum(FILE* fp, struct store_node* store, int* store_size) {
    int index = *store_size;

    // Init univerzum object
    store[index].type = UNIVERZUM;
    // TODO check malloc
    store[index].obj = malloc(sizeof(struct univerzum));
    (*store_size)++;

    // Parse univerzum
    if (!parse_univerzum(fp, store[index].obj)) {
        return false;
    }

    // Check if univerzum is valid
    return univerzum_valid(store[index].obj);
}

/**
 * Process set
 * @param fp File pointer
 * @param store Store
 * @param store_size Store size
 * @return True if everything went well
 */
bool process_set(FILE* fp,
                 struct store_node* store,
                 int* store_size,
                 struct univerzum* u) {
    int index = *store_size;

    // Init set object
    store[index].type = SET;
    // TODO check malloc
    store[index].obj = malloc(sizeof(struct set));
    (*store_size)++;

    // Handle parsing
    if (!parse_set(fp, store[index].obj, u)) {
        return false;
    }

    // Sort set
    set_sort(store[index].obj);

    // Check if set is valid
    return set_valid(store[index].obj);
}

/**
 * @param fp File pointer
 * @param store Store
 * @param store_size Store size
 * @return True if everything went well
 */
// bool process_command(FILE* fp, struct store_node* store, int* store_size) {
//     int index = *store_size;

//     // Init command object
//     store[index].type = COMMAND;
//     // TODO check malloc
//     store[index].obj = malloc(sizeof(struct command));
//     (*store_size)++;

//     // Parse command
//     parse_command(fp, store[index].obj);

//     return true;
// }

/**
 * @param fp File pointer
 * @param store Store
 * @param store_size Store size
 * @return True if everything went well
 */
bool process_relation(FILE* fp,
                      struct store_node* store,
                      int* store_size,
                      struct univerzum* u) {
    int index = *store_size;

    // Init relation object
    store[index].type = RELATION;
    // TODO check malloc
    store[index].obj = malloc(sizeof(struct relation));
    (*store_size)++;

    // Parse relation
    if (!parse_relation(fp, store[index].obj, u)) {
        return false;
    }

    // Sort relation
    relation_sort(store[index].obj);

    // Check if relation is valid
    return relation_valid(store[index].obj);
}

/**
 * Process all lines in file
 * @param fp File pointer
 * @return True if everything went well
 */
bool process_file(FILE* fp) {
    bool ok = false;

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
                ok = process_univerzum(fp, store, &store_size);
                break;
            case 'S':
                // TODO check if univerzum is at 0
                ok = process_set(fp, store, &store_size, store[0].obj);
                break;
            case 'R':
                // TODO check if univerzum is at 0
                ok = process_relation(fp, store, &store_size, store[0].obj);
                break;
            case 'C':
                // ok = process_command(fp, store, &store_size);
                break;
            default:
                // TODO handle other characters
                break;
        }

        if (!ok) {
            break;
        }
    }

    // Free store from memory
    free_store(store, store_size);

    return ok;
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
        fprintf(stderr, "Error parsing file!\n");
        return EXIT_FAILURE;
    }

    // Close file
    if (!close_file(fp)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
