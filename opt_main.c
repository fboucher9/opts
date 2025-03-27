#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <opt_decoder.h>

struct opt_demo_third
{
    char const * p_third_string;
};

static struct opt_descriptor const opt_demo_third_descriptor[] =
{
    { "ts", "third-string", "",
        offsetof(struct opt_demo_third, p_third_string),
        opt_type_string, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

struct opt_demo_second
{
    char const * p_second_string;
    signed long i_second_signed;
    struct opt_demo_third o_third;
    unsigned long i_second_unsigned;
    char b_second_flag;
};

static struct opt_descriptor const opt_demo_enum1[] =
{
    { "123", "password", 0, 0, 0, 0 },
    { "-123", "negative", 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

static struct opt_descriptor const opt_demo_second_descriptor[] =
{
    { "sc", "second-flag", "",
        offsetof(struct opt_demo_second, b_second_flag),
        opt_type_boolean, 0 },
    { "sd", "second-signed", "",
        offsetof(struct opt_demo_second, i_second_signed),
        opt_type_signed_long, opt_demo_enum1 },
    { "su", "second-unsigned", "",
        offsetof(struct opt_demo_second, i_second_unsigned),
        opt_type_unsigned_long, opt_demo_enum1 },
    { "ss", "second-string", "",
        offsetof(struct opt_demo_second, p_second_string),
        opt_type_string, 0 },
    { "so", "third", "",
        offsetof(struct opt_demo_second, o_third),
        opt_type_object, opt_demo_third_descriptor },
    { 0, 0, 0, 0, 0, 0 }
};

struct opt_demo
{
    char const * p_string;
    signed long i_signed;
    struct opt_demo_second o_second;
    unsigned long i_unsigned;
    char b_flag;
};

static struct opt_descriptor const opt_demo_descriptor[] =
{
    { "pc", "flag", "",
        offsetof(struct opt_demo, b_flag),
        opt_type_boolean, 0 },
    { "pd", "signed", "",
        offsetof(struct opt_demo, i_signed),
        opt_type_signed_long, opt_demo_enum1 },
    { "pu", "unsigned", "",
        offsetof(struct opt_demo, i_unsigned),
        opt_type_unsigned_long, opt_demo_enum1 },
    { "ps", "string", "",
        offsetof(struct opt_demo, p_string),
        opt_type_string, 0 },
    { "po", "second", "",
        offsetof(struct opt_demo, o_second),
        opt_type_object, opt_demo_second_descriptor },
    { 0, 0, 0, 0, 0, 0 }
};

int main(
    int argc,
    char * * argv)
{
    struct opt_demo o_fields;
    char * * cur;
    memset(&o_fields, 0, sizeof(o_fields));
    cur = argv + 1;
    cur = opt_decoder_scan(opt_demo_descriptor,
        cur, argv + argc, &o_fields);

    if (cur < argv + argc)
    {
        printf("error here >> %s <<\n", *cur);
    }
    else
    {
        char b_object_prefix;

        b_object_prefix = 0;

        if (o_fields.b_flag)
        {
            printf("--flag\n");
        }
        if (o_fields.i_signed)
        {
            printf("--signed %ld\n", o_fields.i_signed);
        }
        if (o_fields.i_unsigned)
        {
            printf("--unsigned %lu\n", o_fields.i_unsigned);
        }
        if (o_fields.p_string)
        {
            printf("--string %s\n", o_fields.p_string);
        }

        if (o_fields.o_second.b_second_flag)
        {
            if (!b_object_prefix)
            {
                printf("--second\n");
                b_object_prefix = 1;
            }
            printf("    --second-flag\n");
        }
        if (o_fields.o_second.i_second_signed)
        {
            if (!b_object_prefix)
            {
                printf("--second\n");
                b_object_prefix = 1;
            }
            printf("    --second-signed %ld\n", o_fields.o_second.i_second_signed);
        }
        if (o_fields.o_second.i_second_unsigned)
        {
            if (!b_object_prefix)
            {
                printf("--second\n");
                b_object_prefix = 1;
            }
            printf("    --second-unsigned %lu\n", o_fields.o_second.i_second_unsigned);
        }
        if (o_fields.o_second.p_second_string)
        {
            if (!b_object_prefix)
            {
                printf("--second\n");
                b_object_prefix = 1;
            }
            printf("    --second-string %s\n", o_fields.o_second.p_second_string);
        }
        if (o_fields.o_second.o_third.p_third_string)
        {
            if (!b_object_prefix)
            {
                printf("--second\n");
                b_object_prefix = 1;
            }
            printf("    --third\n");
            printf("        --third-string %s\n",
                o_fields.o_second.o_third.p_third_string);
        }
    }

    return 0;
}
