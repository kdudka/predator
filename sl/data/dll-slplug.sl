cl_factory.cc:268: debug: ClFactory: config_string: listener="pp" cld="unify_labels_fnc,unify_regs" [internal location]
cl_factory.cc:281: debug: ClFactory: looking for listener: pp [internal location]
cl_factory.cc:293: debug: ClFactory: listener 'pp' successfully created [internal location]
cl_factory.cc:208: debug: CldChainFactory: cldString: unify_labels_fnc,unify_regs [internal location]
cl_factory.cc:220: debug: CldChainFactory: looking for decorator: unify_regs [internal location]
cl_factory.cc:239: debug: CldChainFactory: decorator 'unify_regs' created successfully [internal location]
cl_factory.cc:220: debug: CldChainFactory: looking for decorator: unify_labels_fnc [internal location]
cl_factory.cc:239: debug: CldChainFactory: decorator 'unify_labels_fnc' created successfully [internal location]
cl_factory.cc:303: debug: ClFactory: createCldIntegrityChk() completed successfully [internal location]
dll_remove(%arg1: list, %arg2: item):
		goto L1

	L1:
		%r1 := (item == 0)
		if (%r1)
			goto L2
		else
			goto L3

	L2:
		goto L4

	L3:
		%r2 := [item:next]
		%r1 := (%r2 != 0)
		if (%r1)
			goto L5
		else
			goto L6

	L5:
		%r3 := [item:next]
		%r4 := [item:prev]
		[%r3:prev] := %r4
		goto L7

	L6:
		%r5 := [item:prev]
		[list:end] := %r5
		goto L7

	L7:
		%r6 := [item:prev]
		%r1 := (%r6 != 0)
		if (%r1)
			goto L8
		else
			goto L9

	L8:
		%r7 := [item:prev]
		%r8 := [item:next]
		[%r7:next] := %r8
		goto L10

	L9:
		%r9 := [item:next]
		[list:beg] := %r9
		goto L10

	L10:
		%r10 := [list:size]
		%r11 := (%r10 + (-1))
		[list:size] := %r11
		free(item)
		goto L4

	L4:
		ret

dll_insert_before(%arg1: list, %arg2: item, %arg3: data):
		goto L1

	L1:
		new_item := create_item(data)
		[new_item:next] := item
		%r1 := [item:prev]
		[new_item:prev] := %r1
		%r2 := [item:prev]
		%r3 := (%r2 != 0)
		if (%r3)
			goto L2
		else
			goto L3

	L2:
		%r4 := [item:prev]
		[%r4:next] := new_item
		goto L4

	L3:
		[list:beg] := new_item
		goto L4

	L4:
		[item:prev] := new_item
		%r5 := [list:size]
		%r6 := (%r5 + 1)
		[list:size] := %r6
		%r7 := new_item
		ret %r7

dll_insert_after(%arg1: list, %arg2: item, %arg3: data):
		goto L1

	L1:
		new_item := create_item(data)
		%r1 := [item:next]
		[new_item:next] := %r1
		[new_item:prev] := item
		%r2 := [item:next]
		%r3 := (%r2 != 0)
		if (%r3)
			goto L2
		else
			goto L3

	L2:
		%r4 := [item:next]
		[%r4:prev] := new_item
		goto L4

	L3:
		[list:end] := new_item
		goto L4

	L4:
		[item:next] := new_item
		%r5 := [list:size]
		%r6 := (%r5 + 1)
		[list:size] := %r6
		%r7 := new_item
		ret %r7

dll_pop_front(%arg1: list, %arg2: data):
		goto L1

	L1:
		%r1 := dll_empty(list)
		%r2 := (%r1 != 0)
		if (%r2)
			goto L2
		else
			goto L3

	L2:
		__assert_fail("!dll_empty(list)", "dll.c", 127, __PRETTY_FUNCTION__)
		abort

	L3:
		item := [list:beg]
		[data] := [item:data]
		%r3 := [item:next]
		[list:beg] := %r3
		%r4 := [list:beg]
		%r2 := (%r4 != 0)
		if (%r2)
			goto L4
		else
			goto L5

	L4:
		%r5 := [list:beg]
		[%r5:prev] := 0
		goto L6

	L5:
		[list:end] := 0
		goto L6

	L6:
		%r6 := [list:size]
		%r7 := (%r6 + (-1))
		[list:size] := %r7
		free(item)
		ret

