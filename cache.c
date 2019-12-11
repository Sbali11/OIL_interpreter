#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "contracts.h"
#include "cachelab.h"

typedef struct {
    unsigned long set;
    unsigned long lines;
    unsigned long block;
    char* trace_file;
} Args ;


typedef struct line_struct {
    bool dirty;
    bool valid;
    long tag;
    char* byte;
    long last_used;
    struct line_struct* next;
} Line;


typedef struct{
    Line* line;
} Set;

typedef struct{
    long misses;
    long hits;
    long evicted;
    long long dirty_bytes_evicted;
    long long dirty_bytes_in_cache;
} Cache_res;

typedef struct{
    char* bytes;
    bool found;
    Line* line_found;
    Line* LRU;
    bool found_empty;
} Found;

/* frees the cache 
 * inputs : the cache and the number of sets
 */
void freeSets(Set* sets, long long no_set)
{
    Line* curr_line;
    Line* temp;
    for(long i=0; i< no_set; i++)
    {
        curr_line = sets[i].line;
        while(curr_line!=NULL)
        {
            temp = curr_line;
            curr_line = curr_line->next;
            free(temp);
        }
    }
    free(sets);
}




/* Finds if a valid line exists in the cache corresponding to 
 * the arguments exist 
 * return a stuct :
 * 1. stores whether the line was found
 * 2. the line 
 * 3. Least Recently Used line
 * Args:
 * 1. in_tag: instruction tag
 * 2. set: the set we need to check
 * 3. args: command line args
 * 4. track: to keep track of the hits, misses etc
 * 5. num_ins: instruction number*/
Found* found(long in_tag, Set* sets, Set set, Args args, 
        Cache_res* track, long num_ins)
{

    int count = 0;
    Found* res = (Found*)calloc(1,sizeof(Found));
    if(res==NULL)
    {
        freeSets(sets, pow(2, args.set));
        free(track);
        exit(1);
    }
    Line* curr_line = set.line;
    long last_use = 2147483647;
    Line* LRU = curr_line;
    long curr_use;
    Line* empty=NULL;
    //loops through all the lines in the corresponding set
    while(count< args.lines && curr_line!=NULL)
    {
        
        count++;
        //if the line is not valid, stores the line in empty
        if(!(curr_line->valid))
        {
            empty = curr_line;
            curr_line= curr_line-> next;
            continue;
        }
        //if the line is valid and the tag=tag in argument
        else if(curr_line->tag==in_tag)
        {

            res-> line_found = curr_line;
            res->found = true;
            track->hits++;
            curr_line->last_used = num_ins;
            return res;
        }
        //sees if the current line is the LRU wrt previous lines.
        else
        {
            curr_use = curr_line->last_used;
            if(curr_use<last_use)
            {
               LRU = curr_line;
               last_use = curr_use;
            }
        }

        curr_line = curr_line-> next;
    }
    if(empty!=NULL)
    {
        LRU=empty;
        res->found_empty = true;
    }
    res->LRU = LRU;
    track->misses+=1;
    return res;
}

//puts the missed memory block in the cache
void put_in_cache(Line* LRU, long num_ins, long tag)
{
    LRU->valid = true;
    LRU->tag = tag;
    LRU->last_used = num_ins;
}

// Function to initialize sets
Set* initSets(long long num_set, long long num_lines)
{
    Set* sets = (Set*) calloc(num_set, sizeof(Set));
    if(sets==NULL)
        exit(1);
    Line* curr_line;
    for(long long i=0; i< num_set; i++)
    {
        
        sets[i].line = (Line*)calloc(1, sizeof(Line));
        if(sets[i].line==NULL)
        {
            exit(1);
            freeSets(sets, num_set);
        }
        curr_line = sets[i].line;
        for(long long j=0; j< num_lines; j++)
        {
            curr_line->next = (Line*)calloc(1, sizeof(Line));
            curr_line = curr_line->next;
        }
       curr_line=NULL; 
    }

    return sets;
}


/* Function to get all command_line args
 * returns a pointer Args structure with the following fiels:
 * set: s, lines: E, block: b, trace_file
*/

