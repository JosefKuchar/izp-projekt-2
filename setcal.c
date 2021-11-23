/**
 * @name IZP Projekt 2 - Prace s datovymi strukturami
 * @author Josef Kuchar - xkucha28
 * @author Martin Hemza - xhemza05
 * @author Filip Hauzvic - xhauzv00
 * 2021
 */

#pragma region LIBRARIES
/*-------------------------------- LIBRARIES --------------------------------*/

#include <ctype.h>    // Char functions
#include <limits.h>   // Number limits
#include <stdbool.h>  // Bool type
#include <stdio.h>    // IO functions
#include <stdlib.h>   // EXIT macros
#include <string.h>   // String manipulation functions
#include <time.h>     // For seeding random generator
#pragma endregion
#pragma region CONSTANTS
/*-------------------------------- CONSTANTS --------------------------------*/
// Define string length hard limits
#define MAX_STRING_LENGTH 30
#define STRING_BUFFER_SIZE MAX_STRING_LENGTH + 1  // +1 is for \0

// Define maximum command arguments
#define MAX_COMMAND_ARGUMENTS 3

// Define initial allocation sizes
#define INITIAL_STORE_ALLOC 10

#pragma endregion
#pragma region ENUMS
/*---------------------------------- ENUMS ----------------------------------*/

enum store_node_type { SET, RELATION, COMMAND };

enum function_input {
    IN_SET,
    IN_SET_SET,
    IN_SET_UNIVERSE,
    IN_RELATION,
    IN_RELATION_UNIVERSE
};

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
    int size;                   // Store size
    struct store_node* nodes;   // Store nodes
    struct universe* universe;  // Universe
};
#pragma endregion
#pragma region SORTING
/*--------------------------------- SORTING ---------------------------------*/

/**
 * @brief Function to compare two numbers - for qsort
 * @param a Pointer to first number
 * @param b Pointer to second number
 * @return Differece between two numbers
 */
