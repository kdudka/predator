#include <stdlib.h>

int main()
{
    int **ppi = malloc(sizeof *ppi);
    *ppi = malloc(sizeof **ppi);
    ppi = realloc(ppi, 2 * sizeof *ppi);
    free(*ppi);
    free(ppi);
}
