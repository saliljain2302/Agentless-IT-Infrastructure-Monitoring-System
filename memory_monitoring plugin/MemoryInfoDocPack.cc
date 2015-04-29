
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


long long GetHugePages_totalInKB(void)//To fetch total number of Huge Page from meminfo
{
    char line[256];
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
        {

		return -1 ;

		}

    
    while(fgets(line, sizeof(line), meminfo))
    {
        long long hugePagesTotal;
        if(sscanf(line, "HugePages_Total: %lldkB", &hugePagesTotal) == 1)
        {
            fclose(meminfo);
            return hugePagesTotal;
        }
    }
 fclose(meminfo);
    return -2;
}
long long GetHugePages_FreeInKB(void) //To fetch total number of free Huge Page from meminfo
{
    char line[256];
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL)
        {

		return -1 ;

		}

    
    while(fgets(line, sizeof(line), meminfo))
    {
        long long hugePagesFree;
        if(sscanf(line, "HugePages_Free: %lld kB", &hugePagesFree) == 1)
        {
            fclose(meminfo);
            return hugePagesFree;
        }
    }

    // If we got here, then we couldn't find the proper line in the meminfo file:
    // do something appropriate like return an error code, throw an exception, etc.
    fclose(meminfo);
    return -2;
}

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
//Declairing & Defining the columns of the Innodb_CPU_INFO virtual table
static ST_FIELD_INFO mysql_memory_info_field_info[]=
{
    

  {"TotPhyMemoAvl", 64, MYSQL_TYPE_LONGLONG, 0, 0, "Total Phy Memo Avl in kB", 	NULL},
  {"PhyMemoCurrFree", 64, MYSQL_TYPE_LONGLONG, 0, 0, "Phy Memo Curr Free in kB", NULL},
  {"TotSwapSpaceAvl", 64, MYSQL_TYPE_LONGLONG, 0, 0, "Total Swap Space Avl in kB", NULL},
  {"SwapSpaceCurrFree", 64, MYSQL_TYPE_LONGLONG, 0, 0, "Phy Swap Space Curr Free in kB", NULL},
  {"TotHugePages", 64, MYSQL_TYPE_LONGLONG, 0, 0, "Total Huge Pages Avl", NULL},
  {"HugePagesFree", 64, MYSQL_TYPE_LONGLONG, 0, 0, "Huge Pages Free", NULL},
  {NULL, 0, MYSQL_TYPE_NULL, NULL, NULL, NULL, NULL}
};


//Function mysql_memory_info_fill_table fills the data into the virtual table at runtime.
//server passes a number of arguments to the fill_table function
int mysql_memory_info_fill_table( THD *thd, TABLE_LIST *tables, Item *cond)
{
//THD *thd - this is a pointer to an instance of the thread descriptor class. In practice, this is a direct handle to the current session
//TABLE_LIST *tables -corresponds to the runtime representation of the table we are implementing as it appears in a query
//COND - This is used to pass the instance of the Item class that holds the internal representation of the WHERE-clause
  
int status;
  
  TABLE *table= (TABLE *)tables->table;

  struct sysinfo memInfo;
     sysinfo (&memInfo);
    long long totalPhysMem = memInfo.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    totalPhysMem = totalPhysMem/1024;


long long physMemUsed =memInfo.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    physMemUsed = physMemUsed/1024;

  long long totalSwapSpace = memInfo.totalswap;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalSwapSpace *= memInfo.mem_unit;
    totalSwapSpace = totalSwapSpace/1024;


long long swapSpaceUsed = memInfo.freeswap;
    //Multiply in next statement to avoid int overflow on right hand side...
    swapSpaceUsed *= memInfo.mem_unit;
    swapSpaceUsed = swapSpaceUsed/1024;
long long totalHugePages =GetHugePages_totalInKB();
long long hugePagesFree =GetHugePages_FreeInKB();

    

  table->field[0]->store((longlong)totalPhysMem, true);
  table->field[1]->store((longlong)physMemUsed, true);
  table->field[2]->store((longlong)totalSwapSpace, true);
  table->field[3]->store((longlong)swapSpaceUsed, true); 
  table->field[4]->store((longlong)totalHugePages, true);
  table->field[5]->store((longlong)hugePagesFree, true);

  status = schema_table_store_record(
    thd
  , table
  );

  return status;
}
//Initialization(init) function for the MemoryDesc.so plugin
//When called, the server passes a pointer to an instance of a ST_SCHEMA_TABLE
//type-specific part of the plug-in API for information schema plug-ins is formed by the struct called ST_SCHEMA_TABLE

static int mysql_memory_info_plugin_init(void *p)
{
  ST_SCHEMA_TABLE * schema= (ST_SCHEMA_TABLE *) p;

  schema->fields_info= mysql_memory_info_field_info;
//fields_info - an array of ST_FIELD_INFO structure.

  schema->fill_table= mysql_memory_info_fill_table;
//fill_table - a pointer to a function that is called whenever the server wants to obtain rows from the information_schema table

  return 0;
}

//De-initialization(init) function for the MemoryDesc.so plugin
static int mysql_memory_info_plugin_deinit(void *p)
{
  return 0;
}

//interface type descriptor
struct st_mysql_information_schema mysql_memory_info_plugin =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

//to list the plug-in in the information_schema.PLUGINS
//Descriptor structure for the telling the server about plugin details
mysql_declare_plugin(mysql_is_hello)//start macro
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN, //Plugin Type               
  &mysql_memory_info_plugin, //plugin descriptor    
  "INNODB_MEMORY_INFO",  //Given table name                                 
  "SALIL JAIN ",//AUTHOR Name
  "Monitor Memory Status at Server", //Description of plugin                               
  PLUGIN_LICENSE_GPL, //Licensed under                       
  mysql_memory_info_plugin_init, //Initialization funtion name      
  mysql_memory_info_plugin_deinit, //De-initialization funtion name             
  0x0000,                                         
  NULL,  //System Variable                                           
  NULL,  //System Variable                                          
  NULL    //System Variable                                         
}
mysql_declare_plugin_end;//end macro




