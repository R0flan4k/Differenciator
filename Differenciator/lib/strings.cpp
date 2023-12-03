#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "strings.h"
#include "my_assert.h"


void show_text(const char * text, const size_t size)
{
    size_t i = 0;

    while (i < size)
    {
        fputc(text[i], stdout);

        i++;
    }
}


size_t get_strings_num(char * buffer)
{
    size_t strings_num = 0;
    char * string_pointer = buffer;

    while ((string_pointer = strchr(string_pointer, '\n')) != NULL)
    {
        *string_pointer = '\0';
        string_pointer++;
        strings_num++;
    }

    return strings_num + 1;
}


void get_pointers(const char * buffer, const char * pointers[], const size_t strings_num)
{
    size_t i = 0;
    char const * string_pointer = buffer;

    while (i < strings_num)
    {
        pointers[i] = string_pointer;

        string_pointer = strchr(string_pointer, '\0') + 1;

        i++;
    }
}


void show_strings(const char * const * strings, const size_t strings_num)
{
    for (size_t i = 0; i < strings_num; i++)
    {
        printf("%s\n", strings[i]);
    }
}


const char * skip_word(const char * buffer)
{
    MY_ASSERT(buffer != nullptr);

    while (!isspace(*buffer) && *buffer != EOF && *buffer != '\0' && *buffer != ';')
        buffer++;

    while (isspace(*buffer) && *buffer != EOF && *buffer != '\0' && *buffer != ';')
        buffer++;

    return (buffer);
}


const char * next_word(const char * buffer)
{
    MY_ASSERT(buffer != nullptr);

    while (!isspace(*buffer) && *(buffer + 1) != EOF && *(buffer + 1) != '\0')
        buffer++;

    return (buffer + 1);
}


char * skip_spaces(char * buffer)
{
    MY_ASSERT(buffer);

    while (isspace(*buffer) && *buffer != EOF && *buffer != '\0')
        buffer++;

    return (buffer);
}


char * skip_no_spaces(char * buffer)
{
    MY_ASSERT(buffer);

    while (!isspace(*buffer) && *buffer!= EOF && *buffer != '\0' &&
           *buffer != '{' && *buffer != '}')
        buffer++;

    return (buffer);
}


char * make_file_extension(char * target, const char * file_name, const char * extension)
{
    MY_ASSERT(target);
    MY_ASSERT(file_name);
    MY_ASSERT(extension);

    strcpy(target, file_name);
    strcat(target, extension);

    return target;
}


size_t get_input(char * buffer, size_t buffer_size)
{
    MY_ASSERT(buffer);

    char character = 0;
    size_t i = 0;

    while ((character = getchar()) != '\n' && character != EOF &&
           i < buffer_size)
    {
        *buffer = character;
        buffer++;
        i++;
    }

    return i;
}
