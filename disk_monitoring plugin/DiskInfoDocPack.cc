//including header files
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


//Declaring store record function  defined in mysql-5.1.22-rc/sql/sql_show.cc.
bool schema_table_store_record(THD *thd, TABLE *table); 

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
//Declairing & Defining the columns of the Innodb_DISK_INFO virtual table
static ST_FIELD_INFO mysql_disk_info_plugin_info[]=
{
  {"FILESYSTEM", 120, MYSQL_TYPE_STRING, 0, 0, "Filesystem", 0},//Name of drive as per the System
  {"SIZE(GB)", 7, MYSQL_TYPE_FLOAT, 0, 0, "Mountpoint Size", 0},//Shows drive size
  {"USED(GB)", 7, MYSQL_TYPE_FLOAT, 0, 0, "Used Space", 0},//shows used space in drive 
  {"AVAILABLE(GB)", 7, MYSQL_TYPE_FLOAT, 0, 0, "Available Space", 0},//shows available in drive
  {"CAPACITY(%)", 120, MYSQL_TYPE_LONGLONG, 0, 0, "Percent Used", 0},//shows capacity in % 
  {"MOUNTED_ON", 120, MYSQL_TYPE_STRING, 0, 0, "Filesystem Mounted On" , 0},//shows disk drive's mounting point
  {0, 0, MYSQL_TYPE_STRING, 0, 0, 0, 0}//Null Element shows the end of the array. 
};



//Function mysql_disk_info_fill_table fills the data into the virtual table at runtime.
//server passes a number of arguments to the fill_table function
int mysql_disk_info_fill_table( THD *thd, TABLE_LIST *tables, Item *cond)
 {//1
//THD *thd - this is a pointer to an instance of the thread descriptor class. In practice, this is a direct handle to the current session
//TABLE_LIST *tables -corresponds to the runtime representation of the table we are implementing as it appears in a query
//COND - This is used to pass the instance of the Item class that holds the internal representation of the WHERE-clause
  int status = 0;
  TABLE *table= (TABLE *)tables->table;
  const CHARSET_INFO *scs = system_charset_info;
  FILE *in;
  int n = 0;
  int m = 0;
  char buff[512];
if(!(in = popen("df", "r")))
//taking the input in pipe from linux terminal command df
  {//2
	 exit(1);
  }//2
	    
  while(fgets(buff, sizeof(buff), in)!=NULL)
//reading the line one by one from pipe using pointer to it(in) and storing it in character array to process
       {//3		
	if(n>0)
	//to filter out first row from the input which is description of data in output	 
	  {//4       
 	   istringstream iss(buff);
	   char sub[128];
	   while(iss>>sub)
		{//5
		 if(m==1||m==2||m==3)
		   //Column 1,2 and 3 are of type MYSQL_TYPE_FLOAT so writing the instruction to store value combinely for 			     these columns. 
		   {//6			    
 		    double d1;
  		    d1 = strtod (sub, NULL);//converting the char array data "sub" to double(double is compatible to 							MYSQL_TYPE_FLOAT.) 
		    d1 = d1/1024;//converting the value in MB
		    d1 = d1/1024;//converting the value in GB
		    d1 = roundf(d1 * 100) / 100;//rounding the values upto two decimal digits
  		    table->field[m]->store(d1);//storing the value in DISK_INFO Table in column No 1,2,3
		   }//6		
		 else
		     if(m==4)
			//Column 4 is the only of type MYSQL_TYPE_LONGLONG so writing instructions separately.
		       {//7
			int i=0;
			char str[4];
			while (sub[i]!= '%')//Removing % from the char array data "sub" as we are storing the value only
			      {//8
			       str[i]=sub[i];
			       i++;
			      }//8
			str[i]='\0';	
			//Converting char array data to integer as per the requirement
			stringstream strstream(str);
			int x;
			strstream >> x;
			if (!strstream)
			   {//9
			    //the conversion failed
			   }//9
			table->field[m]->store(x);//storing the value in DISK_INFO Table in column 4
		       }//7
		     else
			table->field[m]->store(sub, strlen(sub), scs);//storing the value in other columns of type 			             						MYSQL_TYPE_STRING
	          m++;
		 }//5
	    if (m==6)
	       {//10
		m=0;
		status = schema_table_store_record(thd,table);//moving pointer to store in the next row of the DISK_INFO 									TABLE
	       }//10
	   }//4  	
	 else	
	     n++;
      }//3
	    pclose(in);
	 
   return status;
	 
}//1
//Initialization(init) function for the DiskDesc.so plugin
//When called, the server passes a pointer to an instance of a ST_SCHEMA_TABLE
//type-specific part of the plug-in API for information schema plug-ins is formed by the struct called ST_SCHEMA_TABLE
static int mysql_disk_info_plugin_init(void *p)
{
  ST_SCHEMA_TABLE * schema= (ST_SCHEMA_TABLE *) p;

  schema->fields_info= mysql_disk_info_plugin_info;
//fields_info - an array of ST_FIELD_INFO structure.

  schema->fill_table= mysql_disk_info_fill_table;
//fill_table - a pointer to a function that is called whenever the server wants to obtain rows from the information_schema table

  return 0;
}
//De-initialization(deinit) function for the DiskDesc.so plugin
static int mysql_disk_info_plugin_deinit(void *p)
{
  return 0;
}
//interface type descriptor
struct st_mysql_information_schema mysql_disk_info_plugin =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

//to list the plug-in in the information_schema.PLUGINS
//Descriptor structure for the telling the server about plugin details
mysql_declare_plugin(mysql_disk_info)//Start macro
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,//Plugin Type       
  &mysql_disk_info_plugin,                          
  "INNODB_DISK_INFO",   //Plugin_name                            
  "SALIL JAIN ",//Author Name
  "Monitor Disk Status at Server",//Description of plugin                                   
  PLUGIN_LICENSE_GPL,//Licensed under                              
  mysql_disk_info_plugin_init,//Initialization funtion name                      
  mysql_disk_info_plugin_deinit,//De-nitialization funtion name                    
  0x0000,                                         
  NULL, //System Variable 
  NULL, //System Variable                                       
  NULL  //Sytem  Variable      
}
mysql_declare_plugin_end;//End macro




