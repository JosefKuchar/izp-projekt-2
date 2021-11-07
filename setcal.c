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
#include <time.h>     // For seeding random generator

/*-------------------------------- CONSTANTS --------------------------------*/

// Define string length hard limits
#define MAX_STRING_LENGTH 30
#define STRING_BUFFER_SIZE MAX_STRING_LENGTH + 1  // +1 is for \0

// Define maximum command arguments
#define MAX_COMMAND_ARGUMENTS 3

/*---------------------------------- ENUMS ----------------------------------*/

enum store_node_type { UNIVERSE, SET, RELATION, COMMAND };

enum function_input { IN_SET, IN_SET_SET, IN_SET_UNIVERZUM, IN_RELATION };

enum function_output { OUT_VOID, OUT_BOOL, OUT_SET, OUT_RELATION };

/*--------------------------------- STRUCTS ---------------------------------*/

// Struct to keep track of universe
struct universe {
    int size;                           // Universe size
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
    int type;                         // Command type
    int args[MAX_COMMAND_ARGUMENTS];  // Command arguments
    int argc;                         // Argument count
};

// Struct to keep track of one command definition
struct command_def {
    char name[STRING_BUFFER_SIZE];  // Command string
    void* function;                 // Command implementation
    enum function_input input;      // Function input
    enum function_output output;    // Function output
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
 * Check if universe is valid (doesn't contain reserved words, same words)
 * @param u Universe
 * @return True if universe is valid
 */
bool universe_valid(struct universe* u) {
    // Define all illegal words inside universe
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

    // Loop around all elements inside universe
    for (int i = 0; i < u->size; i++) {
        // Loop around all illegal words
        for (int j = 0; j < size; j++) {
            if (strcmp(u->nodes[i], illegal[j]) == 0) {
                fprintf(stderr, "Illegal word inside universe!\n");
                return false;
            }
        }
        // Check for repeated word
        for (int j = i + 1; j < u->size; j++) {
            if (strcmp(u->nodes[i], u->nodes[j]) == 0) {
                fprintf(stderr, "Repeated word inside universe!\n");
                return false;
            }
        }
    }
    // Universe is valid if everything went well
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
        // Universe can only be one and it has to be first element
        if (type == UNIVERSE) {
            if (i == 0) {
                u_found = true;
            } else {
                return false;
            }
            // Sets or relations can be only after universe or each other
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
 * Print universe
 * @param u Universe
 */
void print_universe(struct universe* u) {
    // Indicate we are printing universe
    printf("U");
    // Loop around all nodes inside universe
    for (int i = 0; i < u->size; i++) {
        // Print each node inside universe
        printf(" %s", u->nodes[i]);
    }
    printf("\n");
}

/**
 * Print set
 * @param a Set
 * @param u Universe
 */
void print_set(struct set* a, struct universe* u) {
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
 * @param u Universe
 */
void print_relation(struct relation* r, struct universe* u) {
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
            case UNIVERSE:
                print_universe(s->nodes[i].obj);
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

/*----------------------------- HELPER FUNCTIONS ----------------------------*/

/**
 * Realloc that automatically frees old block when fails
 * @param block Exisiting memory block
 * @param size New size
 * @return Pointer to newly allocated memory, NULL when fails
 */
void* srealloc(void* block, size_t size) {
    void* new = realloc(block, size);
    // Check if relloc failed
    if (new == NULL) {
        // Free old block
        free(block);
        return NULL;
    }
    return new;
}

/**
 * Find minimum of two numbers (integers)
 * @param a First number
 * @param b Second number
 * @return Minimum of two numbers
 */
int get_min(int a, int b) {
    return a > b ? b : a;
}

/**
 * Find maximum of two numbers (integers)
 * @param a First number
 * @param b Second number
 * @return Maximum of two numbers
 */
int get_max(int a, int b) {
    return a > b ? a : b;
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
 * @param u Universe
 * @return Pointer to new set
 */
struct set* set_complement(struct set* a, struct universe* u) {
    // Allocate memory complement set
    struct set* complement = malloc(sizeof(struct set));
    if (complement == NULL) {
        return NULL;
    }
    complement->size = 0;
    int size = (u->size - a->size) * sizeof(int);
    if (size == 0) {
        return complement;
    }
    complement->nodes = malloc(size);
    if (complement->nodes == NULL) {
        free(complement);
        return NULL;
    }

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
    if (s_union == NULL) {
        return NULL;
    }
    s_union->size = 0;
    int size = (a->size + b->size) * sizeof(int);
    if (size == 0) {
        return s_union;
    }
    s_union->nodes = malloc(size);
    if (s_union->nodes == NULL) {
        free(s_union);
        return NULL;
    }

    int i = 0, k = 0;
    while (i < a->size || k < b->size) {
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
    // Allocate memory intersect set
    struct set* intersect = malloc(sizeof(struct set));
    if (intersect == NULL) {
        return NULL;
    }
    intersect->size = 0;
    int size = get_min(a->size, b->size) * sizeof(int);
    if (size == 0) {
        return intersect;
    }
    intersect->nodes = malloc(size);
    if (intersect->nodes == NULL) {
        free(intersect);
        return NULL;
    }

    int i = 0, k = 0;
    while (i < a->size && k < b->size) {
        if (a->nodes[i] == b->nodes[k]) {
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
    if (minus == NULL) {
        return NULL;
    }
    minus->size = 0;
    int size = a->size * sizeof(int);
    if (size == 0) {
        return minus;
    }
    minus->nodes = malloc(size);
    if (minus->nodes == NULL) {
        free(minus);
        return NULL;
    }

    int i = 0, k = 0;
    while (i < a->size) {
        if (k > b->size || a->nodes[i] < b->nodes[k]) {
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
 * @param r Relation - sorted
 * @param u Universe - soted
 * @return True if relation is reflexive
 */
bool relation_reflexive(struct relation* r, struct universe* u) {
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
                // Relation is sorted (ascending), we won't find a match again
                // if the first relation node element is larger than current
                // universe node
            } else if (r->nodes[j].a > i) {
                break;
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
            if (r->nodes[i].a == r->nodes[k].b &&
                r->nodes[i].b == r->nodes[k].a) {
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
    for (int i = 0; i < r->size; i++) {
        for (int k = i + 1; k < r->size; k++) {
            if (r->nodes[i].a == r->nodes[k].b &&
                r->nodes[i].b == r->nodes[k].a) {
                return false;
            }
        }
    }
    return true;
}

bool relation_transitive(struct relation* r) {
    for (int i = 0; i < r->size; i++) {
        for (int j = 0; j < r->size; j++) {
            if (r->nodes[i].b == r->nodes[j].a) {
                for (int k = 0; k < r->size; k++) {
                    if (r->nodes[i].a == r->nodes[k].a &&
                        r->nodes[j].b == r->nodes[k].b) {
                        break;
                    }
                    if (k + 1 == r->size) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool relation_function(struct relation* r) {
    for (int i = 0; i < r->size - 1; i++) {
        if (r->nodes[i].a == r->nodes[i + 1].a) {
            return false;
        }
    }
    return true;
}

struct set* relation_domain(struct relation* r) {
    struct set* domain = malloc(sizeof(struct set));
    if (domain == NULL) {
        return NULL;
    }
    domain->nodes = malloc(sizeof(int));
    if (domain->nodes == NULL) {
        return NULL;
    }
    domain->size = 0;
    domain->nodes[domain->size++] = r->nodes[0].a;
    for (int i = 1; i < r->size; i++) {
        if (r->nodes[i].a != r->nodes[i - 1].a) {
            domain->nodes =
                srealloc(domain->nodes, sizeof(int) * (domain->size + 1));
            if (domain->nodes == NULL) {
                return NULL;
            }
            domain->nodes[domain->size++] = r->nodes[i].a;
        }
    }
    return domain;
}

struct set* relation_codomain(struct relation* r) {
    // TODO too ugly -> make better
    struct set* codomain = malloc(sizeof(struct set));
    if (codomain == NULL) {
        return NULL;
    }
    codomain->nodes = malloc(sizeof(int));
    if (codomain->nodes == NULL) {
        return NULL;
    }
    codomain->size = 0;

    int last = -1;
    int max = -1;
    for (int i = 0; i < r->size; i++) {
        if (r->nodes[i].b > max) {
            max = r->nodes[i].b;
        }
    }

    for (int i = 0; i < r->size; i++) {
        int min = max;
        for (int k = 0; k < r->size; k++) {
            if (r->nodes[k].b < min && r->nodes[k].b > last) {
                min = r->nodes[k].b;
            }
        }
        last = min;
        codomain->nodes =
            srealloc(codomain->nodes, sizeof(int) * (codomain->size + 1));
        if (codomain->nodes == NULL) {
            return NULL;
        }
        codomain->nodes[codomain->size++] = min;
        if (min == max) {
            break;
        }
    }
    return codomain;
}

bool relation_injective(struct relation* r) {
    for (int i = 0; i < r->size; i++) {
        for (int j = i + 1; j < r->size; j++) {
            if ((r->nodes[i].a == r->nodes[j].a) ||
                (r->nodes[i].b == r->nodes[j].b)) {
                return false;
            }
        }
    }
    return true;
}

bool relation_surjective(struct relation* r) {
    for (int i = 1; i < r->size; i++) {
        if (r->nodes[i].a == r->nodes[i - 1].a) {
            return false;
        }
    }
    return true;
}

bool relation_bijective(struct relation* r) {
    return relation_injective(r) && relation_surjective(r);
}

/**
 * Create reflexive relation closure
 * @param r Relation - sorted
 * @param u Universe - soted
 * @return Pointer to sorted relation closure
 * or NULL if didn't execute correctly
 */
struct relation* relation_closure_ref(struct relation* r, struct universe* u) {
    // Allocate memory for result relation, which is a copy of original
    struct relation* result = malloc(sizeof(struct relation));
    if (result == NULL) {
        return NULL;
    }

    result->nodes = malloc(sizeof(struct relation_node) * (r->size));
    if (result->nodes == NULL) {
        return NULL;
    }
    result->size = r->size;

    // Copy original relation to result
    for (int i = 0; i < result->size; i++) {
        result->nodes[i] = r->nodes[i];
    }

    // Look for reflexive nodes for each universe element
    // if we don't find any, then add it to result
    bool reflex_for_i;
    for (int i = 0; i < u->size; i++) {
        reflex_for_i = false;

        for (int j = 0; j < r->size; j++) {
            if (r->nodes[j].a == i) {
                if (r->nodes[j].a == r->nodes[j].b) {
                    reflex_for_i = true;
                    break;
                }
            } else if (r->nodes[j].a > i) {
                break;
            }
        }
        // Add reflexive element for current universe node
        if (!reflex_for_i) {
            result->size += 1;
            result->nodes = srealloc(
                result->nodes, sizeof(struct relation_node) * result->size);
            if (result->nodes == NULL) {
                return NULL;
            }
            result->nodes[result->size - 1].a = i;
            result->nodes[result->size - 1].b = i;
        }
    }
    relation_sort(result);

    return result;
}

/**
 * Create symetric relation closure
 * @param r Relation - sorted
 * @param u Universe - soted
 * @return Pointer to symetric relation closure
 * or NULL if didn't execute correctly
 */
struct relation* relation_closure_sym(struct relation* r) {
    // Create a copy of original relation where additional nodes can be added
    struct relation* result = malloc(sizeof(struct relation));
    if (result == NULL) {
        return NULL;
    }
    result->nodes = malloc(sizeof(struct relation_node) * r->size);
    if (result->nodes == NULL) {
        return NULL;
    }
    result->size = r->size;

    for (int i = 0; i < result->size; i++) {
        result->nodes[i] = r->nodes[i];
    }

    for (int i = 0; i < r->size; i++) {
        for (int k = 0; k < r->size; k++) {
            if (r->nodes[i].a == r->nodes[k].b &&
                r->nodes[i].b == r->nodes[k].a) {
                break;
            }
            // If relation is missing node to be symetric, add that node
            if (k + 1 == r->size) {
                result->size += 1;
                result->nodes = srealloc(
                    result->nodes, sizeof(struct relation_node) * result->size);
                if (result->nodes == NULL) {
                    return NULL;
                }
                result->nodes[result->size - 1].a = r->nodes[i].b;
                result->nodes[result->size - 1].b = r->nodes[i].a;
            }
        }
    }
    relation_sort(result);

    return result;
}

/**
 * Create transitive relation closure
 * @param r Relation - sorted
 * @param u Universe - soted
 * @return Pointer to transitive relation closure
 * or NULL if didn't execute correctly
 */
struct relation* relation_closure_trans(struct relation* r) {
    // Create a copy of the original relation
    struct relation* result = malloc(sizeof(struct relation));
    if (result == NULL) {
        return NULL;
    }
    result->nodes = malloc(sizeof(struct relation_node) * r->size);
    if (result->nodes == NULL) {
        return NULL;
    }
    result->size = r->size;

    for (int i = 0; i < result->size; i++) {
        result->nodes[i] = r->nodes[i];
    }

    // Compare second element of i with all first elements
    for (int i = 0; i < result->size; i++) {
        for (int j = 0; j < result->size; j++) {
            // If aRb and bRc, look if aRc
            if (result->nodes[i].b == result->nodes[j].a) {
                for (int k = 0; k < result->size; k++) {
                    // If aRc, go to the next i
                    if (result->nodes[i].a == result->nodes[k].a &&
                        result->nodes[j].b == result->nodes[k].b) {
                        break;
                    }
                    // If we don't find a node that says aRc,
                    // add that node as the last one
                    if (k + 1 == result->size) {
                        result->size += 1;
                        result->nodes = srealloc(
                            result->nodes,
                            sizeof(struct relation_node) * result->size);
                        if (result->nodes == NULL) {
                            return NULL;
                        }
                        result->nodes[result->size - 1].a = result->nodes[i].a;
                        result->nodes[result->size - 1].b = result->nodes[j].b;

                        // Start highest for loop over (i = 0 after i++)
                        i = -1;
                        break;
                    }
                }
                break;
            }
        }
    }
    relation_sort(result);

    return result;
}

/*---------------------------- SPECIAL COMMANDS -----------------------------*/

/**
 * Select random item from set
 * @param s Set
 * @param u Universe
 * @return True if set is not empty
 */
bool select_random_from_set(struct set* s, struct universe* u) {
    // Check if set is empty
    if (set_empty(s)) {
        return false;
    }
    // Get random index to set
    int rand_index = rand() % s->size;
    // Find index to universe
    int universe_index = s->nodes[rand_index];
    // Print element from universe
    printf("%s\n", u->nodes[universe_index]);

    return true;
}

/**
 * Select random item from relation
 * @param r Relation
 * @param u Universe
 * @return True if relation is not empty
 */
bool select_random_from_relation(struct relation* r, struct universe* u) {
    // Check if relation is empty
    if (r->size == 0) {
        return false;
    }
    // Get random index to relation
    int rand_index = rand() % r->size;
    // Find a, b in relation
    struct relation_node node = r->nodes[rand_index];
    // Randomly select index to universe from a, b
    int universe_index = rand() % 2 == 0 ? node.a : node.b;
    // Print element from universe
    printf("%s\n", u->nodes[universe_index]);

    return true;
}

/**
 * Select command
 * @param node Node from universe (set or relation)
 * @param u Universe
 * @return True if node is not empty
 */
bool select_command(struct store_node* node, struct universe* u) {
    switch (node->type) {
        case SET:
            return select_random_from_set(node->obj, u);
        case RELATION:
            return select_random_from_relation(node->obj, u);
        default:
            fprintf(stderr, "Invalid node type!\n");
            return false;
    }
}

/*------------------------ MEMORY FREEING FUNCTIONS -------------------------*/

/**
 * Free universe struct
 * @param u Universe
 */
void free_universe(struct universe* u) {
    if (u != NULL) {
        free(u->nodes);
        free(u);
    }
}

/**
 * Free set struct
 * @param s Set
 */
void free_set(struct set* s) {
    if (s != NULL) {
        free(s->nodes);
        free(s);
    }
}

/**
 * Free relation struct
 * @param r Relation
 */
void free_relation(struct relation* r) {
    if (r != NULL) {
        free(r->nodes);
        free(r);
    }
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
            case UNIVERSE:
                free_universe(store->nodes[i].obj);
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

// TODO maybe move this into some function
const struct command_def COMMAND_DEFS[] = {{.name = "empty",
                                            .function = set_empty,
                                            .input = IN_SET,
                                            .output = OUT_BOOL},
                                           {.name = "card",
                                            .function = set_card,
                                            .input = IN_SET,
                                            .output = OUT_VOID},
                                           {.name = "complement",
                                            .function = set_complement,
                                            .input = IN_SET,
                                            .output = OUT_SET},
                                           {.name = "union",
                                            .function = set_union,
                                            .input = IN_SET_SET,
                                            .output = OUT_SET},
                                           {.name = "intersect",
                                            .function = set_intersect,
                                            .input = IN_SET_SET,
                                            .output = OUT_SET},
                                           {.name = "minus",
                                            .function = set_minus,
                                            .input = IN_SET_SET,
                                            .output = OUT_SET},
                                           {.name = "subseteq",
                                            .function = set_subseteq,
                                            .input = IN_SET_SET,
                                            .output = OUT_BOOL},
                                           {.name = "subset",
                                            .function = set_subset,
                                            .input = IN_SET_SET,
                                            .output = OUT_BOOL},
                                           {.name = "equals",
                                            .function = set_equals,
                                            .input = IN_SET_SET,
                                            .output = OUT_BOOL},
                                           {.name = "codomain",
                                            .function = relation_codomain,
                                            .input = IN_RELATION,
                                            .output = OUT_SET},
                                           {.name = "function",
                                            .function = relation_function,
                                            .input = IN_RELATION,
                                            .output = OUT_BOOL}};

/*------------------------------- STORE RUNNER ------------------------------*/

/**
 * Function for running commands
 * @param command Command
 * @param store Store
 * @param i Program counter
 */
bool run_command(struct command* command, struct store* store, int* i) {
    struct command_def def = COMMAND_DEFS[command->type];

    void* result;

    switch (def.input) {
        case IN_SET:;
            void* (*function)(struct set*) = def.function;
            result = function(store->nodes[command->args[0] - 1].obj);
            break;
        case IN_SET_SET:;
            void* (*func)(struct set*, struct set*) = def.function;
            // TODO Check number of arguments
            // TODO Check object types
            result = func(store->nodes[command->args[0] - 1].obj,
                          store->nodes[command->args[1] - 1].obj);
            break;
        default:
            printf("This input is not implemented yet!\n");
            return true;  // True so it doesn't end
    }

    switch (def.output) {
        case OUT_SET:;
            if (result == NULL) {
                return false;
            }
            struct set* set = result;
            print_set(set, store->nodes[0].obj);
            // Replace command with actual set in store
            free_command(store->nodes[*i].obj);
            store->nodes[*i].type = SET;
            store->nodes[*i].obj = result;
            break;
        case OUT_RELATION:;
            if (result == NULL) {
                return false;
            }
            struct relation* relation = result;
            print_relation(relation, store->nodes[0].obj);
            // Replace command with actual relation in store
            free_command(store->nodes[*i].obj);
            store->nodes[*i].type = RELATION;
            store->nodes[*i].obj = result;
            break;
        case OUT_BOOL:;
            bool b = result;
            print_bool(b);
            if (!b) {
                // TODO jumping
            }
            break;
        case OUT_VOID:
            break;
    }

    return true;
}

/**
 * Function for running all things on store
 * @param store Store
 * @return True if everything went well
 */
bool store_runner(struct store* store) {
    for (int i = 0; i < store->size; i++) {
        switch (store->nodes[i].type) {
            case UNIVERSE:
                print_universe(store->nodes[i].obj);
                break;
            case SET:
                print_set(store->nodes[i].obj, store->nodes[0].obj);
                break;
            case RELATION:
                print_relation(store->nodes[i].obj, store->nodes[0].obj);
                break;
            case COMMAND:
                // Command can modify program counter
                if (!run_command(store->nodes[i].obj, store, &i)) {
                    fprintf(stderr, "Error running command!\n");
                    return false;
                };
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
 * Parse universe from file stream
 * @param fp File pointer
 * @param u Universe
 * @return True if everything went well
 */
bool parse_universe(FILE* fp, struct universe* u) {
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
            u->nodes = srealloc(u->nodes, sizeof(*u->nodes) * u->size);
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
 * @param u Universe
 * @return True if everything went well
 * */
bool parse_set(FILE* fp, struct set* s, struct universe* u) {
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
            s->nodes = srealloc(s->nodes, sizeof(int) * s->size + 1);

            // Compares set node to universe node
            int max = u->size;
            for (int i = 0; i < max; i++) {
                if (!(strcmp(node, u->nodes[i]))) {
                    s->nodes[s->size - 1] = i;
                    break;
                }
                // If the iteration is the last one => set node wasn't found in
                // universe
                if (i == max - 1) {
                    fprintf(stderr, "S Set node is not in universe.\n");
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
 * @param u Universe
 * @return True if everything went well
 * */
bool parse_relation(FILE* fp, struct relation* r, struct universe* u) {
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

                    // Compares relation node to universe node
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
                        // wasn't found in universe
                        if (i == max - 1) {
                            fprintf(stderr,
                                    "S Relation node is not in universe.\n");
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
            r->nodes = srealloc(r->nodes,
                                sizeof(struct relation_node) * (r->size + 1));
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
bool parse_command(FILE* fp, struct command* command) {
    // TODO handle overflow
    char buffer[STRING_BUFFER_SIZE] = {0};
    int index = 0;
    int argument = 0;

    while (true) {
        int c = getc(fp);
        bool end = c == EOF || c == '\n';

        if (c == ' ' || end) {
            buffer[index] = '\0';
            index = 0;
            if (argument == 0) {
                // Find command string
                // TODO
                bool found = false;
                int command_count =
                    sizeof(COMMAND_DEFS) / sizeof(COMMAND_DEFS[0]);
                for (int i = 0; i < command_count; i++) {
                    if (strcmp(COMMAND_DEFS[i].name, buffer) == 0) {
                        found = true;
                        command->type = i;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            } else {
                if (argument > MAX_COMMAND_ARGUMENTS) {
                    return false;
                }
                int result = 0;
                if (!parse_line_number(buffer, &result)) {
                    return false;
                }
                command->args[argument - 1] = result;
                command->argc++;
            }
            if (end) {
                return true;
            }
            argument++;
            continue;
        }
        // TODO fix buffer overflow
        buffer[index] = c;
        index++;
    }
    return true;
}

/**
 * Process universe
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_universe(FILE* fp, struct store* store, bool empty) {
    int index = store->size;

    // Init universe object
    store->nodes[index].type = UNIVERSE;
    // TODO check malloc
    store->nodes[index].obj = calloc(1, sizeof(struct universe));
    store->size++;

    // If set is empty, we can return it, it is completely valid
    if (empty) {
        return true;
    }

    // Parse universe
    if (!parse_universe(fp, store->nodes[index].obj)) {
        fprintf(stderr, "Error parsing universe!\n");
        return false;
    }

    // Check if universe is valid
    return universe_valid(store->nodes[index].obj);
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
    // Ensure that universe will be first
    if ((next != ' ' && !empty) || (store->size == 0 && c != 'U')) {
        return false;
    }

    switch (c) {
        case 'U':
            return process_universe(fp, store, empty);
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

    // Seed random generator
    srand(time(NULL));

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
