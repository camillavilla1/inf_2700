#include "test_data_gen.h"
#include <stdlib.h>
#include <stdio.h>

#define TEST_STR_LEN 30

static int next_test_record_id = 0;


schema_p create_test_schema ( const char *name)
{
  schema_p sch = new_schema ( name );
  add_field ( sch, new_int_field ( "MyIdField" ));
  add_field ( sch, new_str_field ( "MyStrField", TEST_STR_LEN ));
  add_field ( sch, new_int_field ( "MyIntField" ));
  put_schema_info (DEBUG, sch);
  return sch;
}

void reset_test_data_gen ()
{
  next_test_record_id = 0;
  srand(2);
}

/** values (id, my_schema_Val_id, rand()) */
void fill_gen_record ( schema_p s, record r )
{
  char test_str[TEST_STR_LEN];
  sprintf (test_str, "%s_Val_%d", schema_name(s), next_test_record_id );
  fill_record (r, s,
	       next_test_record_id,
	       test_str,
	       rand()%100);

  next_test_record_id++;
}
