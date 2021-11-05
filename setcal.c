/**
 * @name IZP Projekt 2 - Prace s datovymi strukturami
 * @author Josef Kuchar - xkucha28
 * @author Martin Hemza - xhemza05
 * @author Filip Hauzvic - xhauzv00
 * 2021
 */

/*-------------------------------- LIBRARIES --------------------------------*/

#include <ctype.h>    // Char functions
#include <limits.h>   // Number limits
#include <stdbool.h>  // Bool type
#include <stdio.h>    // IO functions
#include <stdlib.h>   // EXIT macros
#include <string.h>   // String manipulation functions

/*-------------------------------- CONSTANTS --------------------------------*/

// Define string length hard limits
#define MAX_STRING_LENGTH 30
#define STRING_BUFFER_SIZE MAX_STRING_LENGTH + 1  // +1 is for \0

/*---------------------------------- ENUMS ----------------------------------*/

enum store_node_type { UNIVERZUM, SET, RELATION, COMMAND };

enum command_type { EMPTY };

/*--------------------------------- STRUCTS ---------------------------------*/

// Struct to keep track of univerzum
struct univerzum {
    int size;                           // Univerzum size
    char (*nodes)[STRING_BUFFER_SIZE];  // Array of strings
};

// Struct to keep track of one set
struct set {
    int size;    // Set size
    int* nodes;  // Set nodes
};

// Struct to keep track of on node inside relation
struct relation_node {
    int a;  // First number
    int b;  // Second number
};

// Struct to keep track of one relation
struct relation {
    int size;                     // Relation size
    struct relation_node* nodes;  // Relation nodes
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
    enum store_node_type type;  // Store node type
    void* obj;                  // Pointer to node
};

// Struct to keep track of every node inside store
struct store {
    int size;                  // Store size
    struct store_node* nodes;  // Store nodes
};

/*--------------------------------- SORTING ---------------------------------*/

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
int compare_rel_nodes(const void* a, const void* b) {
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
    qsort(r->nodes, r->size, sizeof(struct relation_node), compare_rel_nodes);
}

/*------------------------------- VALIDATIONS -------------------------------*/

/**
 * Check if univerzum is valid (doesn't contain reserved words, same words)
 * @param u Univerzum
 * @return True if univerzum is valid
 */