Args* get_args(int argc, char* argv[])
{
    int opt = 0; 
    Args* curr_args =  malloc(sizeof(Args));
    if(curr_args==NULL)
        exit(1);
    while((opt=getopt(argc, argv, "s:E:b:t:"))!=-1)
    {
        switch(opt)
        {
            case 's':
                curr_args->set = atoi(optarg);
                break;
            case 'E':
                curr_args->lines = atoi(optarg);
                break;
            case 'b':
                curr_args->block = atoi(optarg);
                break;
            case 't':
                curr_args->trace_file = optarg;
                break;
        }
    }
    return curr_args;

}

/* Function for the store instruction
 * Args: Found* found_res: results of the found_res function
 * found_bool : bool to indicate if the line was found
 * track : Cache structure that keeps track of the hits, misses, etc
 * block_size : size of each block
 * num_ins : instruction number
 * in_tag : tag of the instruction */

void store(Found* found_res, bool found_bool, Cache_res* track, 
        long long block_size, unsigned long num_ins, long in_tag)
{
    Line* LRU;
    if(!found_bool)
    {
        LRU = found_res->LRU;
        if(LRU->valid)
        {
            track->evicted++;
            if(LRU->dirty)
                track->dirty_bytes_evicted += block_size;
            else
                track->dirty_bytes_in_cache += block_size;
        }
        else
            track->dirty_bytes_in_cache += block_size;
        LRU->dirty = true;
        put_in_cache(LRU,num_ins, in_tag);
    }
    
    else
    {
        if(!found_res->line_found->dirty)
        {
            found_res->line_found->dirty = true;
            track->dirty_bytes_in_cache+= block_size;
        }
    }
}

/* Function for the load instruction
 * Args: Found* found_res: results of the found_res function
 * found_bool : bool to indicate if the line was found
 * track : Cache structure that keeps track of the hits, misses, etc
 * block_size : size of each block
 * num_ins : instruction number
 * in_tag : tag of the instruction */
void load(Found* found_res, bool found_bool, Cache_res* track, 
        long long block_size, unsigned long num_ins, long in_tag)
{
    Line* LRU;
    if(!found_bool)
    {
        LRU = found_res->LRU;
        if(LRU->valid)
        {
            track->evicted++;
            if(LRU->dirty)
            {
                track->dirty_bytes_evicted += block_size;
                LRU-> dirty = false;
                track->dirty_bytes_in_cache -= block_size;
            }
        }
        put_in_cache(LRU, num_ins, in_tag);  
    }

}
int main(int argc, char* argv[])
{
    Args* args_ptr = get_args(argc, argv);
    Args args = *args_ptr;
    free(args_ptr);
    FILE* pfile;
    char access_type;
    unsigned long address;
    int size;
    unsigned long tag;
    unsigned long set_num;
    Found* found_res;
    unsigned long num_ins = 0;
    long long num_sets = pow(2, args.set);
    Set* cache = initSets(num_sets, args.lines);
    bool found_bool;
    Cache_res* track = (Cache_res*)calloc(1, sizeof(Cache_res));
    if(track==NULL)
    {
        freeSets(cache, num_sets);
        exit(1);
    }
    long long block_size;
    Set curr_set;
    pfile = fopen(args.trace_file,"r");
    long in_tag;
    tag = (64-args.set-args.block);
    
    while(fscanf(pfile, " %c %lx, %d", &access_type, &address, &size)>0)
    {
        in_tag = address>>(args.set+args.block);
        set_num = (address<<tag)>>(64-args.set);
        if(args.set==0)
            set_num = 0; 
        curr_set = cache[set_num];
        found_res = found(in_tag, cache, curr_set, 
                        args,track, num_ins);
        found_bool = found_res->found; 
        block_size = pow(2,args.block);
        switch(access_type)
        {
            case 'S':
                store(found_res, found_bool, track, 
                        block_size, num_ins, in_tag);
                break;
            
            case 'L':
                load(found_res, found_bool, track, 
                        block_size, num_ins, in_tag);
                break;
        }

        num_ins++;
    }
    printSummary(track->hits, track->misses, track->evicted,
            track->dirty_bytes_in_cache, track->dirty_bytes_evicted);
    freeSets(cache, num_sets);
    free(track);
    free(found_res);
    return 0;
}