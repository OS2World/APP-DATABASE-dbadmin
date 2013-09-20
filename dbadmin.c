/* 
**	dbadmin.c	-
**
**
** Copyright (c) 1996 James E. Harrell, Jr.
**
** This software is provided "as is" without any expressed or implied warranty.
**
**
*/

#include <stdio.h>
#include <sys/types.h>
#include "cgic.h"
#include "msql.h"
#include "dbadmin.h"
#include <string.h>

char *thetable;
int table_name_length;
char *thedb;
int  name_length;

int       state;
int       action;
int       numfields;
char      *fieldnames;
int       sock;
char      field[MAX_FIELDS][MAX_FIELDNAME_LEN];
char      type[MAX_FIELDS][5];
char      lengthStr[MAX_FIELDS][5];
char      notnul[MAX_FIELDS][2];
char      key[MAX_FIELDS][2];
char      operation[5];
m_result  *res;
m_result  *res2;
m_row     cur;
m_field   *curField;
int       dberror;
char      qbuf[MAX_TEXTQUERY_LEN];
cgiFormResultType cgiError;

void unimplementedOption(void);
void giveDebugInfo(void);
int  getState(void);
void giveTitle(void);
void connectMSQL(void);
void getNewDBName(void);
void connectDB(void);
void connectTable(void);
void doTitlePage(void);
void doAdminSelection(void);
void showDatabases(int);
void addDatabase(void);
void doAddDB(void);
void doRemoveDB(void);
void removeDatabase(void);
void modifyStructure(void);
void doModifyStructure(void);
void showCurrentTables(void);
void giveAddTable(void);
void doRemoveTable(void);
void doAddTable(void);
void addTableNow(void);
void getFields(void);
void viewUpdateInfo(void);
void doTableSelection(void);
void giveSearch(void);
void giveInsert(void);
void showSelectData(void);
void giveBackQueryPage(void);

void showTables(void);
void giveCannedSelect(void);
void giveTextQuery(void);
void doTextQuery(void);
void doCannedQuery(void);
void addCannedQuery(void);
void giveInsertForm(void);
void searchRecords(void);
void giveSearchForm(void);
void returnSearch(void);
void getSearchSpec(void);
void doSearchQuery(void);
void showSelectDeleteData(void);
void deleteSearchItem(void);
void performDelete(void);

void updateSearchItem(void);
void performUpdate(void);
void giveBackOriginalQuery(void);

void giveInitialActions(void);
void giveButtonBar(void);

int cgiMain() {
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>James Harrell's Database Administration Toolkit</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>\n");
 
        switch (getState())
        {
          case 0: doTitlePage();
                  break;

          case 1: doAdminSelection();
                  break;

          case 2: doAddDB();                    /* ADD DATABASE */
                  break;

          case 3: doRemoveDB();                 /* REMOVE DATABASE */
                  break;
 
          case 4: doModifyStructure();          /* MODIFY TABLES */
                  break;

          case 5: doRemoveTable();              /* DELETE TABLE */
                  break;

          case 6: doAddTable();                 /* ADD TABLE FORM */
                  break;

          case 7: addTableNow();                /* REALLY ADD TABLE */
                  break;
 
          case 8: doTableSelection();
                  break;

          case 9: doTextQuery();		/* Submit Hand query */
                  break;

          case 10: doCannedQuery();             /* Add info to table, etc. */
                  break;
 
          case 11: addCannedQuery();             /* Do the add */
                   break;
 
          case 12: searchRecords();		/* Perform canned search */
                   break;

          case 14: returnSearch();		/* Return data */
                   break;

          case 15: deleteSearchItem();		/* Return data */
                   break;

          case 16: updateSearchItem();		/* Return data */
                   break;

	  default: unimplementedOption();
        }
        giveButtonBar();
#ifdef DEBUG
        giveDebugInfo();
#endif
	fprintf(cgiOut, "</BODY></HTML>\n");
	return 0;
}

void giveDebugInfo(void)
{
  fprintf(cgiOut,"<HR>\n");
  fprintf(cgiOut,"<CENTER><H3>DEBUG INFORMATION</H3></CENTER>\n");
  fprintf(cgiOut,"State = %d\n",state);
  fprintf(cgiOut,"<HR>\n");
}

int getState(void)
{
  cgiFormInteger("state",&state,0);
  return state;
}

void connectMSQL(void)
{
#ifdef DEBUG
  fprintf(cgiOut,"<BR>Connecting to msqld on localhost.\n<BR>");
#endif
  if ((sock = msqlConnect(NULL)) == -1)
  {
    fprintf(cgiOut,"\nError connecting to database : <BR>\n%s\n\n",msqlErrMsg);
    giveButtonBar();
    exit(1);
  }
}

void getNewDBName(void)
{
#ifdef DEBUG
  fprintf(cgiOut,"<BR>Getting space requirements for database name.\n<BR>\n");
#endif
 if ((cgiError=cgiFormStringSpaceNeeded("thedb",&name_length))==cgiFormNotFound)
    { 
    fprintf(cgiOut,"You have requested to create a database without\n");
    fprintf(cgiOut,"specifying a database name!\n");
    giveButtonBar();
    exit(1);
    }
  #ifdef DEBUG
  fprintf(cgiOut,"<BR>Currently allocating %d bytes of memory.<BR>\n",name_length);
  #endif
  thedb = (char *)malloc(sizeof(char)*name_length);
  cgiError=cgiFormString("thedb",thedb,name_length);
}

void connectDB(void)
{
#ifdef DEBUG
  fprintf(cgiOut,"<BR>Getting space requirements for database name.\n<BR>\n");
#endif
 if ((cgiError=cgiFormStringSpaceNeeded("thedb",&name_length))==cgiFormNotFound)
    { 
    fprintf(cgiOut,"You have requested to connect to a database without\n");
    fprintf(cgiOut,"specifying a database name!\n");
    giveButtonBar();
    exit(1);
    }
  #ifdef DEBUG
  fprintf(cgiOut,"<BR>Currently allocating %d bytes of memory.<BR>\n",name_length);
  #endif
  thedb = (char *)malloc(sizeof(char)*name_length);
  cgiError=cgiFormString("thedb",thedb,name_length);
  if(msqlSelectDB(sock,thedb) < 0)
    {
    fprintf(cgiOut,"ERROR selecting database %s<BR>\n",thedb);
    fprintf(cgiOut,"\n%s<BR>\n\n",msqlErrMsg);
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }
  #ifdef DEBUG
  fprintf(cgiOut,"<BR>Connected to database %s.\n<BR>",thedb);
  #endif
}

void connectTable(void)
{
  if ((cgiError=cgiFormStringSpaceNeeded("thetable",&table_name_length)) ==
    cgiFormNotFound)
    {
    fprintf(cgiOut,"You have requested to connect to a table in database %s\n",thedb);
    fprintf(cgiOut,"without specifying the table!\n");
    giveButtonBar();
    exit(1);
    }
  thetable = (char *)malloc(sizeof(char)*table_name_length);
  cgiError=cgiFormString("thetable",thetable,table_name_length);
  #ifdef DEBUG
  fprintf(cgiOut,"<BR>All queries currently apply to table %s.<BR>\n",thetable);
  #endif
}
 
