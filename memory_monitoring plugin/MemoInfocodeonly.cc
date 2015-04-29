
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
long long GetHugePages_totalInKB(void)
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
long long GetHugePages_FreeInKB(void)
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


bool schema_table_store_record(THD *thd, TABLE *table); 

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




int mysql_memory_info_fill_table( THD *thd, TABLE_LIST *tables, Item *cond)
{
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

    Field** fields;
    fields = table->field;

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

static int mysql_memory_info_plugin_init(void *p)
{
  ST_SCHEMA_TABLE * schema= (ST_SCHEMA_TABLE *) p;

  schema->fields_info= mysql_memory_info_field_info;
  schema->fill_table= mysql_memory_info_fill_table;

  return 0;
}

static int mysql_memory_info_plugin_deinit(void *p)
{
  return 0;
}
struct st_mysql_information_schema mysql_memory_info_plugin =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

mysql_declare_plugin(mysql_is_hello)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,                 
  &mysql_memory_info_plugin,                          
  "MEMORY_INFO",                                   
  "SALIL JAIN ",
  "Monitor Memory Status at Server",                                   
  PLUGIN_LICENSE_GPL,                              
  mysql_memory_info_plugin_init,                      
  mysql_memory_info_plugin_deinit,                    
  0x0010,                                         
  NULL,                                            
  NULL,                                            
  NULL                                            
}
mysql_declare_plugin_end;