bool univerzum_valid(struct univerzum* u) {
    // Define all illegal words inside univerzum
    const char* illegal[] = {"empty",       "card",
                             "complement",  "union",
                             "intersect",   "minus",
                             "subseteq",    "subset",
                             "equals",      "reflexive",
                             "symmetric",   "antisymmetric",
                             "transitive",  "function",
                             "domain",      "codomain",
                             "injective",   "surjective",
                             "bijective",   "closure_ref",
                             "closure_sym", "closure_trans",
                             "select",      "true",
                             "false",       ""};
    int size = sizeof(illegal) / sizeof(illegal[0]);

    // Loop around all elements inside univerzum
    for (int i = 0; i < u->size; i++) {
        // Loop around all illegal words
        for (int j = 0; j < size; j++) {
            if (strcmp(u->nodes[i], illegal[j]) == 0) {
                fprintf(stderr, "Illegal word inside univerzum!\n");
                return false;
            }
        }
        // Check for repeated word
        for (int j = i + 1; j < u->size; j++) {
            if (strcmp(u->nodes[i], u->nodes[j]) == 0) {
                fprintf(stderr, "Repeated word inside univerzum!\n");
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
            fprintf(stderr, "Repeated item inside set!\n");
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
            fprintf(stderr, "Repeated item inside relation");
            return false;
        }
    }
    // If we didn't find two same relation nodes then this set is valid
    return true;
}

/**
 * Check if store is valid (correct order of node types)
 * @param store Store
 * @return True if store is valid
 */
bool store_valid(struct store* store) {
    // Ensure good order of node types
    bool u_found = false, s_or_r_found = false, c_found = false;
    for (int i = 0; i < store->size; i++) {
        enum store_node_type type = store->nodes[i].type;
        // Univerzum can only be one and it has to be first element
        if (type == UNIVERZUM) {
            if (i == 0) {
                u_found = true;
            } else {
                return false;
            }
            // Sets or relations can be only after univerzum or each other
        } else if (type == SET || type == RELATION) {
            if (c_found || !u_found) {
                return false;
            }
            s_or_r_found = true;
            // Commands can only be after set or relation or command
        } else if (type == COMMAND) {
            if (!s_or_r_found) {
                return false;
            }
            c_found = true;
        }
    }
    // Ensure all types of nodes are present
    return u_found && s_or_r_found && c_found;
}

/*----------------------------- PRINT FUNCTIONS -----------------------------*/

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

/**
 * Print sotre
 * @param s Store
 */
void print_store(struct store* s) {
    for (int i = 0; i < s->size; i++) {
        switch (s->nodes[i].type) {
            case UNIVERZUM:
                print_univerzum(s->nodes[i].obj);
                break;
            case SET:
                print_set(s->nodes[i].obj, s->nodes[0].obj);
                break;
            case RELATION:
                print_relation(s->nodes[i].obj, s->nodes[0].obj);
                break;
            case COMMAND:
                // TODO
                printf("COMMAND\n");
                break;
        }
    }
}

/*------------------------------ SET FUNCTIONS ------------------------------*/

/**
 * Set empty function
 * @param a Set
 * @return True if set is empty
 */
bool set_empty(struct set* a) {
    return a->size == 0;
}

/**
 * Prints size of set
 * @param a Set
 */
void set_card(struct set* a) {
    printf("%d\n", a->size);
}

/**
 * Set complement function
 * @param a Set - sorted
 * @param u Univerzum
 * @return Pointer to new set
 */
struct set* set_complement(struct set* a, struct univerzum* u) {
    // Allocate memory complement set
    struct set* complement = malloc(sizeof(struct set));
    complement->nodes = malloc((u->size * sizeof(int) - sizeof(a->nodes)));
    complement->size = 0;

    int i = 0, k = 0;
    while (i < u->size) {
        if (i == a->nodes[k]) {
            k++;
            i++;
            continue;
        }
        complement->nodes[complement->size++] = i;
        i++;
    }
    return complement;
}

/**
 * Set union function
 * @param a First set - sorted
 * @param b Second set - sorted
 * @return Pointer to new set
 */
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

/**
 * Set intersect function
 * @param a Set - sorted
 * @param b Set - sorted
 * @return Pointer to new set
 */
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

/**
 * Set minus function
 * @param a Set - sorted
 * @param b Set - sorted
 * @return Pointer to new set
 */
struct set* set_minus(struct set* a, struct set* b) {
    // Allocate memory minus set
    struct set* minus = malloc(sizeof(struct set));
    minus->nodes = malloc(sizeof(int));
    minus->size = 0;

    int i = 0, k = 0;
    while (i < a->size) {
        if (k > b->size || a->nodes[i] < b->nodes[k]) {
            minus->nodes =
                realloc(minus->nodes, sizeof(int) * (minus->size + 1));
            minus->nodes[minus->size++] = a->nodes[i++];
        } else if (a->nodes[i] == b->nodes[k]) {
            i++;
            k++;
        } else {
            k++;
        }
    }
    return minus;
}

/**
 * Set subseteq function
 * @param a Set - sorted
 * @param b Set - sorted
 * @return True if a is subset of b or equal to b
 */
bool set_subseteq(struct set* a, struct set* b) {
    int k = 0;
    for (int i = 0; i < b->size; i++) {
        if (k == a->size) {
            break;
        }
        if (b->nodes[i] == a->nodes[k]) {
            k++;
        }
    }
    return (k == a->size);
}

/**
 * Set subset function
 * @param a Set - sorted
 * @param b Set - sorted
 * @return True if a is subset of b
 */
bool set_subset(struct set* a, struct set* b) {
    int k = 0;
    for (int i = 0; i < b->size; i++) {
        if (k == a->size) {
            break;
        }
        if (b->nodes[i] == a->nodes[k]) {
            k++;
        }
    }
    return (k == a->size) && (a->size != b->size);
}

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

/*--------------------------- RELATION FUNCTIONS ----------------------------*/

/**
 * Find out if relation is reflexive
 * @param r Relation
 * @param u Univerzum
 * @return True if relation is reflexive
 */
bool relation_reflexive(struct relation* r, struct univerzum* u) {
    bool reflex_for_i;

    // Loop around all universe nodes
    for (int i = 0; i < u->size; i++) {
        reflex_for_i = false;

        // Loop around all relation nodes
        for (int j = 0; j < r->size; j++) {
            // Look for relation nodes that have same value as current universe
            // node
            if (r->nodes[j].a == i) {
                // If node is reflective, move on to next universe node
                if (r->nodes[j].a == r->nodes[j].b) {
                    reflex_for_i = true;
                    break;
                }
            }
        }

        // Relation isn't reflexive
        if (!reflex_for_i) {
            return false;
        }
    }

    return true;
}

bool relation_symmetric(struct relation* r) {
    for (int i = 0; i < r->size; i++) {
        for (int k = 0; k < r->size; k++) {
            if (r->nodes[i].a == r->nodes[k].b && r->nodes[i].b == r->nodes[k].a) {
                break;
            }
            if (k + 1 == r->size) {
                return false;
            }
        }
    }
    return true;
}

bool relation_antisymmetric(struct relation* r) {
    // TODO
    for (int i = 0; i < r->size; i++) {
        for (int k = i + 1; k < r->size; k++) {
            if (r->nodes[i].a == r->nodes[k].b && r->nodes[i].b == r->nodes[k].a) {
                return false;
            }
        }
    }
    return true;
}

/*
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

void relation_closure_ref() {
    // TODO
}

void relation_closure_sym() {
    // TODO
}

void relation_trans_sym() {
    //TODO
}


*/

/*------------------------ MEMORY FREEING FUNCTIONS -------------------------*/

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
 * Free relation struct
 * @param r Relation
 */
void free_relation(struct relation* r) {
    free(r->nodes);
    free(r);
}

/**
 * Free command struct
 * @param c Command
 */
void free_command(struct command* c) {
    free(c);
}

/**
 * Free store from memory including all children
 * @param store Pointer to store
 */
void free_store(struct store* store) {
    // Free all store nodes based on their type
    for (int i = 0; i < store->size; i++) {
        switch (store->nodes[i].type) {
            case UNIVERZUM:
                free_univerzum(store->nodes[i].obj);
                break;
            case SET:
                free_set(store->nodes[i].obj);
                break;
            case RELATION:
                free_relation(store->nodes[i].obj);
                break;
            case COMMAND:
                free_command(store->nodes[i].obj);
                break;
        }
    }

    // Free store itself
    free(store->nodes);
}

/*------------------------------- STORE RUNNER ------------------------------*/

/**
 * Function for running all things on store
 * @param store Store
 * @return True if everything went well
 */
bool store_runner(struct store* store) {
    for (int i = 0; i < store->size; i++) {
        switch (store->nodes[i].type) {
            case UNIVERZUM:
                print_univerzum(store->nodes[i].obj);
                break;
            case SET:
                print_set(store->nodes[i].obj, store->nodes[0].obj);
                break;
            case RELATION:
                print_relation(store->nodes[i].obj, store->nodes[0].obj);
                break;
            case COMMAND:
                printf("Not implemented!\n");
                break;
        }
    }

    return true;
}

/*------------------------------- FILE PARSING ------------------------------*/

/**
 * Parse line number
 * @param string String to be parsed
 * @param result Result
 * @return True if everything went well
 */
bool parse_line_number(char* string, int* result) {
    char* end_p;
    long number = strtol(string, &end_p, 10);

    // Check if string starts with number, also check if it ends with number
    if (end_p == string || *end_p != '\0') {
        fprintf(stderr, "Invalid characters inside number!\n");
        return false;
    }
    // Check if number is positive
    if (number <= 0) {
        fprintf(stderr, "Invalid line number!\n");
        return false;
    }
    // Clamp number to INT_MAX
    if (number > (long)INT_MAX) {
        number = INT_MAX;
    }
    // Return result by reference
    *result = number;

    return true;
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
            break;
            // If character is space create new node
        } else if (c == ' ') {
            u->size++;
            index = 0;
            u->nodes = realloc(u->nodes, sizeof(*u->nodes) * u->size);
            memset(u->nodes[u->size - 1], 0, STRING_BUFFER_SIZE);
            continue;
            // Handle invalid characters
        } else if (!isalpha(c)) {
            fprintf(stderr, "Invalid character in universum\n");
            return false;
        }
        // TODO handle overflow
        u->nodes[u->size - 1][index] = c;
        index++;
    }

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
    // TODO handle overflow
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
    // TODO handle overflow
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
    return true;
}

