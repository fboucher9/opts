#include <stddef.h>
#include <string.h> /* strchr, strcmp, memcmp */
#include <stdio.h> /* sscanf */
#include <opt_decoder.h>

struct opt_iterator
{
    char * * cur;
    char * * end;
    char * * checkpoint;
};

static void opt_iterator_init(struct opt_iterator * p_this,
    char * * arg_cur, char * * arg_end)
{
    p_this->cur = arg_cur;
    p_this->end = arg_end;
}

static char opt_iterator_next(struct opt_iterator * p_this,
    char const * * r_value)
{
    p_this->checkpoint = p_this->cur;
    if (p_this->cur < p_this->end)
    {
        if (r_value)
        {
            *r_value = *(p_this->cur);
        }
        p_this->cur ++;
        return 1;
    }
    return 0;
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

static char opt_stack_push(struct opt_stack * p_this,
    struct opt_descriptor const * p_descriptor_array, void * p_object)
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

static char opt_stack_pop(struct opt_stack * p_this)
{
    if (p_this->i_depth)
    {
        p_this->i_depth --;
    }
    return 0 != p_this->i_depth;
}

static char opt_stack_get_object(struct opt_stack * p_this, void * * r_object)
{
    if (p_this->i_depth)
    {
        *r_object = p_this->a_nodes[p_this->i_depth - 1].p_object;
        return 1;
    }
    return 0;
}

static char opt_stack_get_descriptor(
    struct opt_stack * p_this,
    struct opt_descriptor const * * r_descriptor)
{
    if (p_this->i_depth)
    {
        *r_descriptor = p_this->a_nodes[p_this->i_depth - 1].p_descriptor_array;
        return 1;
    }
    return 0;
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

static char opt_argument_init(struct opt_argument * p_argument,
    char const * p_name)
{
    char const * p_split;
    p_argument->c_prefix = p_name[0u];
    p_argument->p_body = p_name;
    while (('-' == p_argument->p_body[0u])
        || ('+' == p_argument->p_body[0u]))
    {
        p_argument->p_body ++;
    }
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
    return 1;
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

union opt_field
{
    void * p_object;
    size_t i_address;
    char * r_boolean;
    double * r_number;
    char const * * r_string;
};

static char opt_field_init(void * p_object, size_t i_offset,
    union opt_field * r_field)
{
    r_field->i_address = (size_t)p_object + i_offset;
    return 1;
}

static char opt_field_write_boolean(union opt_field o_field, char b_value)
{
    *o_field.r_boolean = b_value;
    return 1;
}

static char opt_field_write_number(union opt_field o_field, double f_number)
{
    *o_field.r_number = f_number;
    return 1;
}

static char opt_field_write_string(union opt_field o_field,
    char const * p_value)
{
    *o_field.r_string = p_value;
    return 1;
}

struct opt_decoder
{
    struct opt_stack o_stack;
    struct opt_iterator o_iterator;
};

static char opt_decoder_get_value(struct opt_decoder * p_this,
    struct opt_argument const * p_argument, char const * * r_value)
{
    if ('+' == p_argument->c_prefix)
    {
        *r_value = 0;
        return 1;
    }

    if (p_argument->p_value)
    {
        *r_value = p_argument->p_value;
        return 1;
    }

    return opt_iterator_next(&p_this->o_iterator, r_value);
}

static char opt_decoder_get_field(struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor, union opt_field * r_field)
{
    void * p_object;
    return opt_stack_get_object(&p_this->o_stack, &p_object)
        && opt_field_init(p_object, p_descriptor->i_offset, r_field);
}

static char opt_decoder_process_boolean_value(struct opt_decoder * p_this,
    struct opt_argument const * p_argument,
    struct opt_descriptor const * p_descriptor)
{
    union opt_field o_field;
    return opt_decoder_get_field(p_this, p_descriptor, &o_field)
        && opt_field_write_boolean(o_field, ('+' != p_argument->c_prefix));
}

static char opt_decoder_lookup_enumeration(
    struct opt_descriptor const * p_descriptor,
    char const * p_value, char const * * r_expansion)
{
    if (p_value)
    {
        while (p_descriptor && p_descriptor->p_short_name)
        {
            if (0 == strcmp(p_value, p_descriptor->p_long_name))
            {
                *r_expansion = p_descriptor->p_short_name;
                return 1;
            }
            p_descriptor ++;
        }
    }
    *r_expansion = p_value;
    return 1;
}

static char opt_decoder_scan_number(char const * p_value, double * r_number)
{
    *r_number = 0.0;
    if (p_value)
    {
        sscanf(p_value, "%lf", r_number);
    }
    return 1;
}

static char opt_decoder_process_number(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor,
    char const * p_value)
{
    union opt_field o_field;
    double f_number;
    char const * p_expansion;
    return opt_decoder_get_field(p_this, p_descriptor, &o_field)
        && opt_decoder_lookup_enumeration(
            p_descriptor->p_descriptor_array, p_value, &p_expansion)
        && opt_decoder_scan_number(p_expansion, &f_number)
        && opt_field_write_number(o_field, f_number);
}

static char opt_decoder_process_string(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor,
    char const * p_value)
{
    union opt_field o_field;
    return opt_decoder_get_field(p_this, p_descriptor, &o_field)
        && opt_field_write_string(o_field, p_value);
}

static char opt_decoder_process_object(
    struct opt_decoder * p_this,
    struct opt_descriptor const * p_descriptor)
{
    union opt_field o_field;
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
    return ((opt_type_boolean == p_descriptor->e_type)
        && opt_decoder_process_boolean_value(p_this, p_argument, p_descriptor))
        || ((opt_type_number == p_descriptor->e_type)
            && opt_decoder_get_value(p_this, p_argument, &p_value)
            && opt_decoder_process_number(p_this, p_descriptor, p_value))
        || ((opt_type_string == p_descriptor->e_type)
            && opt_decoder_get_value(p_this, p_argument, &p_value)
            && opt_decoder_process_string(p_this, p_descriptor, p_value))
        || ((opt_type_object == p_descriptor->e_type)
            && opt_decoder_process_object(p_this, p_descriptor));
}

static char opt_decoder_lookup_descriptor(struct opt_decoder * p_this,
    struct opt_argument const * p_argument,
    struct opt_descriptor const * * r_descriptor)
{
    struct opt_descriptor const * p_descriptor;
    if (opt_stack_get_descriptor(&p_this->o_stack, &p_descriptor))
    {
        while (p_descriptor->p_short_name)
        {
            if (compare_argument_and_descriptor(p_argument, p_descriptor))
            {
                *r_descriptor = p_descriptor;
                return 1;
            }
            p_descriptor ++;
        }
    }
    return 0;
}

static char opt_decoder_process_argument(
    struct opt_decoder * p_this,
    struct opt_argument const * p_argument)
{
    do
    {
        struct opt_descriptor const * p_descriptor;
        if (opt_decoder_lookup_descriptor(p_this, p_argument, &p_descriptor)
            && opt_decoder_process_value(p_this, p_descriptor, p_argument))
        {
            return 1;
        }
    }
    /* retry using parent descriptors */
    while (opt_stack_pop(&p_this->o_stack));
    return 0;
}

static void opt_decoder_run(struct opt_decoder * p_this)
{
    ptrdiff_t i_max_count;
    char const * p_name;
    struct opt_argument o_argument;
    i_max_count = p_this->o_iterator.end - p_this->o_iterator.cur;
    while (opt_iterator_next(&p_this->o_iterator, &p_name)
        && opt_argument_init(&o_argument, p_name)
        && opt_decoder_process_argument(p_this, &o_argument)
        && (i_max_count > 0))
    {
        i_max_count --;
    }
}

char * * opt_decoder_scan(struct opt_descriptor const * p_descriptor_array,
    char * * arg_cur, char * * arg_end, void * p_object)
{
    struct opt_decoder o_decoder;
    memset(&o_decoder, 0, sizeof(o_decoder));
    opt_iterator_init(&o_decoder.o_iterator, arg_cur, arg_end);
    opt_stack_push(&o_decoder.o_stack, p_descriptor_array, p_object);
    opt_decoder_run(&o_decoder);
    return o_decoder.o_iterator.checkpoint;
}

