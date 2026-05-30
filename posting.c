#include <string.h>
#include "posting.h"

Vector* createPostingList(void) {
    return createVector(sizeof(PostingEntry));
}

void appendPosting(Vector* list, int doc_id) {
    PostingEntry entry;
    entry.doc_id = doc_id;
    appendVectorItem(list, &entry);
}

Vector* clonePostingList(const Vector* src) {
    if (!src) return NULL;
    Vector* clone = createPostingList();
    for (size_t i = 0; i < src->size; i++)
        appendVectorItem(clone, getVectorItem((Vector*)src, i));
    return clone;
}