/**
 * Parse command
 * @param fp File pointer
 * @param c Command
 * @return True if everything went well
 */
bool parse_command(FILE* fp, struct command* c) {
    // TODO
    c->type = EMPTY;

    while (true) {
        int c = getc(fp);
        if (c == EOF || c == '\n') {
            break;
        }
    }

    return true;
}

/**
 * Process univerzum
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_univerzum(FILE* fp, struct store* store, bool empty) {
    int index = store->size;

    // Init univerzum object
    store->nodes[index].type = UNIVERZUM;
    // TODO check malloc
    store->nodes[index].obj = calloc(1, sizeof(struct univerzum));
    store->size++;

    // If set is empty, we can return it, it is completely valid
    if (empty) {
        return true;
    }

    // Parse univerzum
    if (!parse_univerzum(fp, store->nodes[index].obj)) {
        fprintf(stderr, "Error parsing univerzum!\n");
        return false;
    }

    // Check if univerzum is valid
    return univerzum_valid(store->nodes[index].obj);
}

/**
 * Process set
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_set(FILE* fp, struct store* store, bool empty) {
    int index = store->size;

    // Init set object
    store->nodes[index].type = SET;
    // TODO check malloc
    store->nodes[index].obj = calloc(1, sizeof(struct set));
    store->size++;

    // If set is empty, we can return it, it is completely valid
    if (empty) {
        return true;
    }

    // Handle parsing
    if (!parse_set(fp, store->nodes[index].obj, store->nodes[0].obj)) {
        fprintf(stderr, "Error parsing set!\n");
        return false;
    }

    // Sort set
    set_sort(store->nodes[index].obj);

    // Check if set is valid
    return set_valid(store->nodes[index].obj);
}

/**
 * Process relation
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_relation(FILE* fp, struct store* store, bool empty) {
    int index = store->size;

    // Init relation object
    store->nodes[index].type = RELATION;
    // TODO check malloc
    store->nodes[index].obj = calloc(1, sizeof(struct relation));
    store->size++;

    // If relation is empty, we can return it, it is completely valid
    if (empty) {
        return true;
    }

    // Parse relation
    if (!parse_relation(fp, store->nodes[index].obj, store->nodes[0].obj)) {
        fprintf(stderr, "Error parsing relation!\n");
        return false;
    }

    // Sort relation
    relation_sort(store->nodes[index].obj);

    // Check if relation is valid
    return relation_valid(store->nodes[index].obj);
}

/**
 * Process command
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_command(FILE* fp, struct store* store, bool empty) {
    // Command can't be empty
    if (empty) {
        fprintf(stderr, "Command can't be empty!");
        return false;
    }

    int index = store->size;

    // Init command object
    store->nodes[index].type = COMMAND;
    // TODO check malloc
    store->nodes[index].obj = malloc(sizeof(struct command));
    store->size++;

    // Parse command
    if (!parse_command(fp, store->nodes[index].obj)) {
        fprintf(stderr, "Error parsing command\n");
        return false;
    }

    return true;
}

/**
 * Process one line
 * @param fp File pointer
 * @param c Starting character
 * @param store Store
 * @return True if line was parsed correctly
 */