void giveButtonBar(void)
{
  fprintf(cgiOut,"<HR>\n");
  fprintf(cgiOut,"<CENTER>\n");
  fprintf(cgiOut,"<TABLE CELLPADDING=0 BORDER=4>\n");
  fprintf(cgiOut,"<TR>");
  fprintf(cgiOut,"<TH WIDTH=20%%><A HREF=\"%s/dbadmin?state=-1\">CONNECT SERVER</A></TH>\n",RELPATH); 
  fprintf(cgiOut,"<TH WIDTH=20%%><A HREF=\"%s/dbadmin\">CONNECT DATABASE</A></TH>\n",RELPATH);
  fprintf(cgiOut,"<TH WIDTH=20%%><A HREF=\"%s/dbadmin?state=1&action=1\">ADD DATABASES</A></TH>\n",RELPATH);
  fprintf(cgiOut,"<TH WIDTH=20%%><A HREF=\"%s/dbadmin?state=1&action=2\">REMOVE DATABASES</A></TH>\n",RELPATH);
  /* fprintf(cgiOut,"<TH WIDTH=20%%><A HREF=\"%s/dbadmin?state=1&action=3\">VIEW/MODIFY STRUCTURE</A></TH>\n",RELPATH); */
  /* fprintf(cgiOut,"<TH WIDTH=20%%><A HREF=\"%s/dbadmin?state=1&action=4\">VIEW/MODIFY DATABASE</A></TH>\n",RELPATH); */
  fprintf(cgiOut,"</TR>");
  fprintf(cgiOut,"</TABLE><HR>\n");
}

/* -------------------------- UNIMPLEMENTED ACTIONS --------------------- */
/*                                STATE = -1                              */
void unimplementedOption(void)
{
  giveTitle();
  fprintf(cgiOut,"<HR><H3><CENTER>YOU HAVE REQUESTED AN UNIMPLEMENTED\n");
  fprintf(cgiOut,"OPTION. PLEASE LOOK FOR THIS FUNCTIONALITY IN A FUTURE\n");
  fprintf(cgiOut,"RELEASE...</CENTER></H3><HR>\n");
}


/* -------------------------- INITIAL ACTIONS PAGE ---------------------- */
/*                                STATE = 0                               */

void doTitlePage(void)
{
  giveTitle();
  connectMSQL();
  giveInitialActions();
  msqlClose(sock);
}

void giveTitle(void) 
{
#ifdef DEBUG
  fprintf(cgiOut,"<BR>Now entering \"giveTitle\"<BR>\n");
#endif
  fprintf(cgiOut,"<H1>mSQL DataBase Administration %s</H1>\n",VERSION);
  fprintf(cgiOut,"<HR>\n");
  fprintf(cgiOut,"<CENTER><EM>This database administration tool is NOT\n");
  fprintf(cgiOut,"freeware. Please license any distribution. See the file\n");
  fprintf(cgiOut,"license.doc for further information! ALL commercial sites\n");
  fprintf(cgiOut,"are required to register their copy!</EM></CENTER><HR>\n");
}

void giveInitialActions(void)
{
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
  fprintf(cgiOut,"<H3>Please select a database:</H3>\n");
  showDatabases(RADIO_LIST);
  fprintf(cgiOut,"<H3>Please select an action:</H3>\n");
  fprintf(cgiOut,"<DD><INPUT TYPE=RADIO NAME=\"state\" VALUE=8 CHECKED> Query / Update Database...<BR>\n");
  fprintf(cgiOut,"<DD><INPUT TYPE=RADIO NAME=\"state\" VALUE=4> Add / Remove Table Definitions...<BR>\n");
  fprintf(cgiOut,"<P><CENTER><INPUT TYPE=SUBMIT VALUE=\"Connect to Database...\"></CENTER>\n");
  fprintf(cgiOut,"</FORM>\n<HR><HR>\n");
}

/* -------------------------- DO ADMIN SELECTION --------------------------- */
/*                                STATE = 1                                  */

void doAdminSelection(void)
{
  giveTitle();
  connectMSQL();
  cgiError=cgiFormInteger("action",&action,0);
  switch(action)
    {
    case ADD_DB: addDatabase();
      break;
    case REM_DB: removeDatabase();
      break;
    default : fprintf(cgiOut,"\nERROR : No Action Specified!<BR>\n");
    }
}

void addDatabase(void)
{
  fprintf(cgiOut,"<H3>The following databases currently exist:</H3>\n");
  showDatabases(NORMAL);
  fprintf(cgiOut,"<P><H3>Add Database:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=2>");
  fprintf(cgiOut,"<CENTER><TABLE BORDER=0>\n");
  fprintf(cgiOut,"<TR WIDTH=85%%>\n");
  fprintf(cgiOut,"<TD ALIGN=CENTER WIDTH=33%%>New Database Name:</TD>\n");
  fprintf(cgiOut,"<TD ALIGN=CENTER WIDTH=33%%><INPUT NAME=\"thedb\" SIZE=25></TD>\n");
  fprintf(cgiOut,"<TD ALIGN=CENTER WIDTH=33%%><INPUT TYPE=submit VALUE=\"Add Database\"></TD>\n");
  fprintf(cgiOut,"</TR></TABLE></CENTER>\n");
}

void removeDatabase(void)
{
  fprintf(cgiOut,"<H3>Remove Database:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=3>");
  fprintf(cgiOut,"Please select a database to remove:\n<BR>\n");
  connectMSQL();
  showDatabases(RADIO_LIST);
  fprintf(cgiOut,"<CENTER><H4>THIS OPERATION CANNOT BE UNDONE!!</H4>\n");
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Remove Database\"></CENTER>\n<BR>\n");
  msqlClose(sock);
}

void modifyStructure(void)
{
  fprintf(cgiOut,"<H3>View/Modify Database Structure:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=4>");
  fprintf(cgiOut,"Please select a database to view/modify structure:\n<BR>\n");
  connectMSQL();
  showDatabases(RADIO_LIST);
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"View/Modify Structure\">\n<BR>\n");
  msqlClose(sock);
}

void viewUpdateInfo(void)
{
  fprintf(cgiOut,"<H3>View or Query Database:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=8>");
  fprintf(cgiOut,"Please select a database to view or query:\n<BR>\n");
  connectMSQL();
  showDatabases(RADIO_LIST);
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Connect Database\">\n<BR>\n");
  msqlClose(sock);
}

void showDatabases(int listVersion)
{
int x=0;
  res = msqlListDBs(sock);
  if (!res)
    {
    fprintf(cgiOut,"\nError : Couldn't get database list!\n<BR>\n");
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }
  if (listVersion==NORMAL) { fprintf(cgiOut,"<UL>\n"); }
  while((cur = msqlFetchRow(res)))
    { 
    x++;
    switch(listVersion)
      { 
      case NORMAL: fprintf(cgiOut,"<LI> %s<BR>\n",cur[0]);
           break;
      case RADIO_LIST: fprintf(cgiOut,"<DD><INPUT TYPE=RADIO NAME=\"thedb\" VALUE=\"%s\"",cur[0]);
                       if (x==1) fprintf(cgiOut," CHECKED");
                       fprintf(cgiOut,"> %s<BR>\n",cur[0]);
                       
           break;
      default: fprintf(cgiOut,"%s<BR>\n");
      }
    }
  if (listVersion==NORMAL) { fprintf(cgiOut,"</UL>\n"); }
  msqlFreeResult(res);
}

/* --------------------------- DO ADD DATABASE ---------------------------- */
/*                               STATE = 2                                  */
void doAddDB(void)
{
  giveTitle();
  connectMSQL();
  getNewDBName();

  if(msqlCreateDB(sock,thedb) < 0)
      {
      fprintf(cgiOut,"\n<BR>mSQL Command Failed!\n<BR>Server Error = %s\n<BR>\n",msqlErrMsg);
      msqlClose(sock);
      giveButtonBar();
      exit(1);
      }
     else
      { fprintf(cgiOut,"<BR>Database \"%s\" created.<BR>\n",thedb); }
  addDatabase();  
  msqlClose(sock);
}


