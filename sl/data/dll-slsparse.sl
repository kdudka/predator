
static create_item(%arg1: data):
		goto L1

	L1:
		%r1 := malloc(52)
		if (%r1)
			goto L2
		else
			goto L3

	L3:
		dll_die("create_item")
		abort()
		goto L2

	L2:
dll.c:26: warning: ignored instruction with operand of size 352
slsparse.c:563: note: raised from function 'is_insn_interesting' [internal location]
dll.c:26: warning: ignored instruction with operand of size 352
slsparse.c:563: note: raised from function 'is_insn_interesting' [internal location]
		[%r1:next] := 0
		[%r1:prev] := 0
		ret %r1

dll_init(%arg1: list):
		goto L1

	L1:
		[list:beg] := 0
		[list:end] := 0
		[list:size] := 0
		ret

dll_destroy(%arg1: list):
		goto L1

	L1:
		%r1 := [list:end]
		%r2 := %r1
		goto L2

	L2:
		%r3 := %r2
		if (%r3)
			goto L3
		else
			goto L4

	L3:
		%r4 := [%r3:prev]
		%r5 := %r3
		free(%r5)
		%r2 := %r4
		goto L2

	L4:
		dll_init(list)
		ret

dll_empty(%arg1: list):
		goto L1

	L1:
		%r1 := [list:beg]
		%r2 := (%r1 == 0)
		%r3 := %r2
		ret %r3

dll_size(%arg1: list):
		goto L1

	L1:
		%r1 := [list:size]
		ret %r1

dll_beg(%arg1: list):
		goto L1

	L1:
		%r1 := [list:beg]
		ret %r1

dll_end(%arg1: list):
		goto L1

	L1:
		%r1 := [list:end]
		ret %r1

dll_next(%arg1: item):
		goto L1

	L1:
		%r1 := [item:next]
		ret %r1

dll_prev(%arg1: item):
		goto L1

	L1:
		%r1 := [item:prev]
		ret %r1

dll_data(%arg1: item):
		goto L1

	L1:
		ret item

dll_push_back(%arg1: list, %arg2: data):
		goto L1

	L1:
		%r1 := create_item(data)
		%r2 := [list:end]
		[%r1:prev] := %r2
		if (%r2)
			goto L2
		else
			goto L3

	L2:
		[%r2:next] := %r1
		goto L3

	L3:
		%r3 := [list:beg]
		if (%r3)
			goto L4
		else
			goto L5

	L5:
		[list:beg] := %r1
		goto L4

	L4:
		[list:end] := %r1
		%r4 := [list:size]
		%r5 := (%r4 + 1)
		[list:size] := %r5
		ret %r1

dll_push_front(%arg1: list, %arg2: data):
		goto L1

	L1:
		%r1 := create_item(data)
		%r2 := [list:beg]
		[%r1:next] := %r2
		if (%r2)
			goto L2
		else
			goto L3

	L2:
		[%r2:prev] := %r1
		goto L3

	L3:
		[list:beg] := %r1
		%r3 := [list:end]
		if (%r3)
			goto L4
		else
			goto L5

	L5:
		[list:end] := %r1
		goto L4

	L4:
		%r4 := [list:size]
		%r5 := (%r4 + 1)
		[list:size] := %r5
		ret %r1

dll_pop_back(%arg1: list, %arg2: data):
		goto L1

	L1:
		%r1 := dll_empty(list)
		if (%r1)
			goto L2
		else
			goto L3

	L2:
		__assert_fail("!dll_empty(list)", "dll.c", 112, "dll_pop_back")
		goto L3

	L3:
		%r2 := [list:end]
dll.c:115: warning: ignored instruction with operand of size 352
slsparse.c:563: note: raised from function 'is_insn_interesting' [internal location]
dll.c:115: warning: ignored instruction with operand of size 352
slsparse.c:563: note: raised from function 'is_insn_interesting' [internal location]
		%r3 := [%r2:prev]
		[list:end] := %r3
		if (%r3)
			goto L4
		else
			goto L5

	L4:
		[%r3:next] := 0
		goto L6

	L5:
		[list:beg] := 0
		goto L6

	L6:
		%r4 := [list:size]
		%r5 := (%r4 + (-1))
		[list:size] := %r5
		%r6 := %r2
		free(%r6)
		ret

dll_pop_front(%arg1: list, %arg2: data):
		goto L1

	L1:
		%r1 := dll_empty(list)
		if (%r1)
			goto L2
		else
			goto L3

	L2:
		__assert_fail("!dll_empty(list)", "dll.c", 127, "dll_pop_front")
		goto L3

	L3:
		%r2 := [list:beg]
dll.c:130: warning: ignored instruction with operand of size 352
slsparse.c:563: note: raised from function 'is_insn_interesting' [internal location]
dll.c:130: warning: ignored instruction with operand of size 352
slsparse.c:563: note: raised from function 'is_insn_interesting' [internal location]
		%r3 := [%r2:next]
		[list:beg] := %r3
		if (%r3)
			goto L4
		else
			goto L5

	L4:
		[%r3:prev] := 0
		goto L6

	L5:
		[list:end] := 0
		goto L6

	L6:
		%r4 := [list:size]
		%r5 := (%r4 + (-1))
		[list:size] := %r5
		%r6 := %r2
		free(%r6)
		ret

dll_insert_after(%arg1: list, %arg2: item, %arg3: data):
		goto L1

	L1:
		%r1 := create_item(data)
		%r2 := [item:next]
		[%r1:next] := %r2
		[%r1:prev] := item
		%r3 := [item:next]
		if (%r3)
			goto L2
		else
			goto L3

	L2:
		[%r3:prev] := %r1
		goto L4

	L3:
		[list:end] := %r1
		goto L4

	L4:
		[item:next] := %r1
		%r4 := [list:size]
		%r5 := (%r4 + 1)
		[list:size] := %r5
		ret %r1

dll_insert_before(%arg1: list, %arg2: item, %arg3: data):
		goto L1

	L1:
		%r1 := create_item(data)
		[%r1:next] := item
		%r2 := [item:prev]
		[%r1:prev] := %r2
		%r3 := [item:prev]
		if (%r3)
			goto L2
		else
			goto L3

	L2:
		[%r3:next] := %r1
		goto L4

	L3:
		[list:beg] := %r1
		goto L4

	L4:
		[item:prev] := %r1
		%r4 := [list:size]
		%r5 := (%r4 + 1)
		[list:size] := %r5
		ret %r1

dll_remove(%arg1: list, %arg2: item):
		goto L1

	L1:
		if (item)
			goto L2
		else
			goto L3

	L2:
		%r1 := [item:next]
		if (%r1)
			goto L4
		else
			goto L5

	L4:
		%r2 := [item:prev]
		[%r1:prev] := %r2
		goto L6

	L5:
		%r3 := [item:prev]
		[list:end] := %r3
		goto L6

	L6:
		%r4 := [item:prev]
		if (%r4)
			goto L7
		else
			goto L8

	L7:
		%r5 := [item:next]
		[%r4:next] := %r5
		goto L9

	L8:
		%r6 := [item:next]
		[list:beg] := %r6
		goto L9

	L9:
		%r7 := [list:size]
		%r8 := (%r7 + (-1))
		[list:size] := %r8
		%r9 := item
		free(%r9)
		goto L3

	L3:
		ret


