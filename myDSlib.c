// CODE 1: Include necessary library(ies)
#include <stdlib.h>
#include "myDSlib.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// -----------------
Record *table = NULL;  // DO NOT CHANGE: A global pointer to save the read data from csv files (a pointer to array of Records)
size_t table_size = 0; // DO NOT CHANGE: A global variable showing the number of rows (number of Records) read from csv files (rows in table)
// -----------------

// CODE 2: ADD more global variables if you need

// CODE 3: Implement all the functions here

unsigned int hash_string(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return (unsigned int)(hash % INDEX_SIZE);
}


void read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        exit(1);
    }

    char line[2000]; //initialize a size
    while (fgets(line, sizeof(line), file)!= NULL) { // read each line from the file until your at NULL
        Record *new_table = realloc(table, (table_size + 1) * sizeof(Record)); // allocate more memory as table grows
        if (new_table == NULL) {
            printf("Error: Failed to allocate memory for the new_table.\n");
            fclose(file);
            exit(1);
        }
        table = new_table;
        char district_buf[MAX_FIELD_LEN]; // allocate some memory for it, later chagned
        char date_str[10];

        int data = sscanf(line,
            "\"%38[^\"]\",\"%u\",\"%19[^\"]\",\"%8[^\"]\",\"%c\",\"%c\",\"%c\",\"%99[^\"]\",\"%99[^\"]\",\"%99[^\"]\",\"%99[^\"]\",\"%99[^\"]\",\"%99[^\"]\",\"%99[^\"]\",\"%c\",\"%c\"",
            table[table_size].transaction_id,
            &table[table_size].price,
            date_str,
            table[table_size].postcode,
            &table[table_size].property_type,
            &table[table_size].old_new,
            &table[table_size].duration,
            table[table_size].paon,
            table[table_size].saon,
            table[table_size].street,
            table[table_size].locality,
            table[table_size].town,
            district_buf,
            table[table_size].county,
            &table[table_size].record_status,
            &table[table_size].blank_col
        );


        table[table_size].district = malloc(strlen(district_buf) + 1);
        if (table[table_size].district == NULL) {
            printf("Error: Failed to allocate memory for district(after csv was read).\n");
            fclose(file);
            exit(1);
        }
        strcpy(table[table_size].district, district_buf);

        table_size++;
    }

    fclose(file);
}


IndexEntry **createIndexOnStreet(Record *table, size_t table_size){

    IndexEntry **index_table = malloc(INDEX_SIZE * sizeof(IndexEntry *));
    if (index_table == NULL) {
        printf("Error: Failed to allocate memory for the index table.\n");
        exit(1);
    }
 
    for (int i = 0; i < INDEX_SIZE; i++) {
        index_table[i] = NULL; // fills all buckets with null
    }
 
    for (size_t i = 0; i < table_size; i++) {
        unsigned int hash_value = hash_string(table[i].street);  // hash value for the street
 
        // a new index entry
        IndexEntry *new_entry = malloc(sizeof(IndexEntry));
        if (new_entry == NULL) {
            printf("Error: Failed to allocate memory for index entry.\n");
            exit(1);
        }
        new_entry->record_ptr = &table[i];  // assign the record pointer
        new_entry->next = index_table[hash_value];  // link to an existing chain if there is one, to handle collision
 
        new_entry->key = malloc(strlen(table[i].street) + 1);
        if (new_entry->key == NULL) {
            printf("Failed to allocate memeory for key.\n");
            exit(1);
        }
        strcpy(new_entry->key, table[i].street); //make sure its the same before updating
    
        index_table[hash_value] = new_entry;
    }
 
    return index_table;
}

void searchStreetLinear(Record *table, size_t table_size,const char *target_street, bool printFlagLinearSearch) {
    for (size_t i = 0; i < table_size; i++) {
        if (strcmp(table[i].street, target_street) == 0) { //strcmp to find matching name
            if (printFlagLinearSearch == true) { //fix changed to fgets
                printf("Match Found: Street = %s | Price = %u | Postcode = %s | Date = %04d-%02d-%02d\n",
                       table[i].street, //accessing the structs after going through each bucket of the street which matched
                       table[i].price,
                       table[i].postcode,
                       table[i].date.year,
                       table[i].date.month,
                       table[i].date.day);
            }
        }
    }
}


void searchStreet(IndexEntry **index_on_street, const char *target_street, bool printFlagHashIndexSearch) {
    unsigned int hash_value = hash_string(target_street); // hash for bucket
    IndexEntry *entry = index_on_street[hash_value];

    while (entry != NULL) {
        if (strcmp(entry->record_ptr->street, target_street) == 0) { //strcmp to find matching name by comparing
            if (printFlagHashIndexSearch == true) { //cfix since chagned to fgets
                printf("Match Found(linear): Street: %s | Price: %u | Postcode: %s | Date: %04d-%02d-%02d\n",
                       entry->record_ptr->street,  //using the struct entry points to, to get to record_ptr which ponints to the table to get all the values
                       entry->record_ptr->price,
                       entry->record_ptr->postcode,
                       entry->record_ptr->date.year, // year month and day were not pointers, so use . to access from within strcut
                       entry->record_ptr->date.month,
                       entry->record_ptr->date.day);
            }
        }
        entry = entry->next; // move to the next one each time and will stop when its null
    }
}

//fix
int count_unused_slots(IndexEntry **index_on_street) {
    int unused = 0;
    for (int i = 0; i < INDEX_SIZE; i++) { //goes through the array just checking for which buckets are still Null
        if (index_on_street[i] == NULL) {
            unused++; 
        }
    }
    return unused;
}

void free_index(IndexEntry **index_table) {
    for (int i = 0; i < INDEX_SIZE; i++) {
        IndexEntry *current = index_table[i];
        while (current != NULL) {
            IndexEntry *temp = current;
            current = current->next;

            // Free the copied key and the entry itself
            free(temp->key);
            free(temp);
        }
    }
    free(index_table);
}

void free_table() {
    for (size_t i = 0; i < table_size; i++) {
        free(table[i].district);  // each district was malloced
    }
    free(table);
}
