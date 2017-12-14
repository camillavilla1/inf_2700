/************************************************************
 * Interpreter for db2700 in the Databases course INF-2700  *
 * UIT - The Arctic University of Norway                    *
 * Author: Weihai Yu                                        *
 ************************************************************/

#include "interpreter.h"
#include "schema.h"
#include "pmsg.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* t_database = "database";
static const char* t_show = "show";
static const char* t_print = "print";
static const char* t_create = "create";
static const char* t_drop = "drop";
static const char* t_insert = "insert";
static const char* t_select = "select";
static const char* t_quit = "quit";
static const char* t_help = "help";
static const char* t_int = "int";
static const char* t_str = "str";

static FILE *in_s; /* input stream, default to stdin */

static char front_dir[512];  /* dir where front is running */
static char cmd_file[200];   /* the file where front gets commands */
static char rest_of_line[300]; /* used to read the rest of line */

static void init_with_options ( int argc, char* argv[] )
{
  int c;
  char new_sys_dir[512];

  getcwd (front_dir, sizeof (front_dir));
  new_sys_dir[0] = '\0';
  cmd_file[0] = '\0';
  msglevel = INFO;

  while ((c = getopt (argc, argv, "hm:d:c:")) != -1)
    switch (c)
      {
      case 'h':
	printf ("Usage: runtest [switches]\n");
	printf ("\t-h           help, print this message\n");
	printf ("\t-m [fewid]   msg level [fatal,error,warn,info,debug]\n");
	printf ("\t-d db_dir    default to ./tests/testfront\n");
	printf ("\t-c cmd file  eg. ./tests/testcmd.dbcmd, default to stdin\n");
	exit (0);
      case 'm':
	switch (optarg[0])
	  {
	  case 'f': msglevel = FATAL; break;
	  case 'e': msglevel = ERROR; break;
	  case 'w': msglevel = WARN; break;
	  case 'i': msglevel = INFO; break;
	  case 'd': msglevel = DEBUG; break;
	  }
	break;
      case 'd':
	strcpy (new_sys_dir, optarg);
      case 'c':
	strcpy (cmd_file, optarg);
	break;
      case '?':
	if (optopt == 'm' || optopt == 'd' || optopt == 'c')
	  printf ("Option -%c requires an argument.\n", optopt);
	else if (isprint (optopt))
	  printf ("Unknown option `-%c'.\n", optopt);
	else
	  printf ("Unknown option character `\\x%x'.\n", optopt);
	abort ();
      default:
	abort ();
      }
	
  if ( cmd_file[0] == '\0' )
      in_s = stdin;
  else
    {
      in_s = fopen ( cmd_file, "r");
      if (in_s == NULL)
	{
	  printf ("Cannot open file: %s\n", cmd_file);
	  in_s = stdin;
	} else
	put_msg (DEBUG, "file \"%s\" is open for read.\n", cmd_file);
    }

  if ( in_s == stdin )
    {
      printf ("Welcome to db2700 session\n");
      printf ("  - Enter \"help\" for instructions\n");
      printf ("  - Enter \"quit\" to leave the session\n");
    }
	
  if (new_sys_dir[0] == '\0')
    strcpy (new_sys_dir, "./tests/testfront");

  if ( set_system_dir (new_sys_dir) == -1 )
    {
      put_msg (ERROR, "cannot set database at %s\n", new_sys_dir);
      exit (EXIT_FAILURE);
    }
}

static void skip_line ()
{
  fgets(rest_of_line, 200, in_s);
}

static void syntax_error ( char *near_str )
{
  put_msg (ERROR, "Syntax error near\n");
  put_msg (ERROR, "  ... >>>%s<<<", near_str);
  if (fgets(rest_of_line, 200, in_s))
    append_msg (ERROR, rest_of_line);
}

static void show_help_info ()
{
  printf ("You can run the following commands:\n");
  printf (" - help\n");
  printf (" - quit\n");
  printf (" - # some comments in the res of a line\n");
  printf (" - print text\n");
  printf (" - database /the/place/of/yourdb\n");
  printf (" - show database\n");
  printf (" - create table table_name ( field_name field_type, ... )\n");
  printf (" - drop table table_name (CAUTION: the file will be deleted!!!)\n");
  printf (" - insert into table_name values ( value_1, value_2, ... )\n\n");
}

