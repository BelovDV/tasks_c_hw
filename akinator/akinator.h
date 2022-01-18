#ifndef AKINATOR_HEADER
#define AKINATOR_HEADER

/**
 * @brief initialize data for akinator
 */
void* akinator_create();
/**
 * @brief load data for akinator
 */
void* akinator_load(const char* filename);

/**
 * @brief main function
 * @param akinator should be allocated with akinator create/load
 * @return bool - should continue
 */
void akinator_play(void** akinator);

/**
 * @brief save data of akinator to file
 * @exception use errno to check errors
 * @exception *akinator will be NULL
 */
void akinator_store(void* akinator, const char* filename);

#endif