/* -------------------------- REMOVE DATABASE ----------------------------- */
/*                               STATE = 3                                  */

void doRemoveDB(void)
{
  giveTitle();
  connectMSQL();
  connectDB();

  if(msqlDropDB(sock,thedb) < 0)
    {     
    fprintf(cgiOut,"\nmSQL Command failed!\n<BR>Server error = %s\n<BR>\n",msqlErrMsg);
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }
   else
    { fprintf(cgiOut,"Database \"%s\" dropped!\n<BR>\n",thedb); }
  removeDatabase();
  msqlClose(sock);
}

/* ----------------------- MODIFY TABLE STRUCTURE ------------------------- */
/*                               STATE = 4                                  */

void doModifyStructure(void)
{
  giveTitle();
  connectMSQL();
  connectDB();

  if (msqlSelectDB(sock,thedb) < 0)
    {
    fprintf(cgiOut,"ERROR selecting database %s\n<BR>\n",thedb);
    fprintf(cgiOut,"\n%s<BR>\n\n",msqlErrMsg);
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }
   else
    {
/*  if TABLESEXIST */
    showCurrentTables();
    giveAddTable();
    }
  msqlClose(sock);
}

void showCurrentTables(void)
{
  fprintf(cgiOut,"<H3>The following table definitions exist for database: %s</H3>",thedb);
  fprintf(cgiOut,"<HR>\n");
  res = msqlListTables(sock);
  if (!res)
    {
   fprintf(cgiOut,"\nERROR : Unable to list tables in database %s<BR>\n",thedb);
    giveButtonBar();
    exit(1);
    }
   else
    {
    dberror=msqlNumRows(res);
    if (dberror<1)
      { fprintf(cgiOut,"<CENTER><H3>No tables in database.</H3></CENTER>\n<BR>\n"); }
    else 
      {
      while((cur=msqlFetchRow(res)))
        {
        res2 = msqlListFields(sock,cur[0]);
        if (!res2)
          {
          fprintf(cgiOut,"ERROR : Couldn't find %s in %s!<BR>\n",cur[0],thedb);
          msqlFreeResult(res);
      giveButtonBar();
          exit(1);
          }
         else
          {
          fprintf(cgiOut,"<BR><CENTER>\n");
          fprintf(cgiOut,"<TABLE CELLPADDING=3 BORDER=5>\n");
          fprintf(cgiOut,"<TR WIDTH=80%%><TH COLSPAN=5>TABLE: %s</TH></TR>\n",cur[0]);
          fprintf(cgiOut,"<TR><TH WIDTH=15%%>FIELD</TH>");
          fprintf(cgiOut,"<TH WIDTH=15%%>TYPE</TH>");
          fprintf(cgiOut,"<TH WIDTH=15%%>LENGTH</TH>");
          fprintf(cgiOut,"<TH WIDTH=15%%>NOT NULL</TH>");
          fprintf(cgiOut,"<TH WIDTH=15%%>KEY</TH></TR>");
          while((curField = msqlFetchField(res2)))
            {
            fprintf(cgiOut,"<TR>\n");
            fprintf(cgiOut,"<TH>%s</TH>\n",curField->name);
            fprintf(cgiOut,"<TD>");
            switch(curField->type)
              {
              case INT_TYPE: fprintf(cgiOut,"int");
                   break;
              case CHAR_TYPE: fprintf(cgiOut,"char");
                   break;
              case REAL_TYPE: fprintf(cgiOut,"real");
                   break;
              default: fprintf(cgiOut,"Unknown");
                   break;
              }
            fprintf(cgiOut,"</TD>\n");
            fprintf(cgiOut,"<TD>%d</TD>\n",curField->length);
              /* fprintf(cgiOut,"<TD>PLACEHOLDER</TD>\n"); */
            fprintf(cgiOut,"<TD>%s</TD>\n", IS_NOT_NULL(curField->flags)? "Y":"N");
            /* fprintf(cgiOut,"<TD>%s</TD>\n", IS_PRI_KEY(curField->flags)? "Y":"N"); */
            fprintf(cgiOut,"</TR>");
            }
          fprintf(cgiOut,"</TABLE><BR>\n");
          fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
          fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
          fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thetable\" VALUE=%s>\n",cur[0]);
          fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=5>\n");
          fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"REMOVE TABLE\">\n");
          fprintf(cgiOut,"</FORM>\n");
          fprintf(cgiOut,"<HR WIDTH=75%%>\n");
          fprintf(cgiOut,"</CENTER>\n");
          }
        }
      msqlFreeResult(res2);
      }
    msqlFreeResult(res);
    }
}

void giveAddTable(void)
{
  fprintf(cgiOut,"<HR WIDTH=90%%><HR WIDTH=80%%>\n");
  fprintf(cgiOut,"<H3>Add table definition with:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=6>\n");
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
  fprintf(cgiOut,"Number of fields = ");
  fprintf(cgiOut,"<INPUT NAME=\"numfields\" SIZE=2>\n");
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"CREATE TABLE...\">\n");
  fprintf(cgiOut,"</FORM>\n");
}


/* ------------------------------- REMOVE TABLE --------------------------- */
/*                                  STATE = 5                               */
void doRemoveTable(void)
{
  giveTitle();
  connectMSQL();
  connectDB();
  connectTable(); 

  #ifdef DEBUG
  fprintf(cgiOut,"\n<BR>Table %s being removed from %s\n",thetable,thedb);
  #endif
  strcpy(qbuf,"DROP TABLE ");
  strcat(qbuf,thetable);
  #ifdef DEBUG
  fprintf(cgiOut,"Query = %s<BR>\n",qbuf);
  #endif
  if (msqlQuery(sock,qbuf) == -1)
    {
    fprintf(cgiOut,"ERROR: unable to drop table %s from %s.<BR>\n",thetable,thedb);
    fprintf(cgiOut,"Invalid query = %s<BR>\n",qbuf);
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }
   else
    { fprintf(cgiOut,"\n<BR>Table %s removed from %s.\n",thetable,thedb); }
    
  showCurrentTables();
  giveAddTable();
  msqlClose(sock);
}


