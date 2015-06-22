
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

   

bool schema_table_store_record(THD *thd, TABLE *table); 
//Declairing & Defining the columns of the Innodb_CPU_INFO virtual table
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
int mysql_cpu_info_fill_table( THD *thd, TABLE_LIST *tables, Item *cond)
 {
  int status = 0;
 TABLE *table= (TABLE *)tables->table;
 const CHARSET_INFO *scs = system_charset_info;
  
	    FILE *in;
	    int n = 0;
	    int m = 0;
	    //extern FILE *popen();
	    char buff[512];
	 
	    if(!(in = popen("cat /proc/stat", "r"))){
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
//Initialization(init) function for the CpuDesc.so plugin
static int mysql_cpu_info_plugin_init(void *p)
{
  ST_SCHEMA_TABLE * schema= (ST_SCHEMA_TABLE *) p;

  schema->fields_info= mysql_cpu_info_field_info;
  schema->fill_table= mysql_cpu_info_fill_table;

  return 0;
}
//Deinitialization(deinit) function for the CpuDesc.so plugin
static int mysql_cpu_info_plugin_deinit(void *p)
{
  return 0;
}
//Descriptor structure for the telling the server about plugin details
struct st_mysql_information_schema mysql_cpu_info_plugin =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };
//Descriptor structure for the telling the server about plugin details
mysql_declare_plugin(mysql_cpu_info)//Start macro  
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,//Plugin Type        
  &mysql_cpu_info_plugin,                  
  "CPU_INFO",//Plugin_Name                                   
  "SALIL JAIN ",//Author Name
  "Monitor CPU Status at Server",//Plugin Description                                
  PLUGIN_LICENSE_GPL,//Licensed Under                         
  mysql_cpu_info_plugin_init,   //Initialization function name                   
  mysql_cpu_info_plugin_deinit, //Initialization function name                 
  0x0010,                                         
  NULL,   //System Variable                                         
  NULL,  //System Variable                                          
  NULL   //System Variable                                         
}
mysql_declare_plugin_end;




