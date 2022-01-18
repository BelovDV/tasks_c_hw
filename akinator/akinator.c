#include "akinator.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define SIZE 256

typedef struct {
    // size_t key;         // check file type
    size_t size;        // size of akinator
    size_t capacity;
    size_t position;    // start position
    size_t current;    // current position
} Header;

enum Section_type {
    e_question,
    e_leaf
};

typedef struct {
    enum Section_type type; // = e_question
    size_t size;
    size_t pos_true;
    size_t pos_false;
} Section_q;
#define SZSQ (sizeof(Section_q))

typedef struct {
    enum Section_type type; // = e_question
    size_t size;
} Section_a;
#define SZSA (sizeof(Section_a))

#define KEY 1234567890123456789

// return bool was error
static int provide_space(void** akinator, size_t cap) {
    Header* head = *akinator;
    if (head->capacity > cap) return 0;
    void* vsp = realloc(*akinator, cap);
    if (!vsp) return 1;
    *akinator = vsp;
    head = vsp;
    head->capacity = cap;
    return 0;
}

// return ptr to new answer
static Section_a* add_answer(void** akinator) {
    char buffer[SIZE];
    fgets(buffer, SIZE - 2, stdin);
    fgets(buffer, SIZE - 2, stdin);
    size_t length = strlen(buffer) - 1;

    Header* head = *akinator;
    if (head->size + SZSA + length > head->capacity) {
        size_t nsz = head->size * 2 + length + SZSA;
        if (provide_space(akinator, nsz)) return NULL;
        head = *akinator;
    }

    Section_a* next = (Section_a*)(((char*)*akinator) + head->size);
    next->type = e_leaf;
    next->size = length;
    memcpy(next + 1, buffer, length);
    head->size += SZSA + length;
    return next;
}

// return ptr to new question
static Section_q* add_question(void** akinator) {
    char buffer[SIZE];
    fgets(buffer, SIZE - 2, stdin);
    fgets(buffer, SIZE - 2, stdin);
    size_t length = strlen(buffer) - 1;

    Header* head = *akinator;
    if (head->size + SZSQ + length > head->capacity) {
        size_t nsz = head->size * 2 + length + SZSQ;
        if (provide_space(akinator, nsz)) return NULL;
        head = *akinator;
    }

    Section_q* next = (Section_q*)(((char*)*akinator) + head->size);
    next->type = e_question;
    next->size = length;
    memcpy(next + 1, buffer, length);
    head->size += SZSQ + length;
    return next;
}

void* akinator_create() {
    Header* akinator = malloc(sizeof(Header));
    akinator->size = akinator->capacity = akinator->position = sizeof(Header);
    printf("print start answer\n");
    if (!add_answer((void*)&akinator)) { free(akinator); return NULL; }
    return akinator;
}

void* akinator_load(const char* filename) {
    FILE* in = fopen(filename, "r");
    fseek(in, 0, SEEK_END);
    long size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (size < 0 || (size_t)size < sizeof(Header)) {
        fclose(in);
        return NULL;
    }

    void* akinator = malloc(size);
    if (akinator || !errno) fread(akinator, 1, size, in);
    fclose(in);
    ((Header*)akinator)->capacity = size;
    return akinator;
}

#define CHECK(condition) \
    if (!(condition)) { \
        printf("ERROR: akinator corrupted\n"); \
        return; \
    }

static void akinator_change(void** akinator, size_t* to_change) {
    Header* head = *akinator;
    if (!to_change) to_change = &head->position;
    *to_change = head->size;

    printf("print separation question\n");
    Section_q* next_q = add_question(akinator);
    head = *akinator;
    printf("should be old answer to be true?\n");
    printf("1 - true\n2 - false\n");
    char sym = 0;
    while (sym != '1' && sym != '2') scanf(" %c%*[^\r\n]", &sym);
    if (sym == '1') next_q->pos_true = head->current;
    else next_q->pos_false = head->current;

    printf("print correct answer\n");
    Section_a* next_a = add_answer(akinator);
    size_t diff = (char*)next_a - (char*)*akinator;
    if (sym == '1') next_q->pos_false = diff;
    else next_q->pos_true = diff;
}

void akinator_play(void** akinator) {
    Header* head = *akinator;
    char* begin = *akinator;
    head->current = head->position;

    size_t* to_change = NULL;
    while (((Section_a*)(begin + head->current))->type == e_question) {
        CHECK(head->current + SZSQ <= head->size)
        Section_q* quest = (Section_q*)(begin + head->current);
        CHECK(head->current + SZSQ + quest->size <= head->size)
        printf("%.*s\n", (int)quest->size, (char*)(quest + 1));
        printf("1 - true\n2 - false\n");
        char symbol = 0;
        scanf(" %c%*[^\r\n]", &symbol);
        if (symbol == '1') {
            to_change = &quest->pos_true;
            head->current = quest->pos_true;
        }
        if (symbol == '2') {
            to_change = &quest->pos_false;
            head->current = quest->pos_false;
        }
    }
    Section_a* ans = (Section_a*)(begin + head->current);
    printf("is it %.*s?\n", (int)ans->size, (char*)(ans + 1));
    printf("1 - true\n2 - false\n");
    char symbol = 0;
    while (symbol != '1' && symbol != '2') scanf(" %c%*[^\r\n]", &symbol);
    if (symbol == '1') printf("it's done\n");
    else akinator_change(akinator, to_change);
}

void akinator_store(void* akinator, const char* filename) {
    FILE* out = fopen(filename, "w");
    fwrite(akinator, ((Header*)akinator)->size, 1, out);
    fclose(out);
}
