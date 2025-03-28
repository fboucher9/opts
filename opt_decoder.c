
/*
 *
 *
 */


#include <stddef.h>
#include <string.h> /* strchr, strcmp, memcmp */
#include <stdio.h> /* sscanf */
#include <opt_decoder.h>


/*
 *  Module: opt_iterator.c
 *
 *  Description: Iterator for command-line parameters.
 *
 *  Usage:
 *
 *      struct opt_iterator it;
 *      char const * p;
 *      opt_iterator_init(&it, argc, argv);
 *      while (opt_iterator_next(&it, &p)
 *      {
 *          // use p...
 *      }
 */

struct opt_iterator
{
    char * * cur;
    char * * end;
    char * * checkpoint;
};

/*
 *  Function: opt_iterator_init()
 *
 *  Description: Initialize iterator using standard command-line parameters.
 *
 *  Parameters:
 *
 *      p_this - 
 *
 *      argc
 *
 *      argv
 *
 *  Returns: none.
 */

static void opt_iterator_init(
    struct opt_iterator * p_this,
    char * * arg_cur,
    char * * arg_end)
{
    p_this->cur = arg_cur;
    p_this->end = arg_end;
}

/*
 *  Function: opt_iterator_next()
 *
 *  Description: 
 *
 *  Parameters:
 *
 *      p_this - 
 *
 *      r_value - 
 *
 */

