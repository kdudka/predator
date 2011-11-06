int main() {
    void *p;
    void **p1 = &p;
    void **p2;

    if (p1 == p2) {
        if (p1 != p2) {
            *p2 = (void *)0;
        }
    }

    return 0;
}
