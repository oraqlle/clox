#include "table.h"
#include "common.h"
#include "memory.h"
#include "object.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static Entry *findEntry(Entry *entries, size_t capacity, ObjString *key) {
    size_t index = key->hash % capacity;
    Entry *tombstone = NULL;

    for (;;) {
        Entry *entry = &entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // Empty entry
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) {
                    tombstone = entry;
                }
            }
        } else if (entry->key == key) {
            // Found entry
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table *table, size_t capacity) {
    Entry *entries = ALLOCATE(Entry, capacity);

    for (size_t i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (size_t i = 0; i < table->capacity; i++) {
        Entry *entry = &table->entries[i];

        if (entry->key == NULL) {
            continue;
        }

        Entry *dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count += 1;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table *table, ObjString *key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        size_t capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;

    if (isNewKey) {
        table->count += 1;
    }

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

void tableAddAll(Table *from, Table *to) {
    for (size_t i = 0; i < from->capacity; i++) {
        Entry *entry = &from->entries[i];

        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
    if (table->count == 0) {
        return false;
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);

    if (entry->key == NULL) {
        return false;
    }

    *value = entry->value;
    return true;
}

bool tableDelete(Table *table, ObjString *key) {
    if (table->count == 0) {
        return false;
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);

    if (entry->key == NULL) {
        return false;
    }

    entry->key = NULL;
    entry->value = BOOL_VAL(true);

    return true;
}