static char opt_iterator_next(
    struct opt_iterator * p_this,
    char const * * r_value)
{
    char b_result;

    p_this->checkpoint = p_this->cur;
    if (p_this->cur < p_this->end)
    {
        if (r_value)
        {
            *r_value = *(p_this->cur);
        }

        p_this->cur ++;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}


struct opt_node
{
    struct opt_descriptor const * p_descriptor_array;
    void * p_object;
};


struct opt_stack
{
    struct opt_node a_nodes[8u];
    unsigned int i_depth;
};


static char opt_stack_push(
    struct opt_stack * p_this,
    struct opt_descriptor const * p_descriptor_array,
    void * p_object)
{
    if (p_this->i_depth < sizeof(p_this->a_nodes)
        / sizeof(p_this->a_nodes[0]))
    {
        p_this->a_nodes[p_this->i_depth].p_descriptor_array =
            p_descriptor_array;
        p_this->a_nodes[p_this->i_depth].p_object = p_object;
        p_this->i_depth ++;
    }
    return 1;
}


static char opt_stack_pop(
    struct opt_stack * p_this)
{
    if (p_this->i_depth)
    {
        p_this->i_depth --;
    }
    return 0 != p_this->i_depth;
}


static char opt_stack_get_object(
    struct opt_stack * p_this,
    void * * r_object)
{
    char b_result;

    if (p_this->i_depth)
    {
        *r_object = p_this->a_nodes[p_this->i_depth - 1].p_object;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}


static char opt_stack_get_descriptor(
    struct opt_stack * p_this,
    struct opt_descriptor const * * r_descriptor)
{
    char b_result;

    if (p_this->i_depth)
    {
        *r_descriptor = p_this->a_nodes[p_this->i_depth - 1].p_descriptor_array;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}


struct opt_argument
{
    char const * p_body;
    char const * p_value;
    /* -- */
    size_t i_body_length;
    size_t i_padding[1u];
    /* -- */
    char c_prefix;
    char c_padding[7u];
};


static char opt_argument_init(
    struct opt_argument * p_argument,
    char const * p_name)
{
    char b_result;

    p_argument->c_prefix = p_name[0u];

    p_argument->p_body = p_name;

    while (('-' == p_argument->p_body[0u])
        || ('+' == p_argument->p_body[0u]))
    {
        p_argument->p_body ++;
    }

    {
        char const * p_split;

        p_split = strchr(p_argument->p_body, '=');
        if (p_split)
        {
            p_argument->i_body_length = (size_t)(p_split - p_argument->p_body);
            p_argument->p_value = p_split + 1;
        }
        else
        {
            p_argument->i_body_length = strlen(p_argument->p_body);
            p_argument->p_value = 0;
        }
    }

    b_result = 1;

    return b_result;
}


static char compare_argument_and_descriptor(
    struct opt_argument const * p_argument,
    struct opt_descriptor const * p_descriptor)
{
    return ((p_argument->i_body_length == strlen(p_descriptor->p_long_name))
        && (0 == memcmp(p_argument->p_body, p_descriptor->p_long_name,
                p_argument->i_body_length)))
        || ((p_argument->i_body_length == strlen(p_descriptor->p_short_name))
        && (0 == memcmp(p_argument->p_body, p_descriptor->p_short_name,
                p_argument->i_body_length)));
}


struct opt_decoder
{
    struct opt_stack o_stack;
    struct opt_iterator o_iterator;
};


static char opt_decoder_get_value(
    struct opt_decoder * p_this,
    struct opt_argument const * p_argument,
    char const * * r_value)
{
    char b_result;

    if ('+' == p_argument->c_prefix)
    {
        *r_value = 0;
        b_result = 1;
    }
    else
    {
        if (p_argument->p_value)
        {
            *r_value = p_argument->p_value;
            b_result = 1;
        }
        else
        {
            if (opt_iterator_next(&p_this->o_iterator, r_value))
            {
                b_result = 1;
            }
            else
            {
                b_result = 0;
            }
        }
    }

    return b_result;
}


union opt_decoder_field
{
    void * p_object;
    size_t i_address;
    char * r_flag;
    double * r_number;
    char const * * r_string;
};


static char opt_decoder_get_field(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor,
    union opt_decoder_field * r_field)
{
    char b_result;
    void * p_object;

    if (opt_stack_get_object(&p_this->o_stack, &p_object))
    {
        union opt_decoder_field o_field;
        o_field.i_address = (size_t)p_object + p_descriptor->i_offset;
        *r_field = o_field;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}


static char opt_decoder_process_flag_value(
    struct opt_decoder * p_this,
    struct opt_argument const * p_argument,
    struct opt_descriptor const * p_descriptor)
{
    char b_result;
    union opt_decoder_field o_field;
    char b_value;
    b_value = ('+' == p_argument->c_prefix) ? 0 : 1;
    if (opt_decoder_get_field(p_this, p_descriptor, &o_field))
    {
        *o_field.r_flag = b_value;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }
    return b_result;
}


static char const * opt_decoder_lookup_enumeration(
    struct opt_descriptor const * p_descriptor_array,
    char const * p_value)
{
    struct opt_descriptor const * p_descriptor;
    char b_found;

    b_found = 0;
    p_descriptor = p_descriptor_array;
    while (!b_found && p_descriptor && p_descriptor->p_short_name)
    {
        if (0 == strcmp(p_value, p_descriptor->p_long_name))
        {
            p_value = p_descriptor->p_short_name;
            b_found = 1;
        }
        else
        {
            p_descriptor ++;
        }
    }

    return p_value;
}


static char opt_decoder_process_number(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor,
    char const * p_value)
{
    char b_result;
    union opt_decoder_field o_field;

    if (opt_decoder_get_field(p_this, p_descriptor, &o_field))
    {
        double f_number;
        f_number = 0.0;
        if (p_value)
        {
            p_value = opt_decoder_lookup_enumeration(
                p_descriptor->p_descriptor_array, p_value);
            if (p_value)
            {
                sscanf(p_value, "%lf", &f_number);
            }
        }
        *o_field.r_number = f_number;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}


static char opt_decoder_process_string(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor,
    char const * p_value)
{
    char b_result;
    union opt_decoder_field o_field;

    if (opt_decoder_get_field(p_this, p_descriptor, &o_field))
    {
        *o_field.r_string = p_value;
        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}


static char opt_decoder_process_object(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor)
{
    union opt_decoder_field o_field;
    return opt_decoder_get_field(p_this, p_descriptor, &o_field)
        && opt_stack_push(&p_this->o_stack, p_descriptor->p_descriptor_array,
            o_field.p_object);
}


static char opt_decoder_process_value(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor,
    struct opt_argument const * p_argument)
{
    char const * p_value;

    if (opt_type_boolean == p_descriptor->e_type)
    {
        return opt_decoder_process_flag_value(p_this, p_argument,
            p_descriptor);
    }
    else if (opt_type_number == p_descriptor->e_type)
    {
        return opt_decoder_get_value(p_this, p_argument, &p_value) &&
            opt_decoder_process_number(p_this, p_descriptor, p_value);
    }
    else if (opt_type_string == p_descriptor->e_type)
    {
        return opt_decoder_get_value(p_this, p_argument, &p_value) &&
            opt_decoder_process_string(p_this, p_descriptor, p_value);
    }
    else if (opt_type_object == p_descriptor->e_type)
    {
        return opt_decoder_process_object(p_this, p_descriptor);
    }
    else
    {
        return 0;
    }
}


static char opt_decoder_lookup_descriptor(
    struct opt_decoder * p_this,
    struct opt_argument const * p_argument,
    struct opt_descriptor const * * r_descriptor)
{
    char b_found;
    struct opt_descriptor const * p_descriptor;

    b_found = 0;
    if (opt_stack_get_descriptor(&p_this->o_stack, &p_descriptor))
    {
        while (!b_found && p_descriptor->p_short_name)
        {
            if (compare_argument_and_descriptor(p_argument, p_descriptor))
            {
                *r_descriptor = p_descriptor;
                b_found = 1;
            }
            else
            {
                p_descriptor ++;
            }
        }
    }

    return b_found;
}


static char opt_decoder_process_argument(
    struct opt_decoder * p_this,
    struct opt_argument const * p_argument)
{
    char b_result;
    struct opt_descriptor const * p_descriptor;

    b_result = 1;
    while (b_result && !(opt_decoder_lookup_descriptor(p_this, p_argument,
            &p_descriptor)
            && opt_decoder_process_value(p_this, p_descriptor, p_argument)))
    {
        /* retry using parent descriptors */
        b_result = opt_stack_pop(&p_this->o_stack);
    }

    return b_result;
}


static char opt_decoder_step(
    struct opt_decoder * p_this)
{
    char const * p_name;
    struct opt_argument o_argument;

    return opt_iterator_next(&p_this->o_iterator, &p_name)
        && opt_argument_init(&o_argument, p_name)
        && opt_decoder_process_argument(p_this, &o_argument);
}


static void opt_decoder_run(
    struct opt_decoder * p_this)
{
    ptrdiff_t i_max_count;
    i_max_count = p_this->o_iterator.end - p_this->o_iterator.cur;
    while (opt_decoder_step(p_this) && (i_max_count > 0))
    {
        i_max_count --;
    }
}


static void opt_decoder_init(
    struct opt_decoder * p_this,
    char * * arg_cur,
    char * * arg_end)
{
    memset(p_this, 0, sizeof(*p_this));
    opt_iterator_init(&p_this->o_iterator, arg_cur, arg_end);
}


static void opt_decoder_cleanup(
    struct opt_decoder * p_this)
{
    (void)p_this;
}


char * * opt_decoder_scan(
    struct opt_descriptor const * p_descriptor_array,
    char * * arg_cur,
    char * * arg_end,
    void * p_object)
{
    char * * p_result;
    struct opt_decoder o_decoder;
    opt_decoder_init(&o_decoder, arg_cur, arg_end);
    opt_stack_push(&o_decoder.o_stack, p_descriptor_array, p_object);
    opt_decoder_run(&o_decoder);
    p_result = o_decoder.o_iterator.checkpoint;
    opt_decoder_cleanup(&o_decoder);
    return p_result;
}

