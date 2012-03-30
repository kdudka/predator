static void __checked_list_add(struct list_head *node, struct list_head *list)
{
    struct list_head *pos;
    for (pos = list->next; pos != list; pos = pos->next)
        if (pos == node)
            ___sl_error("list_add() misused, node already in");

    list_add(node, list);
}

#define list_add __checked_list_add
