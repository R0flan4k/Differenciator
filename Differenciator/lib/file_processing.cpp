#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "file_processing.h"
#include "my_assert.h"


long get_file_size(FILE * fp)
{
    struct stat buffer = {};

    if (fstat(fileno(fp), &buffer))
        return (long) NULL;

    return buffer.st_size;
}


FILE * file_open(const char * file_name, const char * mode)
{
    FILE * fp = NULL;

    if ((fp = fopen(file_name, mode)) == NULL)
    {
        printf("Error. Can't open %s\n", file_name);
    }

    return fp;
}


char * read_file(char * buffer, const size_t buffer_size, FILE * fp)
{
    if (!(fread(buffer, sizeof(char), buffer_size, fp)))
    {
        printf("Error. Can't read the file.\n");

        return (char *) NULL;
    }

    return buffer;
}


size_t write_file(char * buffer, const size_t buffer_size, FILE * fp)
{
    size_t val = 0;

    if (!(val = fwrite(buffer, buffer_size, 1, fp)))
    {
        printf("Error. Can't write the file.\n");

        return (size_t) NULL;
    }

    return val;
}


long text_file_to_buffer(char * file_name, char * * buffer_ptr)
{
    MY_ASSERT(file_name);
    MY_ASSERT(buffer_ptr);

    FILE * fp = NULL;

    if (!(fp = file_open(file_name, "r")))
    {
        return (long) NULL;
    }

    long file_size = get_file_size(fp);
    long buffer_size = file_size + 1;

    if (!(*buffer_ptr = (char *) calloc(buffer_size, sizeof(char))))
    {
        printf("Can't allocate a memory\n");
        fclose(fp);
        return (long) NULL;
    }

    if (!(*buffer_ptr = read_file(*buffer_ptr, buffer_size, fp)))
    {
        fclose(fp);
        free(*buffer_ptr);
        return (long) NULL;
    }

    (*buffer_ptr)[buffer_size - 1] = '\0';

    fclose(fp);

    return buffer_size;
}