bool process_line(FILE* fp, char c, struct store* store) {
    int next = getc(fp);
    bool empty = next == '\n';

    // This shouldn't happen with valid file
    // Ensure that univerzum will be first
    if ((next != ' ' && !empty) || (store->size == 0 && c != 'U')) {
        return false;
    }

    switch (c) {
        case 'U':
            return process_univerzum(fp, store, empty);
        case 'S':
            return process_set(fp, store, empty);
        case 'R':
            return process_relation(fp, store, empty);
        case 'C':
            return process_command(fp, store, empty);
        default:
            fprintf(stderr, "Invalid starting character!\n");
            return false;
    }
}

/**
 * Process all lines in file
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_file(FILE* fp, struct store* store) {
    // TODO Realloc store
    // Loop around all lines
    for (int c = 0; (c = getc(fp)) != EOF;) {
        if (!process_line(fp, c, store)) {
            fprintf(stderr, "Error parsing file!\n");
            return false;
        }
    }
    // Check store validity
    if (!store_valid(store)) {
        fprintf(stderr, "Invalid definition of file parts!\n");
        return false;
    }
    // Run store
    if (!store_runner(store)) {
        fprintf(stderr, "Error running commands!\n");
        return false;
    }

    return true;
}

/*---------------------------- FILE MANIPULATION ----------------------------*/

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

/*------------------------ PROGRAM ARGUMENT FUNCTONS ------------------------*/

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

/*---------------------------------- ENTRY ----------------------------------*/

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
    struct store store;
    store.size = 0;
    store.nodes = malloc(sizeof(struct store_node) * 1000);
    if (store.nodes == NULL) {
        fprintf(stderr, "Malloc error!\n");
        return false;
    }
    if (!process_file(fp, &store)) {
        free_store(&store);
        close_file(fp);
        return EXIT_FAILURE;
    }
    free_store(&store);

    // Close file
    if (!close_file(fp)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