/* -------------------------------- ADD TABLE ----------------------------- */
/*                                  STATE = 6                               */
void doAddTable(void)
{
int x;

  giveTitle();
  connectMSQL();
  connectDB();
 
    cgiError=cgiFormIntegerBounded("numfields",&numfields,1,MAX_FIELDS,0);
    if (cgiError==cgiFormSuccess)
      { 
      fprintf(cgiOut,"<H3>Please enter new table definition:</H3><BR>\n");
      fprintf(cgiOut,"<CENTER>\n");
      fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=7>\n");
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"numfields\" VALUE=%d>\n",numfields);
      fprintf(cgiOut,"<TABLE BORDER=0><TR><TH>Table Name:</TH></TR>\n");
      fprintf(cgiOut,"<TR><TD><INPUT NAME=\"thetable\" VALUE=\"(new_table_name)\" SIZE=25></TD></TR></TABLE>\n");
      fprintf(cgiOut,"<TABLE CELLPADDING=3 BORDER=4>\n");
      fprintf(cgiOut,"<TR>\n");
      fprintf(cgiOut,"<TH WIDTH=15%%>FIELD\n");
      fprintf(cgiOut,"<TH WIDTH=15%%>TYPE\n");
      fprintf(cgiOut,"<TH WIDTH=15%%>LENGTH\n");
      fprintf(cgiOut,"<TH WIDTH=15%%>NOT NULL\n");
      fprintf(cgiOut,"<TH WIDTH=15%%>KEY\n");
      fprintf(cgiOut,"</TR>\n");
      for(x=1; x<=numfields; x++)
        {
        fprintf(cgiOut,"<TR>\n");
        fprintf(cgiOut,"<TD ALIGN=CENTER><INPUT NAME=\"field%d\" VALUE=\"field%d\" SIZE=25></TD>\n",x,x);
        fprintf(cgiOut,"<TD ALIGN=CENTER><SELECT NAME=\"type%d\">\n",x);
            fprintf(cgiOut,"<OPTION>INT\n");
            fprintf(cgiOut,"<OPTION>CHAR\n");
            fprintf(cgiOut,"<OPTION>REAL\n");
            fprintf(cgiOut,"</SELECT>\n");
        fprintf(cgiOut,"</TD>\n");
        fprintf(cgiOut,"<TD ALIGN=CENTER><INPUT NAME=\"lengthStr%d\" VALUE=25 SIZE=5></TD>\n",x);
        fprintf(cgiOut,"<TD ALIGN=CENTER><SELECT NAME=\"notnul%d\">\n",x);
          if (x==1)
            {
            fprintf(cgiOut,"<OPTION>Y\n");
            fprintf(cgiOut,"<OPTION>N\n");
            }
           else
            {
            fprintf(cgiOut,"<OPTION>N\n");
            fprintf(cgiOut,"<OPTION>Y\n");
            }
            fprintf(cgiOut,"</SELECT>\n");
        fprintf(cgiOut,"</TD>\n");
        fprintf(cgiOut,"<TD ALIGN=CENTER><SELECT NAME=\"key%d\">\n",x);
          if (x==1)
            {
            fprintf(cgiOut,"<OPTION>Y\n");
            fprintf(cgiOut,"<OPTION>N\n");
            }
           else
            {
            fprintf(cgiOut,"<OPTION>N\n");
            /* fprintf(cgiOut,"<OPTION>Y\n"); */
            }
            fprintf(cgiOut,"</SELECT>\n");
        fprintf(cgiOut,"</TD>\n");
        fprintf(cgiOut,"</TR>\n");
        }
      fprintf(cgiOut,"</TABLE>\n");
      fprintf(cgiOut,"*Note: Length will be ignored for INTs and REALs.<BR><BR>\n",numfields);
      fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Add this table definition...\">\n");
      fprintf(cgiOut,"</FORM></CENTER>\n");
      }
     else
      {
      fprintf(cgiOut,"ERROR: Invalid number if fields chosen!<BR>\n");
      fprintf(cgiOut,"Please submit an integer x where 0 &lt x &lt %d<BR>\n",MAX_FIELDS);
      msqlClose(sock);
      giveButtonBar();
      exit(1);
      }
  msqlClose(sock);
}


/* -------------------------- ADD TABLE NOW ------------------------------- */
/*                               STATE = 7                                  */
void addTableNow(void)
{
int x;

  giveTitle();
  connectMSQL();
  connectDB();
  connectTable();

  cgiFormInteger("numfields",&numfields,0);
  getFields();
  #ifdef DEBUG
  fprintf(cgiOut,"NUMBER OF FIELDS = %d<BR>\n",numfields);
  fprintf(cgiOut,"CREATE TABLE %s (<BR>\n",thetable);
  for(x=1; x<=numfields; x++)
    {
    fprintf(cgiOut,"%s %s",&field[x-1][0],&type[x-1][0]); 
    if (strcmp(&type[x-1][0],"CHAR")==0) { fprintf(cgiOut,"(%s)",&lengthStr[x-1][0]); }
    if (strcmp(&notnul[x-1][0],"Y")==0) { fprintf(cgiOut," not null"); }
    if (strcmp(&key[x-1][0],"Y")==0) { fprintf(cgiOut," primary key"); }
    if (x!=numfields) { fprintf(cgiOut,",<BR>\n"); }
     else { fprintf(cgiOut," )<BR>\n"); }
    }
  #endif
  strcpy(qbuf,"CREATE TABLE ");
  strcat(qbuf,thetable);
  strcat(qbuf," ( ");
  for(x=1; x<=numfields; x++)
    {
    strcat(qbuf,&field[x-1][0]);
    strcat(qbuf," ");
    strcat(qbuf,&type[x-1][0]);
    if (strcmp(&type[x-1][0],"CHAR")==0)
      {
      strcat(qbuf,"(");
      strcat(qbuf,&lengthStr[x-1][0]);
      strcat(qbuf,")");
      }
    if (strcmp(&notnul[x-1][0],"Y")==0) { strcat(qbuf," not null"); }
    if (strcmp(&key[x-1][0],"Y")==0) { strcat(qbuf," primary key"); }
    if (x!=numfields) { strcat(qbuf,", "); }
     else { strcat(qbuf," )"); }
    }
#ifdef DEBUG
  fprintf(cgiOut,"<BR>query = %s",qbuf);
#endif

  if (msqlQuery(sock,qbuf) == -1)
    {
   fprintf(cgiOut,"ERROR: unable to add table %s to %s.<BR>\n",thetable,thedb);
    fprintf(cgiOut,"Invalid query = %s<BR>\n",qbuf);
    msqlClose(sock);
      giveButtonBar();
    exit(1);
    }
  fprintf(cgiOut,"Table %s added.\n",thetable);
  showCurrentTables();
  giveAddTable();
  msqlClose(sock);
}

