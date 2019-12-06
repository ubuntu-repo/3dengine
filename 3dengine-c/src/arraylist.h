#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAYLIST_LIMIT 1

typedef struct arraylist {
    void **elements;
    int limit;
    int length;
} arraylist;

void arraylist_init(arraylist* a) {
    a->limit = ARRAYLIST_LIMIT;
    a->length = 0;
    a->elements = malloc(a->limit * sizeof(void*));
    if (!a->elements) {
        fprintf(stderr, "Error trying to allocate memory for arraylist.");
    }
}

int arraylist_length(arraylist* a) {
    return a->length;
}

void arraylist_resize(arraylist* a, int limit) {
     void **elements = realloc(a->elements, limit * sizeof(void*));
     if (elements) {
         a->elements = elements;
         a->limit = limit;
     } else {
         fprintf(stderr, "Error trying to resize the arraylist.");
     }
}

void arraylist_update(arraylist* a, int index, void* element) {
    if (index >= 0 && index < a->length) {
        a->elements[index] = element;
    }
}

void arraylist_swap(arraylist* a, int i, int j) {
    void* tmp = a->elements[i];
    a->elements[i] = a->elements[j];
    a->elements[j] = tmp;
}

void arraylist_add(arraylist* a, void* element) {
    if (a->length == a->limit) {
        arraylist_resize(a, a->limit * 2);
    }
    a->elements[a->length++] = element;
}

void *arraylist_get(arraylist* a, int index) {
    if (index >= 0 && index < a->length) {
        return a->elements[index];
    }
    return NULL;
}

void arraylist_remove(arraylist* a, int index) {
    int i = 0;
    if (index < 0 || index >= a->length) {
        return;
    }
    a->elements[index] = NULL;

    i = index;
    while (i < a->length - 1) {
        a->elements[i] = a->elements[i + 1];
        a->elements[i + 1] = NULL;
        i++;
    }

    a->length--;

    if (a->length > 0 && a->length == a->limit / 4) {
        arraylist_resize(a, a->limit / 2);
    }
}

void arraylist_free(arraylist* a) {
    free(a->elements);
}

#endif
