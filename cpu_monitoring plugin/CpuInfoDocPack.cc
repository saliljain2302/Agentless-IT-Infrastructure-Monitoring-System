
#include <mysqld_error.h>
#include <sql_acl.h>
#include <m_ctype.h>
#include <hash.h>
#include <myisampack.h>
#include <mysys_err.h>
#include <my_sys.h>
#include <sql_plugin.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <math.h>


using namespace std; 
//isPrefix() is function to check wheather a string is prefix of another string
bool isPrefix(string &s1,string &s2)
{
    const char*p = s1.c_str();
    const char*q = s2.c_str();
    while (*p&&*q)
        if (*p++!=*q++)
            return false;
    return true;
}

   
//Declaring store record function  defined in mysql-5.1.22-rc/sql/sql_show.cc.
bool schema_table_store_record(THD *thd, TABLE *table); 
//Declairing & Defining the columns of the Innodb_CPU_INFO virtual table
/*typedef struct st_field_info
{
  const char* field_name;
  uint field_length;
  enum enum_field_types field_type;
  int value; //not applicable
  uint field_flags;        // by default columns are not null & signed
  const char* old_name;  //don't have use
  uint open_method;  // define the interaction of server with the table
} ST_FIELD_INFO*/

//ST_FIELD_INFO declared in table.h
static ST_FIELD_INFO mysql_cpu_info_field_info[]=
{
  {"CPU_NAME", 50, MYSQL_TYPE_STRING, 0, 0, "Cpu_Name", 0},//Name of cpu as per the system
  {"# USER_PROCESSES ", 7, MYSQL_TYPE_FLOAT, 0, 0, "USER ", 0},//Number of user process
  {"# SYSTEM_PROCESSES", 7, MYSQL_TYPE_FLOAT, 0, 0, "SYSTEM", 0},//Number of System process
  {"# IDLE_PROCESSES", 120, MYSQL_TYPE_FLOAT, 0, 0, "IDLE", 0},//Number of Idle process
  {"# IOWAIT_PROCESSES", 120, MYSQL_TYPE_FLOAT, 0, 0, "IOWAIT" , 0},//Number of IOWAIT process
 
  {0, 0, MYSQL_TYPE_STRING, 0, 0, 0, 0}//Null Element shows the end of the array. 
};




//Function mysql_cpu_info_fill_table fills the data into the virtual table at runtime.
//server passes a number of arguments to the fill_table function
int mysql_cpu_info_fill_table( THD *thd, TABLE_LIST *tables, Item *cond)
 {
//THD *thd - this is a pointer to an instance of the thread descriptor class. In practice, this is a direct handle to the current session
//TABLE_LIST *tables -corresponds to the runtime representation of the table we are implementing as it appears in a query
//COND - This is used to pass the instance of the Item class that holds the internal representation of the WHERE-clause
  int status = 0;
 TABLE *table= (TABLE *)tables->table;
 const CHARSET_INFO *scs = system_charset_info;
  
	    FILE *in;
	    int n = 0;
	    int m = 0;
	    //extern FILE *popen();
	    char buff[512];
	 
	    if(!(in = popen("cat /proc/stat", "r"))){//file read open a file desc & read remove popen
	        exit(1);
	    }
	    while(fgets(buff, sizeof(buff), in)!=NULL){//*3while1
		
		 istringstream iss(buff);
		char sub[128];
		 string t2;
			//cout<<"outer while" <<endl;
		iss>>sub;
		t2=string(sub);
		string t1="cpu";
		if(isPrefix(t1,t2))	
		//if(1)

		{	
			table->field[n]->store(sub, strlen(sub), scs);
			m++;
			n++;
		    while(iss>>sub)
		    {//*5while2
			double d1;
			if(m==1||m==3||m==4||m==5){
  			d1 = strtod (sub, NULL);
		        table->field[n]->store(d1);
			n++;
			}
			else
			{}
			m++;
			 if (n==5)
			{
			  n=0;
			status = schema_table_store_record(thd,table);	
			}
			
		} 
		 m=0;
				
  		
		}
	
		else	
		{}

	    }

	    pclose(in);
	 
	
   //status=1;
  return status;
	 
	}
//Initialization(init declared in table.h) function for the CpuDesc.so plugin
//When called, the server passes a pointer to an instance of a ST_SCHEMA_TABLE
//type-specific part of the plug-in API for information schema plug-ins is formed by the struct called ST_SCHEMA_TABLE
static int mysql_cpu_info_plugin_init(void *p)
{
  ST_SCHEMA_TABLE * schema= (ST_SCHEMA_TABLE *) p;


  schema->fields_info= mysql_cpu_info_field_info;
//fields_info - an array of ST_FIELD_INFO structure.

  schema->fill_table= mysql_cpu_info_fill_table;
//fill_table - a pointer to a function that is called whenever the server wants to obtain rows from the information_schema table

  return 0;
}

//Deinitialization(deinit) function for the CpuDesc.so plugin
static int mysql_cpu_info_plugin_deinit(void *p)
{
  return 0;
}


//interface type descriptor
struct st_mysql_information_schema mysql_cpu_info_plugin =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };


//to list the plug-in in the information_schema.PLUGINS
//Descriptor structure for the telling the server about plugin details
mysql_declare_plugin(mysql_cpu_info)//Start macro  
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,//Plugin Type        
  &mysql_cpu_info_plugin,//pointer to type-specific plugin descriptor                
  "INNODB_CPU_INFO",//Plugin_Name/virtual table name associated with plugin                                   
  "SALIL JAIN ",//Author Name
  "Monitor CPU Status at Server",//Plugin Description                                
  PLUGIN_LICENSE_GPL,//Licensed Under                         
  mysql_cpu_info_plugin_init,   //Initialization function name                   
  mysql_cpu_info_plugin_deinit, //Initialization function name                 
  0x0000,                                         
  NULL,   //System Variable                                         
  NULL,  //System Variable                                          
  NULL   //System Variable(reserved for dependency checking)                                        
}
mysql_declare_plugin_end;//end macro