void getFields(void)
{
  cgiFormString("field1",&field[0][0],MAX_FIELDNAME_LEN);
  cgiFormString("field2",&field[1][0],MAX_FIELDNAME_LEN);
  cgiFormString("field3",&field[2][0],MAX_FIELDNAME_LEN);
  cgiFormString("field4",&field[3][0],MAX_FIELDNAME_LEN);
  cgiFormString("field5",&field[4][0],MAX_FIELDNAME_LEN);
  cgiFormString("field6",&field[5][0],MAX_FIELDNAME_LEN);
  cgiFormString("field7",&field[6][0],MAX_FIELDNAME_LEN);
  cgiFormString("field8",&field[7][0],MAX_FIELDNAME_LEN);
  cgiFormString("field9",&field[8][0],MAX_FIELDNAME_LEN);
  cgiFormString("field10",&field[9][0],MAX_FIELDNAME_LEN);
  cgiFormString("field11",&field[10][0],MAX_FIELDNAME_LEN);
  cgiFormString("field12",&field[11][0],MAX_FIELDNAME_LEN);
  cgiFormString("field13",&field[12][0],MAX_FIELDNAME_LEN);
  cgiFormString("field14",&field[13][0],MAX_FIELDNAME_LEN);
  cgiFormString("field15",&field[14][0],MAX_FIELDNAME_LEN);
  cgiFormString("field16",&field[15][0],MAX_FIELDNAME_LEN);
  cgiFormString("field17",&field[16][0],MAX_FIELDNAME_LEN);
  cgiFormString("field18",&field[17][0],MAX_FIELDNAME_LEN);
  cgiFormString("field19",&field[18][0],MAX_FIELDNAME_LEN);
  cgiFormString("field20",&field[19][0],MAX_FIELDNAME_LEN);
  cgiFormString("field21",&field[20][0],MAX_FIELDNAME_LEN);
  cgiFormString("field22",&field[21][0],MAX_FIELDNAME_LEN);
  cgiFormString("field23",&field[22][0],MAX_FIELDNAME_LEN);
  cgiFormString("field24",&field[23][0],MAX_FIELDNAME_LEN);
  cgiFormString("field25",&field[24][0],MAX_FIELDNAME_LEN);

  cgiFormString("type1",&type[0][0],5);
  cgiFormString("type2",&type[1][0],5);
  cgiFormString("type3",&type[2][0],5);
  cgiFormString("type4",&type[3][0],5);
  cgiFormString("type5",&type[4][0],5);
  cgiFormString("type6",&type[5][0],5);
  cgiFormString("type7",&type[6][0],5);
  cgiFormString("type8",&type[7][0],5);
  cgiFormString("type9",&type[8][0],5);
  cgiFormString("type10",&type[9][0],5);
  cgiFormString("type11",&type[10][0],5);
  cgiFormString("type12",&type[11][0],5);
  cgiFormString("type13",&type[12][0],5);
  cgiFormString("type14",&type[13][0],5);
  cgiFormString("type15",&type[14][0],5);
  cgiFormString("type16",&type[15][0],5);
  cgiFormString("type17",&type[16][0],5);
  cgiFormString("type18",&type[17][0],5);
  cgiFormString("type19",&type[18][0],5);
  cgiFormString("type20",&type[19][0],5);
  cgiFormString("type21",&type[20][0],5);
  cgiFormString("type22",&type[21][0],5);
  cgiFormString("type23",&type[22][0],5);
  cgiFormString("type24",&type[23][0],5);
  cgiFormString("type25",&type[24][0],5);

  cgiFormString("lengthStr1",&lengthStr[0][0],0);
  cgiFormString("lengthStr2",&lengthStr[1][0],0);
  cgiFormString("lengthStr3",&lengthStr[2][0],0);
  cgiFormString("lengthStr4",&lengthStr[3][0],0);
  cgiFormString("lengthStr5",&lengthStr[4][0],0);
  cgiFormString("lengthStr6",&lengthStr[5][0],0);
  cgiFormString("lengthStr7",&lengthStr[6][0],0);
  cgiFormString("lengthStr8",&lengthStr[7][0],0);
  cgiFormString("lengthStr9",&lengthStr[8][0],0);
  cgiFormString("lengthStr10",&lengthStr[9][0],0);
  cgiFormString("lengthStr11",&lengthStr[10][0],0);
  cgiFormString("lengthStr12",&lengthStr[11][0],0);
  cgiFormString("lengthStr13",&lengthStr[12][0],0);
  cgiFormString("lengthStr14",&lengthStr[13][0],0);
  cgiFormString("lengthStr15",&lengthStr[14][0],0);
  cgiFormString("lengthStr16",&lengthStr[15][0],0);
  cgiFormString("lengthStr17",&lengthStr[16][0],0);
  cgiFormString("lengthStr18",&lengthStr[17][0],0);
  cgiFormString("lengthStr19",&lengthStr[18][0],0);
  cgiFormString("lengthStr20",&lengthStr[19][0],0);
  cgiFormString("lengthStr21",&lengthStr[20][0],0);
  cgiFormString("lengthStr22",&lengthStr[21][0],0);
  cgiFormString("lengthStr23",&lengthStr[22][0],0);
  cgiFormString("lengthStr24",&lengthStr[23][0],0);
  cgiFormString("lengthStr25",&lengthStr[24][0],0);

  cgiFormString("notnul1",&notnul[0][0],2);
  cgiFormString("notnul2",&notnul[1][0],2);
  cgiFormString("notnul3",&notnul[2][0],2);
  cgiFormString("notnul4",&notnul[3][0],2);
  cgiFormString("notnul5",&notnul[4][0],2);
  cgiFormString("notnul6",&notnul[5][0],2);
  cgiFormString("notnul7",&notnul[6][0],2);
  cgiFormString("notnul8",&notnul[7][0],2);
  cgiFormString("notnul9",&notnul[8][0],2);
  cgiFormString("notnul10",&notnul[9][0],2);
  cgiFormString("notnul11",&notnul[10][0],2);
  cgiFormString("notnul12",&notnul[11][0],2);
  cgiFormString("notnul13",&notnul[12][0],2);
  cgiFormString("notnul14",&notnul[13][0],2);
  cgiFormString("notnul15",&notnul[14][0],2);
  cgiFormString("notnul16",&notnul[15][0],2);
  cgiFormString("notnul17",&notnul[16][0],2);
  cgiFormString("notnul18",&notnul[17][0],2);
  cgiFormString("notnul19",&notnul[18][0],2);
  cgiFormString("notnul20",&notnul[19][0],2);
  cgiFormString("notnul21",&notnul[20][0],2);
  cgiFormString("notnul22",&notnul[21][0],2);
  cgiFormString("notnul23",&notnul[22][0],2);
  cgiFormString("notnul24",&notnul[23][0],2);
  cgiFormString("notnul25",&notnul[24][0],2);

  cgiFormString("key1",&key[0][0],2);
  cgiFormString("key2",&key[1][0],2);
  cgiFormString("key3",&key[2][0],2);
  cgiFormString("key4",&key[3][0],2);
  cgiFormString("key5",&key[4][0],2);
  cgiFormString("key6",&key[5][0],2);
  cgiFormString("key7",&key[6][0],2);
  cgiFormString("key8",&key[7][0],2);
  cgiFormString("key9",&key[8][0],2);
  cgiFormString("key10",&key[9][0],2);
  cgiFormString("key11",&key[10][0],2);
  cgiFormString("key12",&key[11][0],2);
  cgiFormString("key13",&key[12][0],2);
  cgiFormString("key14",&key[13][0],2);
  cgiFormString("key15",&key[14][0],2);
  cgiFormString("key16",&key[15][0],2);
  cgiFormString("key17",&key[16][0],2);
  cgiFormString("key18",&key[17][0],2);
  cgiFormString("key19",&key[18][0],2);
  cgiFormString("key20",&key[19][0],2);
  cgiFormString("key21",&key[20][0],2);
  cgiFormString("key22",&key[21][0],2);
  cgiFormString("key23",&key[22][0],2);
  cgiFormString("key24",&key[23][0],2);
  cgiFormString("key25",&key[24][0],2);
}


/* -------------------------- CHOOSE TABLE PAGE --------------------------- */
/*                               STATE = 8                                 */

void doTableSelection(void)
{
  giveTitle();
  connectMSQL();
  connectDB();
  giveSearch(); 
  giveInsert();
  giveTextQuery();
  msqlClose(sock);
}

void giveSearch(void)
{
  fprintf(cgiOut,"<H3>Search a table (update/delete)...</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=12>\n");
  showTables();
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Perform Search...\">\n");
  fprintf(cgiOut,"</FORM><HR>");
}

void giveInsert(void)
{
  fprintf(cgiOut,"<H3>Add data to a table...</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=state VALUE=10>\n");
  showTables();
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Begin Insert...\">\n");
  fprintf(cgiOut,"</FORM><HR>");
}

void showTables(void)
{
int x=0;
  res = msqlListTables(sock);
  if (!res)
    {
   fprintf(cgiOut,"\nERROR : Unable to list tables in database %s<BR>\n",thedb);
    msqlClose(sock);
      giveButtonBar();
    exit(1);
    }
  while(cur = msqlFetchRow(res))
    { 
    x++;
    fprintf(cgiOut,"<DD><INPUT TYPE=RADIO NAME=\"thetable\" VALUE=\"%s\" ",cur[0]);
    if (x==1) { fprintf(cgiOut," CHECKED"); }
    fprintf(cgiOut,"> %s\n<BR>",cur[0]);
    }
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
/*  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"New Record... \">\n"); */
/*  fprintf(cgiOut,"</FORM><HR>\n"); */
}

