void dm_list_init(struct dm_list *head)
{
 head->n = head->p = head;
}





void dm_list_add(struct dm_list *head, struct dm_list *elem)
{
 ((head->n) ? (void) (0) : __assert_fail ("head->n", "datastruct/list.c", 34, __PRETTY_FUNCTION__));

 elem->n = head;
 elem->p = head->p;

 head->p->n = elem;
 head->p = elem;
}





void dm_list_add_h(struct dm_list *head, struct dm_list *elem)
{
 ((head->n) ? (void) (0) : __assert_fail ("head->n", "datastruct/list.c", 49, __PRETTY_FUNCTION__));

 elem->n = head->n;
 elem->p = head;

 head->n->p = elem;
 head->n = elem;
}






void dm_list_del(struct dm_list *elem)
{
 elem->n->p = elem->p;
 elem->p->n = elem->n;
}




void dm_list_move(struct dm_list *head, struct dm_list *elem)
{
        dm_list_del(elem);
        dm_list_add(head, elem);
}




int dm_list_empty(const struct dm_list *head)
{
 return head->n == head;
}




int dm_list_start(const struct dm_list *head, const struct dm_list *elem)
{
 return elem->p == head;
}




int dm_list_end(const struct dm_list *head, const struct dm_list *elem)
{
 return elem->n == head;
}




struct dm_list *dm_list_first(const struct dm_list *head)
{
 return (dm_list_empty(head) ? ((void *)0) : head->n);
}




struct dm_list *dm_list_last(const struct dm_list *head)
{
 return (dm_list_empty(head) ? ((void *)0) : head->p);
}




struct dm_list *dm_list_prev(const struct dm_list *head, const struct dm_list *elem)
{
 return (dm_list_start(head, elem) ? ((void *)0) : elem->p);
}




struct dm_list *dm_list_next(const struct dm_list *head, const struct dm_list *elem)
{
 return (dm_list_end(head, elem) ? ((void *)0) : elem->n);
}




unsigned int dm_list_size(const struct dm_list *head)
{
 unsigned int s = 0;
 const struct dm_list *v;

 for (v = (head)->n; v != head; v = v->n)
     s++;

 return s;
}






void dm_list_splice(struct dm_list *head, struct dm_list *head1)
{
 ((head->n) ? (void) (0) : __assert_fail ("head->n", "datastruct/list.c", 155, __PRETTY_FUNCTION__));
 ((head1->n) ? (void) (0) : __assert_fail ("head1->n", "datastruct/list.c", 156, __PRETTY_FUNCTION__));

 if (dm_list_empty(head1))
     return;

 head1->p->n = head;
 head1->n->p = head->p;

 head->p->n = head1->n;
 head->p = head1->p;

 dm_list_init(head1);
}
