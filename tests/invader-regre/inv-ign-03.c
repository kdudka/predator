#include <stdlib.h>

int main()
{
    void *ptr;
    void *fixed_array_of_ptr[7];
    char string[] = "static string";

    /* Invader let you silently free statically allocated objects */
    free(&ptr);
    free(fixed_array_of_ptr);
    free(string);

    return 0;
}