void giveCannedSelect(void)
{
  fprintf(cgiOut,"<H3>Select a query:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=xx>\n");
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
  fprintf(cgiOut,"SELECT...<BR>\n");
  fprintf(cgiOut,"<DD><INPUT TYPE=CHECKBOX NAME=\"columns\"> * <BR>\n");
  fprintf(cgiOut,"FROM...<BR>\n");
  res = msqlListTables(sock);
  while((cur = msqlFetchRow(res)))
    { fprintf(cgiOut,"<DD><INPUT TYPE=CHECKBOX NAME=\"thetable\" VALUE=\"%s\"> %s\n<BR>",cur[0],cur[0]); }
  fprintf(cgiOut,"</FORM>\n");
  fprintf(cgiOut,"<HR>\n");
}

void giveTextQuery(void)
{
  fprintf(cgiOut,"<H3>Enter a mSQL query by hand if you prefer:</H3>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=9>\n");
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
  fprintf(cgiOut,"<TEXTAREA COLS=60 ROWS=10 NAME=\"textquery\">\n");
  fprintf(cgiOut,"SELECT * FROM (table_name)\n");
  fprintf(cgiOut,"</TEXTAREA>\n");
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Submit mSQL query...\">\n");
  fprintf(cgiOut,"</FORM>\n");
}


/* --------------------------- DO TEXT QUERY ----------------------------- */
/*                               STATE = 9                                 */

void doTextQuery(void)
{
int  query_len;
char *thetextquery;
char *command;

/* xxx */

  giveTitle();
  connectMSQL();
  connectDB();

  cgiFormStringSpaceNeeded("textquery",&query_len);
  thetextquery=(char *)malloc(sizeof(char)*query_len);
  cgiFormString("textquery",thetextquery,query_len);

  if (msqlQuery(sock,thetextquery) == -1)
    {
    fprintf(cgiOut,"ERROR: unable to perform query on database %s.<BR>\n",thedb);
    fprintf(cgiOut,"Invalid query = %s<BR>\n",thetextquery);
    msqlClose(sock);
    giveBackQueryPage();
    giveButtonBar();
    exit(1);
    }

  command = strtok(thetextquery," ");
  if ((strcmp(command,"SELECT")==0) || (strcmp(command,"select")==0))
    { 
    fprintf(cgiOut,"<CENTER>The query \"%s\" returned the following values:\n",thetextquery);
    showSelectData();
    }
  fprintf(cgiOut,"<BR>Query request completed.<BR>\n");
  giveBackQueryPage(); 
  free(thetextquery);
  msqlClose(sock);
}

void showSelectData(void)
{
int cols,rows,x,y;

  res=msqlStoreResult();
  fprintf(cgiOut,"<TABLE BORDER=4 CELLPADDING=2>\n");
  rows=msqlNumRows(res);
  cols=msqlNumFields(res);
  for (x=0; x<rows; x++)
    { 
    fprintf(cgiOut,"<TR>\n");
    cur=msqlFetchRow(res);
    for (y=0; y<cols; y++)
      {
      fprintf(cgiOut,"<TD NOWRAP>\n");
      fprintf(cgiOut,"%s\n",cur[y]);
      fprintf(cgiOut,"</TD>\n");
      }
    fprintf(cgiOut,"</TR>\n");
    }
  fprintf(cgiOut,"</TABLE>\n");
  msqlFreeResult(res);
}


/* -------------------------- CANNED QUERY PAGE -------------------------- */
/*                               STATE = 10                                */

void doCannedQuery(void)
{
int size=1;

  giveTitle();
  connectMSQL();
  connectDB();
  connectTable();

  giveInsertForm();
  msqlClose(sock);
}

void giveInsertForm(void)
{
int size=1;
  fieldnames = (char *)malloc(1*sizeof(char));
  strcpy(fieldnames,"");
  res=msqlListFields(sock,thetable);
  if (!res)
    {
    fprintf(cgiOut,"ERROR : Couldn't find %s in %s!<BR>\n",thetable,thedb);
    msqlFreeResult(res);
      giveButtonBar();
    exit(1);
    }
   else
    {
    fprintf(cgiOut,"<H3>Add new data to table %s:</H3>\n",thetable);
    fprintf(cgiOut,"<CENTER>\n");
    fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=11>\n");
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thetable\" VALUE=\"%s\">\n",thetable);
    fprintf(cgiOut,"<TABLE WIDTH=75%% BORDER=2 CELLPADDING=4>\n");
    while((curField = msqlFetchField(res)))
      {
      size = size + strlen(curField->name) + 2;
      fieldnames= (char *)realloc((void *)fieldnames,size);
      strcat(fieldnames,curField->name);
      strcat(fieldnames,",");
      fprintf(cgiOut,"<TR><TH>%s</TH>\n",curField->name);
      fprintf(cgiOut,"<TD>"); 
      if (curField->type==INT_TYPE) { fprintf(cgiOut,"INT"); }
      if (curField->type==CHAR_TYPE) { fprintf(cgiOut,"CHAR[%d]",curField->length);}
      if (curField->type==REAL_TYPE) { fprintf(cgiOut,"REAL"); }
      fprintf(cgiOut,"%s", IS_NOT_NULL(curField->flags)? "(NOT NULL)":"");
      fprintf(cgiOut,"</TD>");
      fprintf(cgiOut,"<TD ALIGN=CENTER><INPUT TYPE=\"TEXT\" NAME=\"%s\" SIZE=25></TD></TR>\n",curField->name);
      }
    fprintf(cgiOut,"</TABLE>\n");
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"fieldnames\" VALUE=\"%s\">\n",fieldnames); 
    fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"Add entry to table...\">\n");
    fprintf(cgiOut,"</FORM>\n");
    fprintf(cgiOut,"</CENTER>\n");
    free(fieldnames);
#ifdef DEBUG
    fprintf(cgiOut,"SIZE=%d\n"),size;
#endif
    }
  giveBackQueryPage(); 
}

void giveBackQueryPage(void)
{
  fprintf(cgiOut,"<CENTER><HR>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=8>\n");
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"RETURN TO MAIN QUERY PAGE\">\n");
  fprintf(cgiOut,"</FORM>\n");
  fprintf(cgiOut,"<HR></CENTER>\n");
}

/* -------------------------- ADD CANNED QUERY  -------------------------- */
/*                               STATE = 11                                */

void addCannedQuery(void)
{
int count=0;
int x,lengthstr;
char *the_data;

  giveTitle();

  connectMSQL();
  connectDB();
  connectTable();

  res=msqlListFields(sock,thetable);
  if (res == NULL)
    {
    fprintf(cgiOut,"ERROR : Couldn't find %s in %s!<BR>\n",thetable,thedb);
    msqlFreeResult(res);
      giveButtonBar();
    exit(1);
    }
  strcpy(qbuf,"INSERT INTO ");
  strcat(qbuf,thetable);
  strcat(qbuf," ( ");
  while((curField = msqlFetchField(res)))
    {
    if (count>0) {strcat(qbuf,", ");}
    count++;
    strcat(qbuf,curField->name);
    }
  strcat(qbuf," ) VALUES ( ");

  msqlFreeResult(res);
  res=msqlListFields(sock,thetable);
  count=0;
  while(curField = msqlFetchField(res))
    {
    cgiFormStringSpaceNeeded(curField->name,&lengthstr);
    the_data = (char *)malloc(sizeof(char)*lengthstr);
    cgiFormString(curField->name,the_data,lengthstr);

    if (count>0) {strcat(qbuf,", ");}
    count++;
    switch (curField->type)
      {
      case INT_TYPE:
      case REAL_TYPE:
                     if (lengthstr > 1)
                       { strcat(qbuf,the_data); }
                      else
                       { strcat(qbuf,"NULL"); }
                     break;
      case CHAR_TYPE:
                     if (lengthstr > 1)
                       { 
                       strcat(qbuf,"'");
                       strcat(qbuf,the_data);
                       strcat(qbuf,"'");
                       }
                      else                       
                       { strcat(qbuf,"NULL"); }
                     break;
      default:
                     fprintf(cgiOut,"Error: Unknown mSQL field type");
                     giveButtonBar();
                     exit(1);
                     break;
      }
    free(the_data);
    }
  strcat(qbuf,")");
  #ifdef DEBUG
  fprintf(cgiOut,"Query = %s<BR>\n",qbuf);
  #endif
 
  
  if (msqlQuery(sock,qbuf) == -1)
    {
   fprintf(cgiOut,"ERROR: unable to perform query on database %s.<BR>\n",thedb);
    fprintf(cgiOut,"Invalid query = %s<BR>\n",qbuf);
    giveInsertForm();
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }

  msqlFreeResult(res);
  fprintf(cgiOut,"Insert completed.\n"); 

   giveInsertForm(); 

   msqlClose(sock); 
} 


