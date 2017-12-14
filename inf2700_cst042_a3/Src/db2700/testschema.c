#include "testschema.h"
#include "test_data_gen.h"
#include "pmsg.h"

#define NUM_RECORDS 1000

/* The records generated in test_rec_write().
   Will be used in test_rec_read() to check for correctness. */
record in_recs[NUM_RECORDS];


void test_rec_write ( const char *sch_name )
{	
  put_msg (INFO, "test_rec_write (\"%s\") ...\n", sch_name);

  open_db ();
  /* put_pager_info (DEBUG, "After open_db"); */

  schema_p sch = get_schema ( sch_name );
  /* Force a fresh start. Clean what is left from a previous run. */
  if (sch != NULL) remove_schema ( sch );
  sch = create_test_schema ( sch_name );
			
  reset_test_data_gen ();

  int rnr;
  for (rnr = 0; rnr < NUM_RECORDS; rnr++)
  {
    /* put_msg (DEBUG, "rnr %d  ", rnr); */
    /* fill the record with new generated data */
    in_recs[rnr] = new_record ( sch );
    fill_gen_record (sch, in_recs[rnr]);
    append_record (in_recs[rnr], sch);
    /* put_pager_info (DEBUG, "After writing a record"); */
  }

  /* put_pager_info (DEBUG, "Before page_terminate"); */
  put_db_info (DEBUG);
  close_db ();
  /* put_pager_info (DEBUG, "After close_db"); */
	
  put_pager_profiler_info (INFO);
  put_msg (INFO,  "test_rec_write() done.\n\n");
}

void test_rec_read ( const char *sch_name )
{
  put_msg (INFO,  "test_rec_read (\"%s\") ...\n", sch_name);
	
  open_db ();
  /* put_pager_info (DEBUG, "After open_db"); */
	
  schema_p sch = get_schema ( sch_name );
  record out_rec = new_record ( sch );
  set_tbl_position ( get_table (sch_name), TBL_BEG );
  int rnr;

  for (rnr = 0; rnr < NUM_RECORDS; rnr++)
    {
      get_record (out_rec, sch);
      if ( !equal_record( out_rec, in_recs[rnr], sch) )
	{
	  put_msg (FATAL, "test_rec_read:\n");
	  put_record_info (FATAL, out_rec, sch);
	  put_msg (FATAL, "should be:\n");
	  put_record_info (FATAL, in_recs[rnr], sch);
	  exit (EXIT_FAILURE);
	}
      release_record (in_recs[rnr], sch);
      /* put_pager_info (DEBUG, "After reading a record"); */
    }

  release_record (out_rec, sch);

  /* put_pager_info (DEBUG, "Before page_terminate"); */
  put_pager_profiler_info (INFO);
  close_db ();
  /* put_pager_info (DEBUG, "After close_db"); */
	
  put_msg (INFO,  "test_rec_read() succeeds.\n");
	
}
