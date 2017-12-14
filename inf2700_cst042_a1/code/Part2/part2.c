#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2

int getLine (char *prmpt, char *buff, size_t sz);


int main(int argc, char *argv[])
{
    int rc, i, ncols, ret;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	char buf[500];


	rc = sqlite3_open(argv[1], &db);

	if (rc != SQLITE_OK)
	{
	    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
	    sqlite3_close(db);
	    
	    return 1;
	}
	else
	{
		fprintf(stderr, "Welcome to sql-json.\nEnter \".help\" for instructions.\nEnter a SQL statment terminated with \";\" for a query.\nEnter \".quit\" to exit.\n");
	}
	


    /*fgets, sjekk om .help eller .quit eller sqlite ->prepare*/
    while(1)
    {
        ret = getLine("sql-json>", buf, sizeof(buf));
        if (strncmp(buf, ".quit", 5) == 0)
        {
            /*handle command .quit or .help*/
            fprintf(stderr, "Leaving sql-json\n");
            sqlite3_close(db);
            return 1;
        }
        else if (strncmp(buf, ".help", 5) == 0)
        {
            fprintf(stderr, "Instructions\n");
            continue;
        }    

        /*if nBYte arg is negativ, then ql is read
         *up to the first zero terminator*/
        rc = sqlite3_prepare_v2(db, buf, -1, &stmt, 0);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            continue;
        }  


        fprintf(stderr, "[");
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            /*Go through column so we can print out query*/
            ncols = sqlite3_column_count(stmt);
            fprintf(stderr, "{");
            for (i = 0; i < ncols; i++)
            {
                /*Print out column name and check if the type is
                 *int/float or text
                 */
                fprintf(stderr, "\"%s\": ", sqlite3_column_name(stmt, i));
                if((sqlite3_column_type(stmt, i) == SQLITE_INTEGER) || (sqlite3_column_type(stmt, i) == SQLITE_FLOAT))
                {
                    fprintf(stderr, "%s, ", sqlite3_column_text(stmt, i));
                }
                else if(sqlite3_column_type(stmt, i) == SQLITE_TEXT)
                {
                    fprintf(stderr, "\"%s\"", sqlite3_column_text(stmt, i));
                }
            }
            
        fprintf(stderr, "}, ");
        fprintf(stderr, "\n");
        }
        fprintf(stderr, "]");
        fprintf(stderr, "\n");

        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    
    return 0;
}


int getLine (char *prmpt, char *buff, size_t sz) {
  int ch, extra;
  size_t len;
 
  // Get line with overrun protection.
 
  if (prmpt != NULL) {
    printf ("%s", prmpt);
    fflush (stdout);
  }
 
  if (fgets (buff, sz, stdin) == NULL)
    return NO_INPUT;
 
  // Line without newline is too long.
 
  len = strlen (buff);
  if (buff[len-1] != '\n') {
    // Consume characters to line end.
 
    extra = 0;
    while ((ch = getchar()) != '\n') {
      if (ch == EOF) break;
      extra = 1;
    }
 
    // If extra was just newline, okay.
 
    return (extra == 1) ? TOO_LONG : OK;
  }
 
  // Remove newline.
 
  buff[len-1] = '\0';
  return OK;
}