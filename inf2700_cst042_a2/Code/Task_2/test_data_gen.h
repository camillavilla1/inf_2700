/** @file test_data_gen.h
 * @brief Test data generator
 * @author Weihai Yu
 */

#ifndef _TEST_DATA_GEN_H_
#define _TEST_DATA_GEN_H_

#include "schema.h"

/** create the test schema
my_schema (MyIdField: int, MyStrField:str[TEST_STR_LEN], myIntField:int) */
extern schema_p create_test_schema ( const char *name);

/** resets the test data generator.
next_test_record_id is set to 0
*/
extern void reset_test_data_gen ();

/** fill a record with some generated data. */
extern void fill_gen_record ( schema_p s, record r );

#endif