dll_pop_back(%arg1: list, %arg2: data):
		goto L1

	L1:
		%r1 := dll_empty(list)
		%r2 := (%r1 != 0)
		if (%r2)
			goto L2
		else
			goto L3

	L2:
		__assert_fail("!dll_empty(list)", "dll.c", 112, __PRETTY_FUNCTION__)
		abort

	L3:
		item := [list:end]
		[data] := [item:data]
		%r3 := [item:prev]
		[list:end] := %r3
		%r4 := [list:end]
		%r2 := (%r4 != 0)
		if (%r2)
			goto L4
		else
			goto L5

	L4:
		%r5 := [list:end]
		[%r5:next] := 0
		goto L6

	L5:
		[list:beg] := 0
		goto L6

	L6:
		%r6 := [list:size]
		%r7 := (%r6 + (-1))
		[list:size] := %r7
		free(item)
		ret

dll_push_front(%arg1: list, %arg2: data):
		goto L1

	L1:
		item := create_item(data)
		%r1 := [list:beg]
		[item:next] := %r1
		%r2 := [item:next]
		%r3 := (%r2 != 0)
		if (%r3)
			goto L2
		else
			goto L3

	L2:
		%r4 := [item:next]
		[%r4:prev] := item
		goto L3

	L3:
		[list:beg] := item
		%r5 := [list:end]
		%r3 := (%r5 == 0)
		if (%r3)
			goto L4
		else
			goto L5

	L4:
		[list:end] := item
		goto L5

	L5:
		%r6 := [list:size]
		%r7 := (%r6 + 1)
		[list:size] := %r7
		%r8 := item
		ret %r8

dll_push_back(%arg1: list, %arg2: data):
		goto L1

	L1:
		item := create_item(data)
		%r1 := [list:end]
		[item:prev] := %r1
		%r2 := [item:prev]
		%r3 := (%r2 != 0)
		if (%r3)
			goto L2
		else
			goto L3

	L2:
		%r4 := [item:prev]
		[%r4:next] := item
		goto L3

	L3:
		%r5 := [list:beg]
		%r3 := (%r5 == 0)
		if (%r3)
			goto L4
		else
			goto L5

	L4:
		[list:beg] := item
		goto L5

	L5:
		[list:end] := item
		%r6 := [list:size]
		%r7 := (%r6 + 1)
		[list:size] := %r7
		%r8 := item
		ret %r8

dll_data(%arg1: item):
		goto L1

	L1:
dll.c:85:12: warning: 'ADDR_EXPR' not handled
slplug.c:296: note: raised from 'handle_operand' [internal location]
dll.c:85:5: warning: 'ADDR_EXPR' not handled
slplug.c:328: note: raised from 'handle_stmt_unop' [internal location]
		ret %r1
dll.c:<unknown line>: error: uninitialized register %r3564

dll_prev(%arg1: item):
		goto L1

	L1:
		%r1 := [item:prev]
		ret %r1

dll_next(%arg1: item):
		goto L1

	L1:
		%r1 := [item:next]
		ret %r1

dll_end(%arg1: list):
		goto L1

	L1:
		%r1 := [list:end]
		ret %r1

dll_beg(%arg1: list):
		goto L1

	L1:
		%r1 := [list:beg]
		ret %r1

dll_size(%arg1: list):
		goto L1

	L1:
		%r1 := [list:size]
		ret %r1

dll_empty(%arg1: list):
		goto L1

	L1:
		%r1 := [list:beg]
		%r2 := (%r1 == 0)
		ret %r2

dll_destroy(%arg1: list):
		goto L1

	L1:
		item := [list:end]
		goto L2

	L3:
		prev := [item:prev]
		free(item)
		item := prev
		goto L2

	L2:
		%r1 := (item != 0)
		if (%r1)
			goto L3
		else
			goto L4

	L4:
		dll_init(list)
		ret

dll_init(%arg1: list):
		goto L1

	L1:
		[list:beg] := 0
		[list:end] := 0
		[list:size] := 0
		ret

static create_item(%arg1: data):
		goto L1

	L1:
		%r1 := malloc(80)
		item := %r1
		%r2 := (item == 0)
		if (%r2)
			goto L2
		else
			goto L3

	L2:
		dll_die(__FUNCTION__)
		abort()
		abort

	L3:
		[item:data] := [data]
		[item:next] := 0
		[item:prev] := 0
		%r3 := item
		ret %r3