/* --------------------------- SEARCH RECORD ----------------------------- */
/*                               STATE = 12                                */

void searchRecords(void)
{
  giveTitle();
  connectMSQL();
  connectDB();
  connectTable();

  giveSearchForm();
  giveBackQueryPage();
  msqlClose(sock);
}

void giveSearchForm(void)
{
  fprintf(cgiOut,"<H3>Search of table %s:</H3>\n",thetable);
  fprintf(cgiOut,"<CENTER>\n");
  fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thetable\" VALUE=%s>\n",thetable);
  fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=14>\n");
  fprintf(cgiOut,"<TABLE WIDTH=75%% BORDER=2 CELLPADDING=4>\n");
  res = msqlListFields(sock,thetable);
  while (curField = msqlFetchField(res))
    {
    fprintf(cgiOut,"<TR>");
    fprintf(cgiOut,"<TH ALIGN=CENTER>%s</TH>",curField->name);
    fprintf(cgiOut,"<TD>");
      if (curField->type==INT_TYPE) { fprintf(cgiOut,"INT"); }
      if (curField->type==CHAR_TYPE) { fprintf(cgiOut,"CHAR[%d]",curField->length);}
      if (curField->type==REAL_TYPE) { fprintf(cgiOut,"REAL"); }
    fprintf(cgiOut,"</TD>\n");
    fprintf(cgiOut,"<TD><SELECT NAME=\"%s_operation\"><OPTION>LIKE\n",curField->name);
    fprintf(cgiOut,"<OPTION>&lt\n");
    fprintf(cgiOut,"<OPTION>=\n");
    fprintf(cgiOut,"<OPTION>&gt\n");
    fprintf(cgiOut,"</SELECT></TD>\n");
    fprintf(cgiOut,"<TD ALIGN=CENTER><INPUT NAME=\"%s\" SIZE=25></TD></TR>\n",curField->name);
    }
  fprintf(cgiOut,"</TABLE>\n");
  fprintf(cgiOut,"ORDER BY:\n");
  fprintf(cgiOut,"<SELECT NAME=\"order\">\n");
  msqlFreeResult(res);
  res = msqlListFields(sock,thetable);
  while (curField = msqlFetchField(res))
    {
    fprintf(cgiOut,"<OPTION> %s\n",curField->name);
    }
  fprintf(cgiOut,"</SELECT>\n");
  fprintf(cgiOut,"OPTION FOR:\n");
  fprintf(cgiOut,"<SELECT NAME=\"action\"><OPTION> UPDATE\n<OPTION> DELETE\n</SELECT>\n");
  fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"PERFORM SEARCH...\">\n");
  fprintf(cgiOut,"</FORM></CENTER>\n");
  msqlFreeResult(res);
}


/* --------------------------- RETURN SEARCH  ----------------------------- */
/*                               STATE = 14                                */

void returnSearch(void)
{
  giveTitle();
  connectMSQL();
  connectDB();
  connectTable();
  getSearchSpec();
  doSearchQuery();
  giveSearchForm();
  giveBackQueryPage();
  msqlClose(sock);
}

void getSearchSpec(void)
{
int count;
int  op_temp_len;
char *op_temp; 		/* (field_name)_operation\0 */
int  operation_len;
char *operation;
int  pattern_len;
char *pattern;
int  order_len;
char *order;
int  action_len;
char *action;

  count=0;
  pattern_len=0;
  strcpy(qbuf,"SELECT * FROM ");
  strcat(qbuf,thetable);
  strcat(qbuf," WHERE ");
  res = msqlListFields(sock,thetable);
  while (curField = msqlFetchField(res))
    {
    op_temp_len = strlen(curField->name) + 11; 
    op_temp = (char *)malloc(sizeof(char)*op_temp_len);
    strcpy(op_temp,curField->name);
    strcat(op_temp,"_operation");

    cgiError=cgiFormStringSpaceNeeded(op_temp,&operation_len);
    operation=(char *)malloc(sizeof(char)*operation_len);
    cgiError=cgiFormString(op_temp,operation,operation_len);

    cgiError=cgiFormStringSpaceNeeded(curField->name,&pattern_len);
    if (pattern_len > 1) 
      {
      count++;
      pattern=(char *)malloc(sizeof(char)*pattern_len);
      cgiFormString(curField->name,pattern,pattern_len);
      if (count>1) { strcat(qbuf," AND "); }
      strcat(qbuf,curField->name);
      strcat(qbuf," ");
      strcat(qbuf,operation);
      strcat(qbuf," ");
      if (curField->type == CHAR_TYPE) { strcat(qbuf,"'"); }
      strcat(qbuf,pattern);
      if (curField->type == CHAR_TYPE) { strcat(qbuf,"'"); }
      free(pattern);
      }
    free(operation);
    free(op_temp);
    }
  cgiError=cgiFormStringSpaceNeeded("order",&order_len);
  order=(char *)malloc(sizeof(char)*order_len);
  cgiError=cgiFormString("order",order,order_len);
  strcat(qbuf," ORDER BY ");
  strcat(qbuf,order);
  free(order);
}

void doSearchQuery(void)
{
#ifdef DEBUG
  fprintf(cgiOut,"Query = %s\n",qbuf);
#endif
  if (msqlQuery(sock,qbuf) == -1)
    {
    fprintf(cgiOut,"ERROR: unable to perform query on database %s.<BR>\n",thedb);
    fprintf(cgiOut,"Invalid query = %s<BR>\n",qbuf);
    msqlClose(sock);
    giveButtonBar();
    exit(1);
    }
  showSelectDeleteData(); 
}

