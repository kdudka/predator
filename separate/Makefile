SPARSE_CFLAGS += `pkg-config sparse --cflags`
SPARSE_LIBS ?= `pkg-config sparse --libs`

.PHONY: check clean demo

separate: separate.c
	$(CC) $(CFLAGS) $(SPARSE_CFLAGS) -o $@ $< $(SPARSE_LIBS)

test-list: list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

check: test-list
	./runtest.sh list

clean:
	rm -f separate test-list

demo: check list.c separate
	./separate list.c
