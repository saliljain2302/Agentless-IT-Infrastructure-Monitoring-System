
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


   

bool schema_table_store_record(THD *thd, TABLE *table); 
 
static ST_FIELD_INFO mysql_is_hello_field_info[]=
{
  {"FILESYSTEM", 120, MYSQL_TYPE_STRING, 0, 0, "Filesystem", 0},
  {"SIZE", 7, MYSQL_TYPE_FLOAT, 0, 0, "Mountpoint Size", 0},
  {"USED", 7, MYSQL_TYPE_FLOAT, 0, 0, "Used Space", 0},
  {"AVAILABLE", 7, MYSQL_TYPE_FLOAT, 0, 0, "Available Space", 0},
  {"CAPACITY", 120, MYSQL_TYPE_LONGLONG, 0, 0, "Percent Used", 0},
  {"MOUNTED_ON", 120, MYSQL_TYPE_STRING, 0, 0, "Filesystem Mounted On" , 0},
  {0, 0, MYSQL_TYPE_STRING, 0, 0, 0, 0}
};





int mysql_is_hello_fill_table( THD *thd, TABLE_LIST *tables, Item *cond)
 {
  int status = 0;
 TABLE *table= (TABLE *)tables->table;
 const CHARSET_INFO *scs = system_charset_info;
  
	    FILE *in;
	    int n = 0;
	    int m = 0;
	    //extern FILE *popen();
	    char buff[512];
	 
	    if(!(in = popen("df", "r"))){
	        exit(1);
	    }
	    
	    while(fgets(buff, sizeof(buff), in)!=NULL){
		
				
		if(n>0)	 
		{       
 		
		istringstream iss(buff);
		char sub[128];

		    while(iss>>sub)
		    {
		       if (m==1||m==2||m==3)
				{			    
 				 double d1;
  				d1 = strtod (sub, NULL);
				d1 = d1/1024;
				d1 = d1/1024;
				d1 = roundf(d1 * 100) / 100;
  				//cout <<"Substring: "<< m <<"   " << d1 << endl ;
  				//cout <<"Substring: "<< m <<"   " << f1 << endl ;
				table->field[m]->store(d1); 
			

				}		
		        //cout << "Substring: " << m << "   "<< sub << endl;
			else
				if(m==4)
				{
				int i=0;
				char str[4];
				 while (sub[i]!= '%')
				{
				str[i]=sub[i];
				
				//cout <<str<<endl;
				i++;
				}
				str[i]='\0';	
				stringstream strstream(str);
				int x;
				strstream >> x;
				if (!strstream)
				{
				 //the conversion failed
				}
				 table->field[m]->store(x);
					}

			
			else
			table->field[m]->store(sub, strlen(sub), scs);
			m++;
		    }
			 if (m==6)
			{
			  m=0;
			status = schema_table_store_record(thd,table);  
			}
		}  	
		else	
		
		n++;
	    }
	    pclose(in);
	 
	
   //status=1;
  return status;
	 
	}

static int mysql_is_plugin_init(void *p)
{
  ST_SCHEMA_TABLE * schema= (ST_SCHEMA_TABLE *) p;

  schema->fields_info= mysql_is_hello_field_info;
  schema->fill_table= mysql_is_hello_fill_table;

  return 0;
}

static int mysql_is_plugin_deinit(void *p)
{
  return 0;
}
struct st_mysql_information_schema mysql_is_hello_plugin =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

mysql_declare_plugin(mysql_is_hello)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,                 
  &mysql_is_hello_plugin,                          
  "DISK_INFO",                                   
  "SALIL JAIN ",
  "Monitor Memory Status at Server",                                   
  PLUGIN_LICENSE_GPL,                              
  mysql_is_plugin_init,                      
  mysql_is_plugin_deinit,                    
  0x0010,                                         
  NULL,                                            
  NULL,                                            
  NULL                                            
}
mysql_declare_plugin_end;




