/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
#pragma once
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>

#include "config.h"
#include <malloc.h>

#define DEFAULT_UMAP_SIZE 100
/**
 * @brief unorder map entry element  
 * 
 */
typedef struct
{
    int key;
    void *entry;
} umap_entry;

/**
 * @brief hashtable/unorderedmap struct  
 * 
 */
typedef struct
{
    umap_entry *entries; // a list of umap entry pointers
    void (*free_entry)(void *entry);
    int (*add_table_entry)(void *map, void *data); // 0 for success , -1 otherwise
    void (*initialize_table)(void *map, void *arg);

    uint16_t count;
    uint16_t capacity;
    // capacity is always decided by hash table size
} unordered_map;

/**
 * @brief This function is used to create a new unordered map. It takes custom function
 *  pointers as arguments (not necessary) and an arg used to initialize table if wanted 
 * 
 * @param free_entry free entry function pointer 
 * @param add_table_entry adding to table entry function pointer 
 * @param initialize_table custom table initialization 
 * @param arg arguments passed to initialize_table function 
 * @return unordered_map* 
 */
unordered_map *umap_create(void (*free_entry)(void *entry),
                           int (*add_table_entry)(void *map, void *data),              // 0 for success , -1 otherwise
                           void (*initialize_table)(void *map, void *arg), void *arg); 

/**
 * @brief Used to free up memory and clean up map 
 * 
 * @param map 
 */
void umap_destroy(unordered_map *map);

/**
 * @brief hashing algorithm used for map  
 * 
 * @param id 
 * @param capacity 
 * @return uint32_t 
 */
uint32_t hash_key(uint32_t id, int capacity); // FNV_PRIME

/**
 * @brief used to add data to entry element inside umap_entry 
 * 
 * @param map 
 * @param data 
 * @param key 
 * @return int 0 for success -1 for failure
 */
int umap_add_to_entry(unordered_map *map, void *data, int key);
/**
 * @brief Returns entry element inside umap_entry by index 
 * 
 * @param map 
 * @param index 
 * @return void* 
 */
void *umap_get_entry_by_index(unordered_map *map, uint32_t index);

/**
 * @brief Returns entry element inside umap_entry by key  
 * 
 * @param map 
 * @param key 
 * @return void* 
 */
void *umap_get_entry_by_key(unordered_map *map, int key);

/**
 * @brief 
 * 
 * @param map 
 * @param value 
 * @param key 
 * @return int 0 for SUCCESS , - 1 FOR FAILURE 
 */
int umap_add_new(unordered_map *map, void *value, int key);

/**
 * @brief Used to expand map size by factor. (rounds down ) 
 * 
 * @param map 
 * @param factor 
 */
void umap_expand(unordered_map *map, float factor); // expand map by  factor
