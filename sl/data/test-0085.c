#include <stdlib.h>

int main()
{
    // define a user type
    struct T {
        struct T *next;
        struct T *prev;
    };

    // allocate an instance of it
    struct T *null = (struct T *)0;
    struct T *data = malloc(sizeof *data);
    if (null == data)
        // OOM
        return EXIT_FAILURE;

    // now introduce some generic pointers
    void *const pd = data;
    void *const i0 = &data->next;
    void *const i1 = &data->prev;

    if (!pd || !i0 || !i1)
        // something went wrong, yell
        null->prev = i0;

    if (i0 == i1)
        // something went wrong, yell
        null->prev = i0;

    if (pd == i1)
        // something went wrong, yell
        null->next = i1;

    if (pd != i0)
        // something went wrong, yell
        null->next = pd;

    // wait, the following is _not_ covered by our aliasing check ... whoever
    // really needs it is either incompetent or using some tricky programming
    // techniques that we are not going to support anyway
    free(i0);

    // this is always bad idea
    free(i1);

    // this should be OK
    free(pd);

    return EXIT_SUCCESS;
}