static void set_database ()
{
  char new_sys_dir[512];

  if (fscanf (in_s, "%s", new_sys_dir) == 1)
    {
      put_msg(DEBUG, "database at: \"%s\".\n", new_sys_dir);
      if ( (chdir (front_dir) == -1) || (set_system_dir (new_sys_dir) == -1) )
	{
	  put_msg (ERROR, "cannot set database at \"front_dir/%s\"\n",
		   front_dir, new_sys_dir);
	  exit (EXIT_FAILURE);
	}
    }
  else
    {
      put_msg (ERROR, "no database provided.\n");
      exit (EXIT_FAILURE);
    }

  open_db ();
}

static void quit ()
{
  close_db();
  if ( in_s != stdin )
    fclose (in_s);  
}

static void show_database ()
{
  char db_token[20];
  if ( fscanf (in_s, "%s", db_token) != 1)
    {
      put_msg (ERROR, "Show what?\n");
      return;
    }
  if ( strcmp (db_token, t_database) != 0 )
    {
      put_msg (ERROR, "Cannot show \"%s\".\n", db_token );
      return;
    }
  put_db_info (FORCE);
}

static void print_str ()
{
  if (fgets(rest_of_line, 200, in_s))
      put_msg (FORCE, "%s\n", rest_of_line);
}

static void create_tbl ()
{
  char tbl_name[30], field_name[30], field_type[4], separator[5];
  int len;
  if ( fscanf (in_s, " table %s (", tbl_name) != 1)
    {
      put_msg (ERROR, "Do not know what to create\n");
      return;
    }

  put_msg (DEBUG, "table name: \"%s\".\n", tbl_name);

  schema_p sch = get_schema (tbl_name);
  
  if ( sch != NULL )
    {
      put_msg (ERROR, "Table \"%s\" already exists.\n", tbl_name);
      skip_line ();
      return;      
    }
  sch = new_schema (tbl_name);
  while (!feof(in_s))
    {
      fscanf (in_s, "%s %3s", field_name, field_type);
      if ( strcmp (field_type, t_int) == 0 )
	{
	  add_field ( sch, new_int_field ( field_name ));
	}
      else if ( strcmp (field_type, t_str) == 0 )
	{
	  fscanf (in_s, "(%d)", &len);
	  add_field ( sch, new_str_field ( field_name, len ));
	}
      else
	{
	  strcat (field_name, " ");
	  strcat (field_name, field_type);
	  syntax_error (field_name);
	  remove_schema (sch);
	  return;
	}
      fscanf (in_s, "%s", separator);
      put_msg(DEBUG, "seperator: \"%s\".\n", separator);
      if (separator[0] == ')')
	{
	  skip_line ();
	  break;
	}
    }
}

static void trim_pending_semicolon ( char *str )
{
  int len = strlen (str);
  if ( str[len - 1] == ';')
    str[len - 1] = '\0';
}

static void drop_tbl ()
{
  char tbl_name[30];
  fscanf (in_s, " table %s;", tbl_name);
  trim_pending_semicolon (tbl_name);
  remove_table ( get_table(tbl_name) );
}

/* trim the ending ',', ')' or ");" of a string value */
static void trim_str_val_end ( char *str_val )
{
  int i = strlen (str_val) - 1;
  while (i > 0
	 && (str_val[i] == ',' || str_val[i] == ')' || str_val[i] == ';'))
    i--;
  str_val[i+1] = '\0';
}

static record new_filled_record ( schema_p s )
{
  record r = new_record ( s );
  int int_val;
  char str_val[MAX_STR_LEN];
  field_desc_p fld_d;
  int i = 0;
  for ( fld_d = schema_first_fld_desc(s);
	fld_d != NULL;
	fld_d = field_desc_next(fld_d), i++)
    {
      if ( is_int_field (fld_d) )
	{
	  fscanf (in_s, "%d,", &int_val);
	  assign_int_field (r[i], int_val);
	}
      else
	{
	  fscanf (in_s, "%s,", str_val);
	  trim_str_val_end ( str_val );
	  assign_str_field (r[i], str_val);
	}
    }
  skip_line ();
  put_record_info (DEBUG, r, s);
  return r;
}

static void insert_row ()
{
  char tbl_name[30];
	
  fscanf (in_s, " into %s values (", tbl_name);
  put_msg(DEBUG, "table name: \"%s\".\n", tbl_name);
  schema_p sch = get_schema (tbl_name);
  if (!sch)
    {
      put_msg (ERROR, "Schema \"%s\" does not exist.\n", tbl_name);
      skip_line ();
      return;
    }
  /* put_schema_info (DEBUG, sch); */
  record rec = new_filled_record ( sch );

  append_record (rec, sch);
  release_record (rec, sch);
}