void showSelectDeleteData(void)
{
int cols,rows,x,y,z;
int  action_len;
char *action;
int  rec_id_len;
char *rec_id;

  cgiError=cgiFormStringSpaceNeeded("action",&action_len);
  action=(char *)malloc(sizeof(char)*action_len);
  cgiError=cgiFormString("action",action,action_len);

  res=msqlStoreResult();
  fprintf(cgiOut,"<CENTER>\n");
  rows=msqlNumRows(res);
  cols=msqlNumFields(res);
  fprintf(cgiOut,"%d rows were returned by your search:<BR>\n",rows);
  z=0;
  for (x=0; x<rows; x++)
    {
    cur=msqlFetchRow(res);
    res2 = msqlListFields(sock,thetable);
    rec_id = (char *)malloc(sizeof(char));
    strcpy(rec_id,"");
    rec_id_len = 1;
    z=0;
    while (curField = msqlFetchField(res2))
      {
      if (z > 0)
        {
        rec_id_len += 5;
        rec_id=(char *)realloc(rec_id,rec_id_len);
        strcat(rec_id," AND ");
        }
      rec_id_len += strlen(curField->name);
      rec_id_len += 3;
      rec_id=(char *)realloc(rec_id,rec_id_len);
      strcat(rec_id,curField->name);
      strcat(rec_id," = ");
      if (curField->type==CHAR_TYPE)
        {
        if (cur[z] != NULL)
          {
          rec_id_len += 1;
          rec_id=(char *)realloc(rec_id,rec_id_len);
          strcat(rec_id,"'");
          }
        }
/* HERE */
      if (cur[z] == NULL)
        {
        rec_id_len += 4;
        rec_id=(char *)realloc(rec_id,rec_id_len);
        strcat(rec_id,"NULL");
        }
       else
        {
      rec_id_len += strlen(cur[z]);
      rec_id=(char *)realloc(rec_id,rec_id_len);
      strcat(rec_id,cur[z]);
        }
      if (curField->type==CHAR_TYPE)
        {
        if (cur[z] != NULL)
          {
          rec_id_len += 1;
          rec_id=(char *)realloc(rec_id,rec_id_len);
          strcat(rec_id,"'");
          }
        }
      z++;
      }
    fprintf(cgiOut,"<FORM METHOD=POST ACTION=%s/dbadmin>\n",RELPATH);
    if (strcmp(action,"DELETE")==0)
      {
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=15>\n"); 
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"action\" VALUE=\"DELETE\">\n");
      }
    if (strcmp(action,"UPDATE")==0)
      {
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"state\" VALUE=16>\n"); 
      fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"action\" VALUE=\"UPDATE\">\n");
      }
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thedb\" VALUE=\"%s\">\n",thedb);
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"thetable\" VALUE=\"%s\">\n",thetable);
    res2 = msqlListFields(sock,thetable);
    y=0;
    while (curField = msqlFetchField(res2))
      {
      fprintf(cgiOut,"<INPUT name=%s LENGTH=25 VALUE=\"%s\"> \n",curField->name,cur[y]);
      y++;
      }
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"record_query\" VALUE=\"%s\">\n",rec_id);
    fprintf(cgiOut,"<INPUT TYPE=HIDDEN NAME=\"total_query\" VALUE=\"%s\">\n",qbuf);
    if (strcmp(action,"DELETE")==0)
      { fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"DELETE RECORD\">\n"); }
    if (strcmp(action,"UPDATE")==0)
      { fprintf(cgiOut,"<INPUT TYPE=SUBMIT VALUE=\"UPDATE RECORD\">\n"); }
    fprintf(cgiOut,"</FORM>\n");
    /* fprintf(cgiOut,"<BR>\n"); */
    }
/*
  fprintf(cgiOut,"</CENTER><HR>\n");
  if (z>0) { free(rec_id); } 
  free(action);
  msqlFreeResult(res); 
*/
}


/* ------------------------- DELETE SEARCH ITEM  ------------------------- */
/*                               STATE = 15                                */

void deleteSearchItem(void)
{
  giveTitle();
  connectMSQL();
  connectDB();
  connectTable();
  performDelete(); 
  giveBackOriginalQuery();
  giveBackQueryPage();
  msqlClose(sock);
}

void performDelete(void)
{
int rec_id_len;
char *rec_id;
int  query_len;
char *query;

  query_len = 13;
  query=(char *)malloc(sizeof(int)*query_len);
  strcpy(query,"DELETE FROM ");
  query_len += strlen(thetable);
  query=(char *)realloc(query,query_len);
  strcat(query,thetable);
  query_len += 7;
  query=(char *)realloc(query,query_len);
  strcat(query," WHERE ");
  cgiFormStringSpaceNeeded("record_query",&rec_id_len);
  rec_id=(char *)malloc(sizeof(char)*rec_id_len);
  cgiFormString("record_query",rec_id,rec_id_len);
  query_len += rec_id_len;
  query=(char *)realloc(query,query_len);
  strcat(query,rec_id);
  #ifdef DEBUG
  fprintf(cgiOut,"<BR>QUERY=%s\n<BR>\n",query);
  #endif
  msqlQuery(sock,query);
  free(query);
  free(rec_id);
}


/* ------------------------- UPDATE SEARCH ITEM  ------------------------- */
/*                               STATE = 16                                */

void updateSearchItem(void)
{
  giveTitle();
  connectMSQL();
  connectDB();
  connectTable();
  performUpdate(); 
  giveBackOriginalQuery();
  giveBackQueryPage();
  msqlClose(sock);
}

void performUpdate(void)
{
int rec_id_len;
char *rec_id;
int  query_len;
char *query;
int rec_new_len;
char *rec_new;
int z;
char *field_value;
int  field_value_len;


  query_len = 8;
  query=(char *)malloc(sizeof(int)*query_len);
  strcpy(query,"UPDATE ");
  query_len += strlen(thetable);
  query=(char *)realloc(query,query_len);
  strcat(query,thetable);
  query_len += 5;
  query=(char *)realloc(query,query_len);
  strcat(query," SET ");

    res2 = msqlListFields(sock,thetable);
    rec_new = (char *)malloc(sizeof(char));
    strcpy(rec_new,"");
    rec_new_len = 1;
    z=0;
    while (curField = msqlFetchField(res2))
      {
      if (z > 0)
        {
        rec_new_len += 2;
        rec_new=(char *)realloc(rec_new,rec_new_len);
        strcat(rec_new,", ");
        }
      rec_new_len += strlen(curField->name);
      rec_new_len += 3;
      rec_new=(char *)realloc(rec_new,rec_new_len);
      strcat(rec_new,curField->name);
      strcat(rec_new," = ");

      cgiFormStringSpaceNeeded(curField->name,&field_value_len);
      field_value=(char *)malloc(sizeof(char)*field_value_len);
      cgiFormString(curField->name,field_value,field_value_len);
      
      if (strcmp(field_value,"(null)")==0) 
        {
        rec_new_len += 4;
        rec_new=(char *)realloc(rec_new,rec_new_len);
        strcat(rec_new,"NULL");
        }
       else
        {      
        

        if (curField->type==CHAR_TYPE)
          {
          rec_new_len += 1;
          rec_new=(char *)realloc(rec_new,rec_new_len);
          strcat(rec_new,"'");
          }

/*
      cgiFormStringSpaceNeeded(curField->name,&field_value_len);
      field_value=(char *)malloc(sizeof(char)*field_value_len);
      cgiFormString(curField->name,field_value,field_value_len);
*/ 


        rec_new_len += field_value_len;
        rec_new=(char *)realloc(rec_new,rec_new_len);
        strcat(rec_new,field_value);
        free(field_value);
      
        if (curField->type==CHAR_TYPE)
          {
          rec_new_len += 1;
          rec_new=(char *)realloc(rec_new,rec_new_len);
          strcat(rec_new,"'");
          }
        }
      z++;
      }
  query_len += strlen(rec_new);
  query=(char *)realloc(query,query_len);
  strcat(query,rec_new);

  query_len += 7;
  query=(char *)realloc(query,query_len);
  strcat(query," WHERE ");
  cgiFormStringSpaceNeeded("record_query",&rec_id_len);
  rec_id=(char *)malloc(sizeof(char)*rec_id_len);
  cgiFormString("record_query",rec_id,rec_id_len);
  query_len += rec_id_len;
  query=(char *)realloc(query,query_len);
  strcat(query,rec_id);
  #ifdef DEBUG
  fprintf(cgiOut,"<BR>QUERY=%s\n<BR>\n",query);
  #endif
  msqlQuery(sock,query);
  free(query);
  free(rec_id);
  free(rec_new);
}
 
void giveBackOriginalQuery(void)
{
  cgiFormString("total_query",qbuf,MAX_TEXTQUERY_LEN);
  doSearchQuery();
  giveSearchForm();
}
