
/*
 *  Module: opt_decoder.h
 *
 *  Description: Parse argc/argv and serialize to a structure.
 */

enum opt_type
{
    opt_type_boolean = 1,
    opt_type_number = 2,
    opt_type_string = 3,
    opt_type_object = 4
};

/*
 *  Structure: opt_descriptor
 *
 *  Description:
 *
 *  Usage: Declare an array of this type to define the all fields of a
 *  structure.  Mark the end of the array using a NULL entry.
 */

struct opt_descriptor
{
    char const * p_short_name;
    char const * p_long_name;
    char const * p_help;
    size_t i_offset;
    size_t e_type;
    struct opt_descriptor const * p_descriptor_array;
};

char * * opt_decoder_scan(
    struct opt_descriptor const * p_descriptor_array,
    char * * arg_cur,
    char * * arg_end,
    void * p_object);