/* split str into substrings, separated by char c,
   which is not white space. The substring has no white space. */
static int str_split ( char *str, char c, char *arr[], int max_count )
{
  char *p;

  p = str;
  while ( (*p == ' ' || *p == '\t' || *p == '\n') && *p != '\0' )
    p++;

  if (*p == '\0') return 0;

  int count = 0;
  
  while (*p != '\0')
    {
      if (*p == c) /* end of substring */
        {
	  *p = '\0';
	  if (arr[count] == NULL) /* empty substring */
	    arr[count] = p;
	  count++;
	  if ( count > max_count )
	    {
	      put_msg (ERROR, "str_split: more substring tha allowed %d",
		       max_count);
	      return -1;
	    }
        }
      else if (*p == ' ' || *p == '\t' || *p == '\n')
	{
	  if (arr[count] != NULL) /* white space after substring */
	    *p = '\0';
	}
      else if (arr[count] == NULL) /* start of new substring */
	arr[count] = p;
      p++;
    }

  return count + 1;
}

static void select_rows ()
{
  char in_str[200] = "", from_str[100] = "";
  char *where_str, *p;
  char attr[11] = "", op[3] = "";
  int val;
  tbl_p where_tbl = NULL, res_tbl = NULL, from_tbl = NULL;
  char *attrs[10];
  int num_res_attrs = 0, i;
  for ( i = 0; i < 10; i++ ) attrs[i] = NULL;

  fscanf (in_s, "%[^;];", in_str);

  p = strstr (in_str, " from ");
  if ( p == NULL )
    {
      put_msg (ERROR, "select %s: from which table to select?\n", in_str);
      return;
    }
  *p = '\0';
  p += 6;
  if ( sscanf (p, "%s", from_str) != 1 )
    {
      put_msg (ERROR, "select from what?\n");
      return;
    }

  num_res_attrs = str_split ( in_str, ',', attrs, 10);
  if (num_res_attrs == 0)
    {
      put_msg (ERROR, "select from %s: select what?\n", p);
      return;
    }

  p += 2;
  where_str = strstr (p, " where ");
  if ( where_str != NULL ) where_str += 7;

  put_msg (DEBUG, "from: \"%s\", where: \"%s\"\n", from_str, where_str);
  from_tbl = get_table (from_str);
  if (from_tbl == NULL)
    {
      put_msg (ERROR, "select: table \"%s\" does not exist.\n", from_str);
      return;
    }

  if ( where_str != NULL )
    {
      if (sscanf ( where_str, "%s %s %d", attr, op, &val) != 3)
	{
	  put_msg (ERROR, "query \"%s\" is not supported.\n", where_str);
	  return;
	}
      where_tbl = table_search ( from_tbl, attr, op, val);
      if (where_tbl == NULL)
	return;
    }

  if ( attrs[0][0] == '*' )
    table_display (where_tbl ? where_tbl : from_tbl);
  else
    {
      res_tbl = table_project ( where_tbl ? where_tbl : from_tbl,
				"tmp_res_01", num_res_attrs, attrs);
      table_display (res_tbl);
    }

  remove_table (where_tbl);
  remove_table (res_tbl);
}

void interpret ( int argc, char* argv[] )
{
  init_with_options (argc, argv);

  char token[30];
	
  while (!feof(in_s))
    {
      fscanf (in_s, "%s", token);
      put_msg (DEBUG, "current token is \"%s\".\n", token);
      if (strcmp(token, t_quit) == 0)
	{ quit(); break;}
      if (token[0] == '#')
	{ skip_line (); continue; }
      if (strcmp(token, t_help) == 0)
	{ show_help_info (); continue; }
      if (strcmp(token, t_database) == 0)
	{ set_database(); continue; }
      if (strcmp(token, t_show) == 0)
	{ show_database(); continue; }
      if (strcmp(token, t_print) == 0)
	{ print_str(); continue; }
      if (strcmp(token, t_create) == 0)
	{ create_tbl(); continue; }
      if (strcmp(token, t_drop) == 0)
	{ drop_tbl(); continue; }
      if (strcmp(token, t_insert) == 0)
	{ insert_row(); continue; }
      if (strcmp(token, t_select) == 0)
	{ select_rows(); continue; }
      syntax_error (token);
    }
}