int compare_num_nodes(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

/**
 * @brief Function to compare two relation nodes
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
 * @brief Sort set
 * @param s Set
 * @return Sorted set (ascending)
 */
void set_sort(struct set* s) {
    qsort(s->nodes, s->size, sizeof(int), compare_num_nodes);
}

/**
 * @brief Sort relation
 * @param r Relation
 * @return Sorted relation (ascending)
 */
void relation_sort(struct relation* r) {
    qsort(r->nodes, r->size, sizeof(struct relation_node), compare_rel_nodes);
}
#pragma endregion
#pragma region HELPER FUNCTIONS
/*----------------------------- HELPER FUNCTIONS ----------------------------*/

/**
 * @brief Realloc that automatically frees old block when fails
 * @param block Existing memory block
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
 * @brief Find minimum of two numbers (integers)
 * @param a First number
 * @param b Second number
 * @return Minimum of two numbers
 */
int get_min(int a, int b) {
    return a > b ? b : a;
}

/**
 * @brief Find maximum of two numbers (integers)
 * @param a First number
 * @param b Second number
 * @return Maximum of two numbers
 */
int get_max(int a, int b) {
    return a > b ? a : b;
}

/**
 * @brief Generate universe from set
 * @param universe Universe
 * @return Set if everything went correctly, null if malloc failed
 */
struct set* get_set_from_universe(struct universe* universe) {
    // Allocate new set
    struct set* set = malloc(sizeof(struct set));
    // Check if malloc failed
    if (set == NULL) {
        return NULL;
    }
    // Define set
    set->size = universe->size;
    set->nodes = malloc(sizeof(int) * set->size);
    // If set has 0 size we are done
    if (set->size == 0) {
        return set;
    }
    // Check if second malloc failed
    if (set->nodes == NULL) {
        free(set);
        return NULL;
    }
    // Generate set nodes
    for (int i = 0; i < universe->size; i++) {
        set->nodes[i] = i;
    }

    return set;
}

/**
 * @brief Get number of arguments from input type
 * @param input_type Input type
 * @return Number of arguments
 */
int get_argument_count(enum function_input input_type) {
    switch (input_type) {
        case IN_SET:
            return 1;
        case IN_SET_UNIVERSE:
            return 1;
        case IN_RELATION:
            return 1;
        case IN_RELATION_UNIVERSE:
            return 1;
        case IN_SET_SET:
            return 2;
    }
    return 1;
}

/**
 * @brief Error printing function
 * @param message Error message
 * @return Always false for simple usage
 */
bool error(const char* message) {
    fprintf(stderr, "%s", message);
    return false;
}

#pragma endregion
#pragma region VALIDATIONS
/*------------------------------- VALIDATIONS -------------------------------*/

/**
 * @brief Check if universe is valid (doesn't contain reserved words, same words)
 * @param u Universe
 * @retval true - Universe valid
 * @retval false - Universe invalid
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
                return error("Illegal word inside universe!\n");
            }
        }
        // Check for repeated word
        for (int j = i + 1; j < u->size; j++) {
            if (strcmp(u->nodes[i], u->nodes[j]) == 0) {
                return error("Repeated word inside universe!\n");
            }
        }
    }
    // Universe is valid if everything went well
    return true;
}

/**
 * @brief Check if set is valid
 * @param s Set - sorted
 * @retval true - Set valid
 * @retval false - Set invalid
 */
bool set_valid(struct set* a) {
    // Loop around all elements inside set
    for (int i = 1; i < a->size; i++) {
        // If last item is same as current then this set is invalid
        if (a->nodes[i] == a->nodes[i - 1]) {
            return error("Repeated item inside set!\n");
        }
    }
    // If we didn't find two same elements then this set is valid
    return true;
}

/**
 * @brief Check if relation is valid
 * @param r Relation - sorted
 * @retval true - Relation valid
 * @retval false - Relation invalid
 */
bool relation_valid(struct relation* r) {
    // Loop around all elements inside relation
    for (int i = 1; i < r->size; i++) {
        // If last item is same as current then this relation is invalid
        if (r->nodes[i].a == r->nodes[i - 1].a &&
            r->nodes[i].b == r->nodes[i - 1].b) {
            return error("Repeated item inside relation");
        }
    }
    // If we didn't find two same relation nodes then this set is valid
    return true;
}

/**
 * @brief Check if arguments of command are valid
 * @param command command
 * @param store store
 * @param def command definition
 * @retval true - Command arguments valid
 * @retval false - Command arguments invalid
 */
bool command_arguments_valid(struct command* command,
                             struct store* store,
                             struct command_def def) {
    // Argument points to non-existant line
    for (int i = 0; i < command->argc; i++) {
        if (store->size < command->args[i]) {
            return false;
        }
    }

    // Check argument count
    int argument_count = get_argument_count(def.input);
    if (def.output == OUT_BOOL) {
        if (command->argc != argument_count &&
            command->argc != argument_count + 1) {
            return false;
        }
    } else {
        if (command->argc != argument_count) {
            return false;
        }
    }
    // Check if argument types match input types
    switch (def.input) {
        case IN_SET:
            return store->nodes[command->args[0] - 1].type == SET;
        case IN_SET_UNIVERSE:
            return store->nodes[command->args[0] - 1].type == SET;
        case IN_SET_SET:
            return store->nodes[command->args[0] - 1].type == SET &&
                   store->nodes[command->args[1] - 1].type == SET;
        case IN_RELATION:
            return store->nodes[command->args[0] - 1].type == RELATION;
        case IN_RELATION_UNIVERSE:
            return store->nodes[command->args[0] - 1].type == RELATION;
    }

    return true;
}

/**
 * @brief Check if store is valid (correct order of node types)
 * @param store Store
 * @retval true - Store valid
 * @retval false - Store invalid
 */
bool store_valid(struct store* store) {
    // Ensure good order of node types
    bool s_or_r_found = false, c_found = false;
    for (int i = 1; i < store->size; i++) {
        enum store_node_type type = store->nodes[i].type;
        // Sets or relations can be only after universe or each other
        if (type == SET || type == RELATION) {
            if (c_found) {
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
    return s_or_r_found && c_found;
}
#pragma endregion
#pragma region PRINT FUNCTIONS
/*----------------------------- PRINT FUNCTIONS -----------------------------*/

/**
 * @brief Print bool value
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
 * @brief Print set
 * @param a Set
 * @param u Universe
 */
void print_set(struct set* a, struct universe* u, bool is_universe) {
    // Indicate we are printing set or universe
    printf(is_universe ? "U" : "S");
    // Loop around all nodes inside set
    for (int i = 0; i < a->size; i++) {
        // Print each node inside set
        printf(" %s", u->nodes[a->nodes[i]]);
    }
    printf("\n");
}

/**
 * @brief Print relation
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
#pragma endregion
#pragma region SET FUNCTIONS
/*------------------------------ SET FUNCTIONS ------------------------------*/

/**
 * @brief Check if set is empty
 * @param a Set
 * @retval true - Set is empty
 * @retval false - Set is not empty
 */
bool set_empty(struct set* a) {
    return a->size == 0;
}

/**
 * @brief Prints size of set
 * @param a Set
 */
void set_card(struct set* a) {
    printf("%d\n", a->size);
}

/**
 * @brief Create set complement
 * @param a Set - sorted
 * @param u Universe
 * @return Pointer to new set
 */
struct set* set_complement(struct set* a, struct universe* u) {
    // Memory allocation for set
    struct set* complement = malloc(sizeof(struct set));
    if (complement == NULL) {
        return NULL;
    }
    complement->size = 0;

    int size = (u->size - a->size) * sizeof(int);
    // If set is same as universe => complement is empty
    if (size == 0) {
        complement->nodes = NULL;
        return complement;
    }

    // Memory allocation for set nodes
    complement->nodes = malloc(size);
    if (complement->nodes == NULL) {
        free(complement);
        return NULL;
    }

    // Loop around all universe nodes
    for (int i = 0, k = 0; i < u->size; i++) {
        // If universe node is in given set => skip adding that node into
        // complement
        if (k < a->size && i == a->nodes[k]) {
            k++;
            continue;
        }
        // Adds universe node into complement
        complement->nodes[complement->size++] = i;
    }
    return complement;
}

/**
 * @brief Find union of two sets
 * @param a First set - sorted
 * @param b Second set - sorted
 * @return Pointer to new set
 */
struct set* set_union(struct set* a, struct set* b) {
    // Memory allocation for set
    struct set* s_union = malloc(sizeof(struct set));
    if (s_union == NULL) {
        return NULL;
    }
    s_union->size = 0;

    int size = (a->size + b->size) * sizeof(int);
    // If sets are empty => union is empty
    if (size == 0) {
        s_union->nodes = NULL;
        return s_union;
    }

    // Memory allocation for set nodes
    s_union->nodes = malloc(size);
    if (s_union->nodes == NULL) {
        free(s_union);
        return NULL;
    }

    // Starting indexes of sets A and B
    int i = 0, k = 0;
    // Loop around all nodes from set A
    while (i < a->size) {
        // Check if k index is smaller than size of set B and if node from set B
        // is smaller or equal to node from set A => add node from set B
        if (k < b->size && b->nodes[k] <= a->nodes[i]) {
            // If nodes are equal => increment index of set A
            if (b->nodes[k] == a->nodes[i]) {
                i++;
            }
            // Adds node from set B
            s_union->nodes[s_union->size++] = b->nodes[k++];
            continue;
        }
        // Adds node from set A
        s_union->nodes[s_union->size++] = a->nodes[i++];
    }
    // Adds remaining nodes from set B
    while (k < b->size) {
        s_union->nodes[s_union->size++] = b->nodes[k++];
    }

    return s_union;
}

/**
 * @brief Find intersect of two sets
 * @param a Set - sorted
 * @param b Set - sorted
 * @return Pointer to new set
 */
struct set* set_intersect(struct set* a, struct set* b) {
    // Memory allocation for set
    struct set* intersect = malloc(sizeof(struct set));
    if (intersect == NULL) {
        return NULL;
    }
    intersect->size = 0;

    int size = get_min(a->size, b->size) * sizeof(int);
    // If one of the sets is empty => intersect is empty
    if (size == 0) {
        intersect->nodes = NULL;
        return intersect;
    }

    // Memory allocation for set nodes
    intersect->nodes = malloc(size);
    if (intersect->nodes == NULL) {
        free(intersect);
        return NULL;
    }

    // Starting indexes of sets A and B
    int i = 0, k = 0;
    // Loop until one of the indexers is same as size of it's set
    while (i < a->size && k < b->size) {
        // If nodes are the same => add node to interset and increment indexers
        if (a->nodes[i] == b->nodes[k]) {
            intersect->nodes[intersect->size++] = a->nodes[i++];
            k++;
        }
        // If node from set A is smaller than node from set B => increment index
        // of set A, else increment index of set b
        else if (a->nodes[i] < b->nodes[k]) {
            i++;
        } else {
            k++;
        }
    }
    return intersect;
}

/**
 * @brief Finds difference between two sets
 * @param a Set - sorted
 * @param b Set - sorted
 * @return Pointer to new set
 */
struct set* set_minus(struct set* a, struct set* b) {
    // Memory allocation for set
    struct set* minus = malloc(sizeof(struct set));
    if (minus == NULL) {
        return NULL;
    }
    minus->size = 0;

    int size = a->size * sizeof(int);
    // If set A is empty => minus is empty
    if (size == 0) {
        minus->nodes = NULL;
        return minus;
    }

    // Memory allocation for set nodes
    minus->nodes = malloc(size);
    if (minus->nodes == NULL) {
        free(minus);
        return NULL;
    }

    // Starting indexes of sets A and B
    int i = 0, k = 0;
    // Loop around all nodes from set A
    while (i < a->size) {
        // Check if k index is smaller than size of set B and if node from set B
        // is smaller or equal to node from set A => skip adding node
        if (k < b->size && b->nodes[k] <= a->nodes[i]) {
            // If nodes are equal => increment index of set A
            if (b->nodes[k] == a->nodes[i]) {
                i++;
            }
            // increment index of set A
            k++;
            continue;
        }
        // Adds node from set A
        minus->nodes[minus->size++] = a->nodes[i++];
    }
    return minus;
}

/**
 * @brief Set subseteq function
 * @param a Set - sorted
 * @param b Set - sorted
 * @retval true - Set a is subset of b or equal to b
 * @retval false - Set a isn't subset of b or equal to b
 */
bool set_subseteq(struct set* a, struct set* b) {
    // Index of set A
    int k = 0;
    // Loop around all nodes from set B or until all nodes from set A were
    // found in set B
    for (int i = 0; i < b->size && k < a->size; i++) {
        //  Check if node from set A is in set B => increment index of set A
        if (b->nodes[i] == a->nodes[k]) {
            k++;
        }
    }
    // If the index of set A is the same as size of set A => is subseteq
    return (k == a->size);
}

/**
 * @brief Set subset function
 * @param a Set - sorted
 * @param b Set - sorted
 * @retval true - Set a is subset of b
 * @retval false - Set a isn't subset of b
 */
bool set_subset(struct set* a, struct set* b) {
    // Index of set A
    int k = 0;
    // Loop around all nodes from set B or until all nodes from set A were
    // found in set B
    for (int i = 0; i < b->size && k < a->size; i++) {
        //  Check if node from set A is in set B => increment index of set A
        if (b->nodes[i] == a->nodes[k]) {
            k++;
        }
    }
    // If the index of set A is the same as size of set A and size of set A and
    // B are different => is subset
    return (k == a->size) && (a->size != b->size);
}

/**
 * @brief Compare two sets
 * @param a First set - sorted
 * @param b Second set - sorted
 * @retval true - sets are equal
 * @retval false - sets aren't equal
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
#pragma endregion
#pragma region RELATION FUNCTIONS
/*--------------------------- RELATION FUNCTIONS ----------------------------*/

/**
 * @brief Find out if relation is reflexive
 * @param r Relation - sorted
 * @param u Universe - sorted
 * @retval true - Relation is reflexive
 * @retval false - Relation isn't reflexive
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

        if (!reflex_for_i) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Find out if relation is symmetric
 *
 * @param r Relation - sorted
 * @retval true - Relation is symmetric
 * @retval false - Relation is not symmetric
 */
bool relation_symmetric(struct relation* r) {
    // Loop around all relation nodes
    for (int i = 0; i < r->size; i++) {
        // Loop around all relation nodes
        for (int k = 0; k < r->size; k++) {
            // If symmetric node is found => break
            if (r->nodes[i].a == r->nodes[k].b &&
                r->nodes[i].b == r->nodes[k].a) {
                break;
            }
            // If the iteration in the last one => symmetric node wasn't found
            // and relation is not symmetric
            if (k + 1 == r->size) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Find out if relation is symmetric
 *
 * @param r Relation - sorted
 * @retval true - Relation is antisymmetric
 * @retval false - Relation is not antisymmetric
 */
bool relation_antisymmetric(struct relation* r) {
    // Loop around all relation nodes
    for (int i = 0; i < r->size; i++) {
        // Loop around all remaining nodes
        for (int k = i + 1; k < r->size; k++) {
            // If symmetric node is found => relation is not antisymmetric
            if (r->nodes[i].a == r->nodes[k].b &&
                r->nodes[i].b == r->nodes[k].a) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Find out if relation is transitive
 *
 * @param r Relation - sorted
 * @retval true - Relation is transitive
 * @retval false - Relation is not transitive
 */
bool relation_transitive(struct relation* r) {
    // Transitive relation: (aRb & bRa) => aRc

    // Loop around all relation nodes
    for (int i = 0; i < r->size; i++) {
        // Loop around all relation nodes
        for (int j = 0; j < r->size; j++) {
            // Looks for node that has second element same as current node (bRa)
            if (r->nodes[i].b == r->nodes[j].a) {
                // Loop around all relation nodes
                for (int k = 0; k < r->size; k++) {
                    // Looks for node that meets aRc
                    if (r->nodes[i].a == r->nodes[k].a &&
                        r->nodes[j].b == r->nodes[k].b) {
                        break;
                    }
                    // If the iteration is the last one => aRc wasn't found and
                    // relation is not transitive
                    if (k + 1 == r->size) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

/**
 * @brief Find out if relation is a function
 *
 * @param r Relation - sorted
 * @retval true - Relation is a function
 * @retval false - Relation is not a function
 */
bool relation_function(struct relation* r) {
    // Loops around all elemnts - 1
    for (int i = 0; i < r->size - 1; i++) {
        // If current node is same as next node => relation is not a function
        if (r->nodes[i].a == r->nodes[i + 1].a) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Relation domain function
 *
 * @param r Relation - sorted
 * @return Pointer to a new set
 */
struct set* relation_domain(struct relation* r) {
    // Memory allocation for set
    struct set* domain = malloc(sizeof(struct set));
    if (domain == NULL) {
        return NULL;
    }

    // Memory allocation for set nodes
    domain->nodes = malloc(sizeof(int));
    if (domain->nodes == NULL) {
        free(domain);
        return NULL;
    }

    domain->size = 0;

    // Checks if relation is empty => returns emtpy relation
    if (r->size <= 0) {
        domain->nodes = NULL;
        return domain;
    }

    // Puts the first element of the first relation node to set
    domain->nodes[domain->size++] = r->nodes[0].a;
    for (int i = 1; i < r->size; i++) {
        // If the last element is not the same as current element => adds
        // element in set
        if (r->nodes[i].a != r->nodes[i - 1].a) {
            // Memory reallocation for set nodes
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

/**
 * @brief Relation codomain function
 *
 * @param r Relation - sorted
 * @return Pointer to a new set
 */
struct set* relation_codomain(struct relation* r) {
    // Memory allocation for set
    struct set* codomain = malloc(sizeof(struct set));
    if (codomain == NULL) {
        return NULL;
    }

    // Memory allocation for set nodes
    codomain->nodes = malloc(sizeof(int));
    if (codomain->nodes == NULL) {
        return NULL;
    }

    codomain->size = 0;

    // Loop around all nodes
    for (int i = 0; i < r->size; i++) {
        bool found = false;
        // Find out if current element is already in the codomain set
        for (int k = 0; k < codomain->size; k++) {
            if (r->nodes[i].b == codomain->nodes[k]) {
                found = true;
                break;
            }
        }
        // If element wasn't found in codomain => adds element
        if (!found) {
            // Memory reallocation for set nodes
            codomain->nodes =
                srealloc(codomain->nodes, sizeof(int) * (codomain->size + 1));
            if (codomain->nodes == NULL) {
                return NULL;
            }

            codomain->nodes[codomain->size++] = r->nodes[i].b;
        }
    }
    set_sort(codomain);
    return codomain;
}

/**
 * @brief Find out if relation is injective
 *
 * @param r Relation sorted
 * @retval true - Relation is injective
 * @retval false - Relation is not injective
 */
bool relation_injective(struct relation* r) {
    // Loop around all nodes
    for (int i = 0; i < r->size; i++) {
        // Loop around all remaining nodes
        for (int j = i + 1; j < r->size; j++) {
            // If current node has first or second element same as one of the
            // remaining nodes => not injective
            if ((r->nodes[i].a == r->nodes[j].a) ||
                (r->nodes[i].b == r->nodes[j].b)) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Find out if relation is surjective
 *
 * @param r Relation - sorted
 * @return true - Relation is surjective
 * @return false - Relation is not surjective
 */
bool relation_surjective(struct relation* r) {
    // Loop around all nodes
    for (int i = 1; i < r->size; i++) {
        // If current node is same as next node => relation is not surjective
        if (r->nodes[i].a == r->nodes[i - 1].a) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Find out if relation is bijective
 *
 * @param r Relation - sorted
 * @retval true - Relation is bijective
 * @return false - Relation is not bijective
 */
bool relation_bijective(struct relation* r) {
    // Relation is bijective if relation is injective and surjective
    return relation_injective(r) && relation_surjective(r);
}

/**
 * @brief Create reflexive relation closure
 * @param r Relation - sorted
 * @param u Universe - sorted
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
 * @brief Create symmetric relation closure
 * @param r Relation - sorted
 * @param u Universe - sorted
 * @return Pointer to symmetric relation closure
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
            // If relation is missing node to be symmetric, add that node
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
 * @brief Create transitive relation closure
 * @param r Relation - sorted
 * @param u Universe - sorted
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

#pragma endregion
#pragma region SPECIAL COMMANDS
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
            return error("Invalid node type!\n");
    }
}
#pragma endregion
#pragma region MEMORY FREEING FUNCTIONS
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

    // Free universe
    free_universe(store->universe);

    // Free store itself
    free(store->nodes);
}

// TODO maybe move this into some function
const struct command_def COMMAND_DEFS[] = {
    // Function name, function pointer, input, output
    {"empty", set_empty, IN_SET, OUT_BOOL},
    {"card", set_card, IN_SET, OUT_VOID},
    {"complement", set_complement, IN_SET_UNIVERSE, OUT_SET},
    {"union", set_union, IN_SET_SET, OUT_SET},
    {"intersect", set_intersect, IN_SET_SET, OUT_SET},
    {"minus", set_minus, IN_SET_SET, OUT_SET},
    {"subseteq", set_subseteq, IN_SET_SET, OUT_BOOL},
    {"subset", set_subset, IN_SET_SET, OUT_BOOL},
    {"equals", set_equals, IN_SET_SET, OUT_BOOL},
    {"reflexive", relation_reflexive, IN_RELATION_UNIVERSE, OUT_BOOL},
    {"symmetric", relation_symmetric, IN_RELATION, OUT_BOOL},
    {"antisymmetric", relation_antisymmetric, IN_RELATION, OUT_BOOL},
    {"transitive", relation_transitive, IN_RELATION, OUT_BOOL},
    {"function", relation_function, IN_RELATION, OUT_BOOL},
    {"domain", relation_domain, IN_RELATION, OUT_SET},
    {"codomain", relation_codomain, IN_RELATION, OUT_SET},
    {"injective", relation_injective, IN_RELATION, OUT_BOOL},
    {"surjective", relation_surjective, IN_RELATION, OUT_BOOL},
    {"bijective", relation_bijective, IN_RELATION, OUT_BOOL},
    {"closure_ref", relation_closure_ref, IN_RELATION_UNIVERSE, OUT_RELATION},
    {"closure_sym", relation_closure_sym, IN_RELATION, OUT_RELATION},
    {"closure_trans", relation_closure_trans, IN_RELATION, OUT_RELATION}};
#pragma endregion
#pragma region STORE RUNNER
/*------------------------------- STORE RUNNER ------------------------------*/

/**
 * Function for procesing bool ouput
 * @param r Result - bool
 * @return True if no error occurred
 */
bool process_output_bool(bool r,
                         enum function_input input,
                         struct command* command,
                         int* i) {
    // Print the actual bool
    print_bool(r);

    // Handle jumping by modifying program counter
    if (!r) {
        int arg_count = get_argument_count(input);

        // Jump
        if (arg_count < command->argc) {
            // Use last argument as new program counter location
            *i = command->args[command->argc - 1] - 1;
        }
    }
    return true;
}

/**
 * Function for processing set output
 * @param s Store
 * @param r Result - relation
 * @param i Program counter
 * @return True if no error occurred
 */
bool process_output_relation(struct store* s, struct relation* r, int i) {
    // Check if function actually returned valid object
    if (r == NULL) {
        return false;
    }

    // Print the actual relation
    print_relation(r, s->universe);

    // Replace command with actual relation in store
    free_command(s->nodes[i].obj);
    s->nodes[i].type = RELATION;
    s->nodes[i].obj = r;

    return true;
}

/**
 * Function for processing set output
 * @param s Store
 * @param r Result - set
 * @param i Program counter
 * @return True if no error occurred
 */
bool process_output_set(struct store* s, struct set* r, int i) {
    // Check if function actually returned valid object
    if (r == NULL) {
        return false;
    }

    // Print the actual set
    print_set(r, s->universe, false);

    // Replace command with actual set in store
    free_command(s->nodes[i].obj);
    s->nodes[i].type = SET;
    s->nodes[i].obj = r;

    return true;
}

void* process_function_input(struct store* s,
                             struct command* c,
                             struct command_def def) {
    switch (def.input) {
        case IN_SET:;
            void* (*f_s)(struct set*) = def.function;
            return f_s(s->nodes[c->args[0] - 1].obj);
        case IN_SET_SET:;
            void* (*f_s_s)(struct set*, struct set*) = def.function;
            return f_s_s(s->nodes[c->args[0] - 1].obj,
                         s->nodes[c->args[1] - 1].obj);
        case IN_SET_UNIVERSE:;
            void* (*f_s_u)(struct set*, struct universe*) = def.function;
            return f_s_u(s->nodes[c->args[0] - 1].obj, s->universe);
        case IN_RELATION:;
            void* (*f_r)(struct relation*) = def.function;
            return f_r(s->nodes[c->args[0] - 1].obj);
        case IN_RELATION_UNIVERSE:;
            void* (*f_r_u)(struct relation*, struct universe*) = def.function;
            return f_r_u(s->nodes[c->args[0] - 1].obj, s->universe);
        default:
            return NULL;
    }
}

/**
 * Function for running commands
 * @param command Command
 * @param store Store
 * @param i Program counter
 */
bool run_command(struct command* command, struct store* store, int* i) {
    struct command_def def = COMMAND_DEFS[command->type];

    if (!command_arguments_valid(command, store, def)) {
        return error("Invalid command arguments!\n");
    }

    void* result = process_function_input(store, command, def);

    switch (def.output) {
        case OUT_SET:;
            return process_output_set(store, result, *i);
        case OUT_RELATION:;
            return process_output_relation(store, result, *i);
        case OUT_BOOL:;
            return process_output_bool(result, def.input, command, i);
        case OUT_VOID:
            return true;
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
            case SET:
                print_set(store->nodes[i].obj, store->universe, i == 0);
                break;
            case RELATION:
                print_relation(store->nodes[i].obj, store->universe);
                break;
            case COMMAND:
                // Command can modify program counter
                if (!run_command(store->nodes[i].obj, store, &i)) {
                    return error("Error running command!\n");
                };
                break;
        }
    }

    return true;
}
#pragma endregion
#pragma region FILE PARSING
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
        return error("Invalid characters inside number!\n");
    }
    // Check if number is positive
    if (number <= 0) {
        return error("Invalid line number!\n");
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
    u->nodes = calloc(sizeof(*u->nodes), 1);
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
            return error("Invalid character in universe\n");
        }

        if (index >= MAX_STRING_LENGTH) {
            return error("Element name too long!\n");
        }

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
                    return error("S Set node is not in universe.\n");
                }
            }
            // If character is EOF or newline we can end parsing
            if (c == EOF || c == '\n') {
                break;
            } else {
                continue;
            }
        }

        if (index >= MAX_STRING_LENGTH) {
            return error("Element name too long!\n");
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
                            return error(
                                "S Relation node is not in universe.\n");
                        }
                    }
                    continue;
                }

                if (index >= MAX_STRING_LENGTH) {
                    return error("Element name too long!\n");
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
    command->argc = 0;

    while (true) {
        int c = getc(fp);
        bool end = c == EOF || c == '\n';

        if (c == ' ' || end) {
            buffer[index] = '\0';
            index = 0;
            if (argument == 0) {
                // Find command string
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

    // TODO check malloc
    // TODO make this function better
    store->universe = malloc(sizeof(struct universe));

    // If universe is empty, we can return, it is completely valid
    if (empty) {
        store->universe->nodes = NULL;
        store->universe->size = 0;
        store->nodes[index].type = SET;
        store->nodes[index].obj = get_set_from_universe(store->universe);
        store->size++;
        return true;
    }

    // Parse universe
    if (!parse_universe(fp, store->universe)) {
        return error("Error parsing universe!\n");
    }

    // Check if universe is valid
    if (!universe_valid(store->universe)) {
        return error("Invalid universe!\n");
    }

    // Generate set from universe
    store->nodes[index].type = SET;
    store->nodes[index].obj = get_set_from_universe(store->universe);

    // Check malloc error
    if (store->nodes[index].obj == NULL) {
        return error("Malloc error!\n");
    }

    store->size++;

    return true;
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
    if (!parse_set(fp, store->nodes[index].obj, store->universe)) {
        return error("Error parsing set!\n");
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
    if (!parse_relation(fp, store->nodes[index].obj, store->universe)) {
        return error("Error parsing relation!\n");
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
        return error("Command can't be empty!");
    }

    int index = store->size;

    // Init command object
    store->nodes[index].type = COMMAND;
    // TODO check malloc
    store->nodes[index].obj = malloc(sizeof(struct command));
    store->size++;

    // Parse command
    if (!parse_command(fp, store->nodes[index].obj)) {
        return error("Error parsing command\n");
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
    // Ensure that universe will be first and present only once
    if ((next != ' ' && !empty) ||
        (store->size != 0 && store->universe == NULL)) {
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
            return error("Invalid starting character!\n");
    }
}

/**
 * Process all lines in file
 * @param fp File pointer
 * @param store Store
 * @return True if everything went well
 */
bool process_file(FILE* fp, struct store* store) {
    int allocated = INITIAL_STORE_ALLOC;

    // Loop around all lines
    for (int c = 0, i = 0; (c = getc(fp)) != EOF; i++) {
        // Realloc store
        if (i >= allocated) {
            allocated *= 2;
            const int new_alloc = allocated * sizeof(struct store_node);
            store->nodes = srealloc(store->nodes, new_alloc);
            if (store->nodes == NULL) {
                return error("Error reallocating store!\n");
            }
        }

        if (!process_line(fp, c, store)) {
            return error("Error parsing file!\n");
        }
    }
    // Check store validity
    if (!store_valid(store)) {
        return error("Invalid definition of file parts!\n");
    }
    // Run store
    if (!store_runner(store)) {
        return error("Error running commands!\n");
    }

    return true;
}
#pragma endregion
#pragma region FILE MANIPULATION
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
        error("Nepodarilo se otevrit soubor!\n");
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
        return error("Nepodarilo se uzavrit soubor!\n");
    } else {
        return true;
    }
}
#pragma endregion
#pragma region PROGRAM ARGUMENT FUNCTIONS
/*------------------------ PROGRAM ARGUMENT FUNCTIONS ------------------------*/

/**
 * Check number of arguments
 * @param argc Number of arguments
 * @return True if number of arguments is correct
 */
bool check_arguments(int argc) {
    if (argc != 2) {
        return error("Nespravny pocet argumentu!\n");
    } else {
        return true;
    }
}
#pragma endregion
#pragma region ENTRY
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
    store.nodes = malloc(sizeof(struct store_node) * INITIAL_STORE_ALLOC);
    store.universe = NULL;
    if (store.nodes == NULL) {
        error("Malloc error!\n");
        return EXIT_FAILURE;
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
#pragma endregion
