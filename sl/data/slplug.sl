slplug.c:1368: ./slplug.so: plugin_init
slplug.c:1370: ./slplug.so: using gcc 4.5.0 experimental, built at 20090804
cl_factory.cc:264: debug: ClFactory: config_string: listener="pp" listener_args="" cld="unify_labels_fnc,unify_regs" [internal location]
cl_factory.cc:277: debug: ClFactory: looking for listener: pp [internal location]
cl_factory.cc:287: debug: ClFactory: creating listener 'pp' with args '' [internal location]
cl_factory.cc:295: debug: ClFactory: createCldIntegrityChk() completed successfully [internal location]
cl_factory.cc:204: debug: CldChainFactory: cldString: unify_labels_fnc,unify_regs [internal location]
cl_factory.cc:216: debug: CldChainFactory: looking for decorator: unify_regs [internal location]
cl_factory.cc:226: debug: CldChainFactory: decorator 'unify_regs' created successfully [internal location]
cl_factory.cc:216: debug: CldChainFactory: looking for decorator: unify_labels_fnc [internal location]
cl_factory.cc:226: debug: CldChainFactory: decorator 'unify_labels_fnc' created successfully [internal location]
cl_factory.cc:264: debug: ClFactory: config_string: listener="dotgen" listener_args="data/all.dot" cld="unify_labels_fnc,unify_regs" [internal location]
cl_factory.cc:277: debug: ClFactory: looking for listener: dotgen [internal location]
cl_factory.cc:287: debug: ClFactory: creating listener 'dotgen' with args 'data/all.dot' [internal location]
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/all.dot' [internal location]
cl_factory.cc:295: debug: ClFactory: createCldIntegrityChk() completed successfully [internal location]
cl_factory.cc:204: debug: CldChainFactory: cldString: unify_labels_fnc,unify_regs [internal location]
cl_factory.cc:216: debug: CldChainFactory: looking for decorator: unify_regs [internal location]
cl_factory.cc:226: debug: CldChainFactory: decorator 'unify_regs' created successfully [internal location]
cl_factory.cc:216: debug: CldChainFactory: looking for decorator: unify_labels_fnc [internal location]
cl_factory.cc:226: debug: CldChainFactory: decorator 'unify_labels_fnc' created successfully [internal location]
slplug.c:1386: ./slplug.so: 'slplug 0.1 [experimental]' successfully initialized
slplug.c:1191: ./slplug.so: processing input file 'data/slplug.c'
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c.dot' [internal location]
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-plugin_init.dot' [internal location]
plugin_init(%arg1: [struct plugin_name_args *]plugin_info, %arg2: [struct plugin_gcc_version *]version):
		goto L1

	L1:
		[int]rv := slplug_init([struct plugin_name_args *]plugin_info, [struct sl_plug_options *]&opt)
		[bool]%r1 := ([int]rv != 0)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		[int]%r2 := [int]rv
		goto L4

	L3:
		[int]verbose.0 := [int]verbose
data/slplug.c:1368:5: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r3 := [int]%r4
		[bool]%r1 := ([bool]%r3 != false)
		if ([bool]%r1)
			goto L5
		else
			goto L6

	L5:
		[char *]plugin_name.1 := [char *]plugin_name
		[char *]%r5 := "%s:%d: %s: "
		[struct _IO_FILE *]stderr.2 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.3 := [struct _IO_FILE *]stderr.2
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.3, [char *]%r5, "data/slplug.c", 1368, [char *]plugin_name.1)
		[struct _IO_FILE *]stderr.4 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.5 := [struct _IO_FILE *]stderr.4
		__builtin_fwrite([char *]&__FUNCTION__[0], 1, 11, [struct FILE *]stderr.5)
		[struct _IO_FILE *]stderr.6 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.7 := [struct _IO_FILE *]stderr.6
		__builtin_fputc(10, [struct FILE *]stderr.7)
		goto L6

	L6:
		[int]verbose.8 := [int]verbose
data/slplug.c:1369:5: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r6 := [int]%r7
		[bool]%r1 := ([bool]%r6 != false)
		if ([bool]%r1)
			goto L7
		else
			goto L8

	L7:
		[char *]plugin_name.9 := [char *]plugin_name
		[char *]%r8 := "%s:%d: %s: "
		[struct _IO_FILE *]stderr.10 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.11 := [struct _IO_FILE *]stderr.10
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.11, [char *]%r8, "data/slplug.c", 1370, [char *]plugin_name.9)
		[char *]%r9 := [char *]version->datestamp
		[char *]%r10 := [char *]version->devphase
		[char *]%r11 := [char *]version->basever
		[char *]%r12 := "using gcc %s %s, built at %s"
		[struct _IO_FILE *]stderr.12 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.13 := [struct _IO_FILE *]stderr.12
		fprintf([struct FILE *]stderr.13, [char *]%r12, [char *]%r11, [char *]%r10, [char *]%r9)
		[struct _IO_FILE *]stderr.14 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.15 := [struct _IO_FILE *]stderr.14
		__builtin_fputc(10, [struct FILE *]stderr.15)
		goto L8

	L8:
		[int]verbose.16 := [int]verbose
data/slplug.c:1375:28: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		cl_global_init_defaults(NULL, [int]%r13)
		[struct cl_code_listener *]cl.17 := create_cl_chain([struct sl_plug_options *]&opt)
		[struct cl_code_listener *]cl := [struct cl_code_listener *]cl.17
		[struct cl_code_listener *]cl.18 := [struct cl_code_listener *]cl
		[bool]%r1 := ([struct cl_code_listener *]cl.18 == NULL)
		if ([bool]%r1)
			goto L9
		else
			goto L10

	L9:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 1377, [char *]&__FUNCTION__[0])
		abort

	L10:
		[struct htab *]type_db.19 := type_db_create()
		[struct htab *]type_db := [struct htab *]type_db.19
		[struct htab *]type_db.20 := [struct htab *]type_db
		[bool]%r1 := ([struct htab *]type_db.20 == NULL)
		if ([bool]%r1)
			goto L11
		else
			goto L12

	L11:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 1381, [char *]&__FUNCTION__[0])
		abort

	L12:
		[struct htab *]type_db.21 := [struct htab *]type_db
		[struct cl_code_listener *]cl.22 := [struct cl_code_listener *]cl
		register_type_db([struct cl_code_listener *]cl.22, [struct htab *]type_db.21)
		[char *]%r14 := [char *]plugin_info->base_name
		sl_regcb([char *]%r14)
		[int]verbose.23 := [int]verbose
data/slplug.c:1386:5: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r15 := [int]%r16
		[bool]%r1 := ([bool]%r15 != false)
		if ([bool]%r1)
			goto L13
		else
			goto L14

	L13:
		[char *]plugin_name.24 := [char *]plugin_name
		[char *]%r17 := "%s:%d: %s: "
		[struct _IO_FILE *]stderr.25 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.26 := [struct _IO_FILE *]stderr.25
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.26, [char *]%r17, "data/slplug.c", 1386, [char *]plugin_name.24)
		[char *]%r18 := [char *]plugin_info->version
		[char *]%r19 := "'%s' successfully initialized"
		[struct _IO_FILE *]stderr.27 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.28 := [struct _IO_FILE *]stderr.27
		fprintf([struct FILE *]stderr.28, [char *]%r19, [char *]%r18)
		[struct _IO_FILE *]stderr.29 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.30 := [struct _IO_FILE *]stderr.29
		__builtin_fputc(10, [struct FILE *]stderr.30)
		goto L14

	L14:
		[int]%r2 := 0
		goto L4

	L4:
		ret [int]%r2
data/slplug.c:1368:5: error: uninitialized register %r4
data/slplug.c:1369:5: error: uninitialized register %r7
data/slplug.c:1375:28: error: uninitialized register %r13
data/slplug.c:1386:5: error: uninitialized register %r16

data/slplug.c:1368:5: error: uninitialized register %r4
data/slplug.c:1369:5: error: uninitialized register %r7
data/slplug.c:1375:28: error: uninitialized register %r13
data/slplug.c:1386:5: error: uninitialized register %r16
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-create_cl_chain.dot' [internal location]
static create_cl_chain(%arg1: [struct sl_plug_options *]opt):
		goto L1

	L1:
		[struct cl_code_listener *]chain := cl_chain_create()
		[bool]%r1 := ([struct cl_code_listener *]chain == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		[struct cl_code_listener *]%r2 := NULL
		goto L4

	L3:
		[int]verbose.31 := [int]verbose
data/slplug.c:1320:9: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r1 := ([int]%r3 != 0)
		if ([bool]%r1)
			goto L5
		else
			goto L6

	L5:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[bool]%r4 := sl_append_listener([struct cl_code_listener *]chain, "listener="locator"")
		[bool]%r5 := ![bool]%r4
		[bool]%r1 := ([bool]%r5 != false)
		if ([bool]%r1)
			goto L7
		else
			goto L6

	L7:
		[struct cl_code_listener *]%r2 := NULL
		goto L4

	L6:
		[bool]%r6 := [bool]opt->use_pp
		[bool]%r1 := ([bool]%r6 != false)
		if ([bool]%r1)
			goto L8
		else
			goto L9

	L8:
		[char *]%r7 := [char *]opt->pp_out_file
		[bool]%r1 := ([char *]%r7 != NULL)
		if ([bool]%r1)
			goto L10
		else
			goto L11

	L10:
		[char *]iftmp.32 := [char *]opt->pp_out_file
		goto L12

	L11:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[char *]iftmp.32 := ""
		goto L12

	L12:
		[char *]out := [char *]iftmp.32
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[bool]%r8 := sl_append_listener([struct cl_code_listener *]chain, "listener="pp" listener_args="%s" cld="unify_labels_fnc,unify_regs"", [char *]out)
		[bool]%r9 := ![bool]%r8
		[bool]%r1 := ([bool]%r9 != false)
		if ([bool]%r1)
			goto L13
		else
			goto L9

	L13:
		[struct cl_code_listener *]%r2 := NULL
		goto L4

	L9:
		[bool]%r10 := [bool]opt->use_dotgen
		[bool]%r1 := ([bool]%r10 != false)
		if ([bool]%r1)
			goto L14
		else
			goto L15

	L14:
		[char *]%r11 := [char *]opt->gl_dot_file
		[bool]%r1 := ([char *]%r11 != NULL)
		if ([bool]%r1)
			goto L16
		else
			goto L17

	L16:
		[char *]iftmp.33 := [char *]opt->gl_dot_file
		goto L18

	L17:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[char *]iftmp.33 := ""
		goto L18

	L18:
		[char *]gl_dot := [char *]iftmp.33
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[bool]%r12 := sl_append_listener([struct cl_code_listener *]chain, "listener="dotgen" listener_args="%s" cld="unify_labels_fnc,unify_regs"", [char *]gl_dot)
		[bool]%r13 := ![bool]%r12
		[bool]%r1 := ([bool]%r13 != false)
		if ([bool]%r1)
			goto L19
		else
			goto L15

	L19:
		[struct cl_code_listener *]%r2 := NULL
		goto L4

	L15:
		[struct cl_code_listener *]%r2 := [struct cl_code_listener *]chain
		goto L4

	L4:
		ret [struct cl_code_listener *]%r2
data/slplug.c:1320:8: error: uninitialized register %r3

data/slplug.c:1320:8: error: uninitialized register %r3
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-sl_append_listener.dot' [internal location]
static sl_append_listener(%arg1: [struct cl_code_listener *]chain, %arg2: [char *]fmt):
		goto L1

	L1:
		__builtin_va_start([struct __va_list_tag *]&ap[0], 0)
		[int]rv := vasprintf([char **]&config_string, [char *]fmt, [struct __va_list_tag *]&ap[0])
		[bool]%r1 := ([int]rv <= 0)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 1296, [char *]&__FUNCTION__[0])
		abort

	L3:
		__builtin_va_end([struct __va_list_tag *]&ap[0])
		[char *]config_string.34 := [char *]config_string
		[struct cl_code_listener *]cl := cl_code_listener_create([char *]config_string.34)
		[char *]config_string.35 := [char *]config_string
		free([char *]config_string.35)
		[bool]%r1 := ([struct cl_code_listener *]cl == NULL)
		if ([bool]%r1)
			goto L4
		else
			goto L5

	L4:
		[fnc *]%r2 := [fnc *]chain->destroy
		[fnc *]%r2([struct cl_code_listener *]chain)
		[bool]%r3 := false
		goto L6

	L5:
		cl_chain_append([struct cl_code_listener *]chain, [struct cl_code_listener *]cl)
		[bool]%r3 := true
		goto L6

	L6:
		ret [bool]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-slplug_init.dot' [internal location]
static slplug_init(%arg1: [struct plugin_name_args *]info, %arg2: [struct sl_plug_options *]opt):
		goto L1

	L1:
		memset([struct sl_plug_options *]opt, 0, 24)
		[char *]plugin_name.36 := [char *]info->full_name
		[char *]plugin_name := [char *]plugin_name.36
		[int]argc := [int]info->argc
		[struct plugin_argument *]argv := [struct plugin_argument *]info->argv
		[int]i := 0
		goto L2

	L3:
		[int]%r1 := [int]i
data/slplug.c:1252:50: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:1252:39: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[char *]key := [char *]arg->key
		[char *]value := [char *]arg->value
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[int]%r2 := strcmp([char *]key, "verbose")
		[bool]%r3 := ([int]%r2 == 0)
		if ([bool]%r3)
			goto L4
		else
			goto L5

	L4:
		[bool]%r3 := ([char *]value != NULL)
		if ([bool]%r3)
			goto L6
		else
			goto L7

	L6:
		[int]iftmp.37 := atoi([char *]value)
		goto L8

	L7:
		[int]iftmp.37 := (-1)
		goto L8

	L8:
		[int]verbose := [int]iftmp.37
		goto L9

	L5:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[int]%r4 := strcmp([char *]key, "version")
		[bool]%r3 := ([int]%r4 == 0)
		if ([bool]%r3)
			goto L10
		else
			goto L11

	L10:
		[char *]%r5 := [char *]sl_info.version
		[char *]%r6 := "
%s
"
		printf([char *]%r6, [char *]%r5)
		[int]%r7 := 1
		goto L12

	L11:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[int]%r8 := strcmp([char *]key, "help")
		[bool]%r3 := ([int]%r8 == 0)
		if ([bool]%r3)
			goto L13
		else
			goto L14

	L13:
		[char *]%r9 := [char *]sl_info.help
		[char *]%r10 := "
%s
"
		printf([char *]%r10, [char *]%r9)
		[int]%r7 := 1
		goto L12

	L14:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[int]%r11 := strcmp([char *]key, "dump-pp")
		[bool]%r3 := ([int]%r11 == 0)
		if ([bool]%r3)
			goto L15
		else
			goto L16

	L15:
		[bool]opt->use_pp := true
		[char *]opt->pp_out_file := [char *]value
		goto L9

	L16:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[int]%r12 := strcmp([char *]key, "gen-dot")
		[bool]%r3 := ([int]%r12 == 0)
		if ([bool]%r3)
			goto L17
		else
			goto L18

	L17:
		[bool]opt->use_dotgen := true
		[char *]opt->gl_dot_file := [char *]value
		goto L9

	L18:
		[char *]%r13 := "%s:%d: warning: '%s' not handled in '%s' [internal location]
"
		[struct _IO_FILE *]stderr.38 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.39 := [struct _IO_FILE *]stderr.38
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.39, [char *]%r13, "data/slplug.c", 1280, [char *]key, [char *]&__FUNCTION__[0])
		[int]%r7 := 1
		goto L12

	L9:
		[int]i := ([int]i + 1)
		goto L2

	L2:
		[bool]%r3 := ([int]i < [int]argc)
		if ([bool]%r3)
			goto L3
		else
			goto L19

	L19:
		[int]%r7 := 0
		goto L12

	L12:
		ret [int]%r7
data/slplug.c:1252:50: warning: unused register %r1

data/slplug.c:1252:50: warning: unused register %r1
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-sl_regcb.dot' [internal location]
static sl_regcb(%arg1: [char *]name):
		goto L1

	L1:
		register_callback([char *]name, [enum plugin_event]0, NULL, [struct plugin_pass *]&sl_plugin_pass)
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		register_callback([char *]name, [enum plugin_event]2, cb_finish_unit, NULL)
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		register_callback([char *]name, [enum plugin_event]4, cb_finish, NULL)
		register_callback([char *]name, [enum plugin_event]5, NULL, [struct plugin_info *]&sl_info)
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		register_callback([char *]name, [enum plugin_event]11, cb_start_unit, NULL)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-cb_finish_unit.dot' [internal location]
static cb_finish_unit(%arg1: [void *]gcc_data, %arg2: [void *]user_data):
		goto L1

	L1:
		[int]verbose.40 := [int]verbose
data/slplug.c:1200:5: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r1 := [int]%r2
		[bool]%r3 := ([bool]%r1 != false)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[char *]plugin_name.41 := [char *]plugin_name
		[char *]%r4 := "%s:%d: %s: "
		[struct _IO_FILE *]stderr.42 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.43 := [struct _IO_FILE *]stderr.42
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.43, [char *]%r4, "data/slplug.c", 1200, [char *]plugin_name.41)
		[struct _IO_FILE *]stderr.44 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.45 := [struct _IO_FILE *]stderr.44
		__builtin_fwrite([char *]&__FUNCTION__[0], 1, 14, [struct FILE *]stderr.45)
		[struct _IO_FILE *]stderr.46 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.47 := [struct _IO_FILE *]stderr.46
		__builtin_fputc(10, [struct FILE *]stderr.47)
		goto L3

	L3:
		[struct cl_code_listener *]cl.48 := [struct cl_code_listener *]cl
		[fnc *]%r5 := [fnc *]cl.48->file_close
		[struct cl_code_listener *]cl.49 := [struct cl_code_listener *]cl
		[fnc *]%r5([struct cl_code_listener *]cl.49)
		ret
data/slplug.c:1200:5: error: uninitialized register %r2

data/slplug.c:1200:5: error: uninitialized register %r2
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-cb_start_unit.dot' [internal location]
static cb_start_unit(%arg1: [void *]gcc_data, %arg2: [void *]user_data):
		goto L1

	L1:
		[int]verbose.50 := [int]verbose
data/slplug.c:1191:5: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r1 := [int]%r2
		[bool]%r3 := ([bool]%r1 != false)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[char *]plugin_name.51 := [char *]plugin_name
		[char *]%r4 := "%s:%d: %s: "
		[struct _IO_FILE *]stderr.52 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.53 := [struct _IO_FILE *]stderr.52
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.53, [char *]%r4, "data/slplug.c", 1191, [char *]plugin_name.51)
		[char *]main_input_filename.54 := [char *]main_input_filename
		[char *]%r5 := "processing input file '%s'"
		[struct _IO_FILE *]stderr.55 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.56 := [struct _IO_FILE *]stderr.55
		fprintf([struct FILE *]stderr.56, [char *]%r5, [char *]main_input_filename.54)
		[struct _IO_FILE *]stderr.57 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.58 := [struct _IO_FILE *]stderr.57
		__builtin_fputc(10, [struct FILE *]stderr.58)
		goto L3

	L3:
		[struct cl_code_listener *]cl.59 := [struct cl_code_listener *]cl
		[fnc *]%r6 := [fnc *]cl.59->file_open
		[char *]main_input_filename.60 := [char *]main_input_filename
		[struct cl_code_listener *]cl.61 := [struct cl_code_listener *]cl
		[fnc *]%r6([struct cl_code_listener *]cl.61, [char *]main_input_filename.60)
		ret
data/slplug.c:1191:5: error: uninitialized register %r2

data/slplug.c:1191:5: error: uninitialized register %r2
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-cb_finish.dot' [internal location]
static cb_finish(%arg1: [void *]gcc_data, %arg2: [void *]user_data):
		goto L1

	L1:
		[int]verbose.62 := [int]verbose
data/slplug.c:1178:5: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r1 := [int]%r2
		[bool]%r3 := ([bool]%r1 != false)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[char *]plugin_name.63 := [char *]plugin_name
		[char *]%r4 := "%s:%d: %s: "
		[struct _IO_FILE *]stderr.64 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.65 := [struct _IO_FILE *]stderr.64
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.65, [char *]%r4, "data/slplug.c", 1178, [char *]plugin_name.63)
		[struct _IO_FILE *]stderr.66 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.67 := [struct _IO_FILE *]stderr.66
		__builtin_fwrite([char *]&__FUNCTION__[0], 1, 9, [struct FILE *]stderr.67)
		[struct _IO_FILE *]stderr.68 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.69 := [struct _IO_FILE *]stderr.68
		__builtin_fputc(10, [struct FILE *]stderr.69)
		goto L3

	L3:
		[struct htab *]type_db.70 := [struct htab *]type_db
		type_db_destroy([struct htab *]type_db.70)
		[struct cl_code_listener *]cl.71 := [struct cl_code_listener *]cl
		[fnc *]%r5 := [fnc *]cl.71->destroy
		[struct cl_code_listener *]cl.72 := [struct cl_code_listener *]cl
		[fnc *]%r5([struct cl_code_listener *]cl.72)
		cl_global_cleanup()
		ret
data/slplug.c:1178:5: error: uninitialized register %r2

data/slplug.c:1178:5: error: uninitialized register %r2
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-sl_pass_execute.dot' [internal location]
static sl_pass_execute():
		goto L1

	L1:
		[union tree_node *]current_function_decl.73 := [union tree_node *]current_function_decl
		[bool]%r1 := ([union tree_node *]current_function_decl.73 == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		[char *]%r2 := "%s:%d: warning: '%s' not handled in '%s' [internal location]
"
		[struct _IO_FILE *]stderr.74 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.75 := [struct _IO_FILE *]stderr.74
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.75, [char *]%r2, "data/slplug.c", 1138, "NULL == current_function_decl", [char *]&__FUNCTION__[0])
		[int]%r3 := 0
		goto L4

	L3:
		[union tree_node *]current_function_decl.76 := [union tree_node *]current_function_decl
		[int]%r4 := [int]current_function_decl.76->base.code
		[bool]%r1 := ([int]%r4 != 29)
		if ([bool]%r1)
			goto L5
		else
			goto L6

	L5:
		[char *]%r5 := "%s:%d: warning: '%s' not handled in '%s' [internal location]
"
		[struct _IO_FILE *]stderr.77 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.78 := [struct _IO_FILE *]stderr.77
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.78, [char *]%r5, "data/slplug.c", 1143, "TREE_CODE (current_function_decl)", [char *]&__FUNCTION__[0])
		[int]%r3 := 0
		goto L4

	L6:
		[union tree_node *]current_function_decl.79 := [union tree_node *]current_function_decl
		handle_fnc_decl([union tree_node *]current_function_decl.79)
		[int]%r3 := 0
		goto L4

	L4:
		ret [int]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_fnc_decl.dot' [internal location]
static handle_fnc_decl(%arg1: [union tree_node *]decl):
		goto L1

	L1:
		[union tree_node *]__t := [union tree_node *]decl
		[int]%r1 := [int]__t->base.code
		[int]%r2 := [int]%r1
data/slplug.c:1102:29: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r3 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r3 != 1)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 1102, [char *]&__FUNCTION__[0])
		abort

	L3:
		[union tree_node *]%r5 := [union tree_node *]__t
		[int]%r6 := [int]%r5->decl_minimal.locus
		read_gcc_location([struct cl_location *]&loc, [int]%r6)
		[union tree_node *]__t := [union tree_node *]decl
		[int]%r7 := [int]__t->base.code
		[int]%r8 := [int]%r7
data/slplug.c:1105:18: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r9 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r9 != 1)
		if ([bool]%r4)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 1105, [char *]&__FUNCTION__[0])
		abort

	L5:
		[union tree_node *]%r10 := [union tree_node *]__t
		[union tree_node *]ident := [union tree_node *]%r10->decl_minimal.name
		[struct cl_code_listener *]cl.80 := [struct cl_code_listener *]cl
		[fnc *]%r11 := [fnc *]cl.80->fnc_open
data/slplug.c:1108:17: warning: 'BIT_FIELD_REF' not handled
slplug.c:547: note: raised from 'handle_accessor' [internal location]
		[int]%r12 := [int]*decl
data/slplug.c:1106:17: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[enum cl_scope_e]%r13 := ([int]%r14 == 0)
		[union tree_node *]__t := [union tree_node *]ident
		[int]%r15 := [int]__t->base.code
		[bool]%r4 := ([int]%r15 != 1)
		if ([bool]%r4)
			goto L6
		else
			goto L7

	L6:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 1106, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L7:
		[union tree_node *]%r16 := [union tree_node *]__t
		[int *]%r17 := [int *]%r16->identifier.id.str
		[char *]%r18 := [int *]%r17
		[struct cl_code_listener *]cl.81 := [struct cl_code_listener *]cl
		[fnc *]%r11([struct cl_code_listener *]cl.81, [struct cl_location *]&loc, [char *]%r18, [enum cl_scope_e]%r13)
		[union tree_node *]__t := [union tree_node *]decl
		[int]%r19 := [int]__t->base.code
		[bool]%r4 := ([int]%r19 != 29)
		if ([bool]%r4)
			goto L8
		else
			goto L9

	L8:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 1112, [char *]&__FUNCTION__[0], 29, 0)
		abort

	L9:
		[union tree_node *]%r20 := [union tree_node *]__t
		[union tree_node *]args := [union tree_node *]%r20->decl_non_common.arguments
		handle_fnc_decl_arglist([union tree_node *]args)
		[union tree_node *]__t := [union tree_node *]decl
		[int]%r21 := [int]__t->base.code
		[bool]%r4 := ([int]%r21 != 29)
		if ([bool]%r4)
			goto L10
		else
			goto L11

	L10:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 1116, [char *]&__FUNCTION__[0], 29, 0)
		abort

	L11:
		[union tree_node *]%r22 := [union tree_node *]__t
		[struct function *]fnc := [struct function *]%r22->function_decl.f
		[bool]%r4 := ([struct function *]fnc == NULL)
		if ([bool]%r4)
			goto L12
		else
			goto L13

	L12:
		[char *]%r23 := "%s:%d: warning: '%s' not handled in '%s' [internal location]
"
		[struct _IO_FILE *]stderr.82 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.83 := [struct _IO_FILE *]stderr.82
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.83, [char *]%r23, "data/slplug.c", 1118, "NULL == fnc", [char *]&__FUNCTION__[0])
		goto L14

	L13:
		[struct control_flow_graph *]cfg := [struct control_flow_graph *]fnc->cfg
		[bool]%r4 := ([struct control_flow_graph *]cfg == NULL)
		if ([bool]%r4)
			goto L15
		else
			goto L16

	L15:
		[char *]%r24 := "%s:%d: warning: '%s' not handled in '%s' [internal location]
"
		[struct _IO_FILE *]stderr.84 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.85 := [struct _IO_FILE *]stderr.84
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.85, [char *]%r24, "data/slplug.c", 1123, "CFG not found", [char *]&__FUNCTION__[0])
		goto L14

	L16:
		handle_fnc_cfg([struct control_flow_graph *]cfg)
		[struct cl_code_listener *]cl.86 := [struct cl_code_listener *]cl
		[fnc *]%r25 := [fnc *]cl.86->fnc_close
		[struct cl_code_listener *]cl.87 := [struct cl_code_listener *]cl
		[fnc *]%r25([struct cl_code_listener *]cl.87)
		goto L14

	L14:
		ret
data/slplug.c:1102:29: warning: unused register %r2
data/slplug.c:1105:18: warning: unused register %r8
data/slplug.c:1108:17: warning: unused register %r12
data/slplug.c:1106:17: error: uninitialized register %r14

data/slplug.c:1102:29: warning: unused register %r2
data/slplug.c:1105:18: warning: unused register %r8
data/slplug.c:1108:17: warning: unused register %r12
data/slplug.c:1106:17: error: uninitialized register %r14
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_fnc_decl_arglist.dot' [internal location]
static handle_fnc_decl_arglist(%arg1: [union tree_node *]args):
		goto L1

	L1:
		[int]argc := 0
		goto L2

	L3:
		handle_operand([struct cl_operand *]&arg_src, [union tree_node *]args)
		[enum cl_scope_e]arg_src.scope := [enum cl_scope_e]2
		[struct cl_code_listener *]cl.88 := [struct cl_code_listener *]cl
		[fnc *]%r1 := [fnc *]cl.88->fnc_arg_decl
		[int]argc := ([int]argc + 1)
		[struct cl_code_listener *]cl.89 := [struct cl_code_listener *]cl
		[fnc *]%r1([struct cl_code_listener *]cl.89, [int]argc, [struct cl_operand *]&arg_src)
		[union tree_node *]__t := [union tree_node *]args
		[union tree_node **]%r2 := [union tree_node **]&__t->common.chain
		[union tree_node *]args := [union tree_node *]*%r2
		goto L2

	L2:
		[bool]%r3 := ([union tree_node *]args != NULL)
		if ([bool]%r3)
			goto L3
		else
			goto L4

	L4:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_fnc_cfg.dot' [internal location]
static handle_fnc_cfg(%arg1: [struct control_flow_graph *]cfg):
		goto L1

	L1:
		[struct basic_block_def *]bb := [struct basic_block_def *]cfg->x_entry_block_ptr
		[struct VEC_edge_gc **]%r1 := [struct VEC_edge_gc **]&bb->succs
		[struct edge_iterator]ei := ei_start_1([struct VEC_edge_gc **]%r1)
		[bool]%r2 := ei_cond([struct edge_iterator]ei, [struct edge_def **]&e)
		[bool]%r3 := ([bool]%r2 != false)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[struct edge_def *]e.90 := [struct edge_def *]e
		[struct basic_block_def *]%r4 := [struct basic_block_def *]e.90->dest
		[bool]%r3 := ([struct basic_block_def *]%r4 != NULL)
		if ([bool]%r3)
			goto L4
		else
			goto L3

	L4:
		[struct edge_def *]e.91 := [struct edge_def *]e
		handle_jmp_edge([struct edge_def *]e.91)
		[struct basic_block_def *]bb := [struct basic_block_def *]bb->next_bb
		goto L3

	L3:
		[bool]%r3 := ([struct basic_block_def *]bb == NULL)
		if ([bool]%r3)
			goto L5
		else
			goto L6

	L5:
		raise(5)
		goto L6

	L6:
		goto L7

	L8:
		handle_fnc_bb([struct basic_block_def *]bb)
		[struct basic_block_def *]bb := [struct basic_block_def *]bb->next_bb
		goto L7

	L7:
		[struct basic_block_def *]%r5 := [struct basic_block_def *]bb->next_bb
		[bool]%r3 := ([struct basic_block_def *]%r5 != NULL)
		if ([bool]%r3)
			goto L8
		else
			goto L9

	L9:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-ei_start_1.dot' [internal location]
static ei_start_1(%arg1: [struct VEC_edge_gc **]ev):
		goto L1

	L1:
		[int]i.index := 0
		[struct VEC_edge_gc **]i.container := [struct VEC_edge_gc **]ev
		[struct edge_iterator]%r1 := [struct edge_iterator]i
		ret [struct edge_iterator]%r1

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-ei_cond.dot' [internal location]
static ei_cond(%arg1: [struct edge_iterator]ei, %arg2: [struct edge_def **]p):
		goto L1

	L1:
		[bool]%r1 := ei_end_p([struct edge_iterator]ei)
		[bool]%r2 := ![bool]%r1
		[bool]%r3 := ([bool]%r2 != false)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[struct edge_def *]%r4 := ei_edge([struct edge_iterator]ei)
		[struct edge_def *]*p := [struct edge_def *]%r4
		[bool]%r5 := true
		goto L4

	L3:
		[struct edge_def *]*p := NULL
		[bool]%r5 := false
		goto L4

	L4:
		ret [bool]%r5

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-ei_end_p.dot' [internal location]
static ei_end_p(%arg1: [struct edge_iterator]i):
		goto L1

	L1:
		[int]%r1 := [int]i.index
		[struct VEC_edge_gc *]%r2 := ei_container([struct edge_iterator]i)
		[bool]%r3 := ([struct VEC_edge_gc *]%r2 != NULL)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[struct VEC_edge_gc *]%r4 := ei_container([struct edge_iterator]i)
		[struct VEC_edge_base *]iftmp.92 := [struct VEC_edge_base *]&%r4->base
		goto L4

	L3:
		[struct VEC_edge_base *]iftmp.92 := NULL
		goto L4

	L4:
		[int]%r5 := VEC_edge_base_length([struct VEC_edge_base *]iftmp.92)
		[bool]%r6 := ([int]%r1 == [int]%r5)
		ret [bool]%r6

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-ei_container.dot' [internal location]
static ei_container(%arg1: [struct edge_iterator]i):
		goto L1

	L1:
		[struct VEC_edge_gc **]%r1 := [struct VEC_edge_gc **]i.container
		[bool]%r2 := ([struct VEC_edge_gc **]%r1 == NULL)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/basic-block.h", 689, [char *]&__FUNCTION__[0])
		abort

	L3:
		[struct VEC_edge_gc **]%r3 := [struct VEC_edge_gc **]i.container
		[struct VEC_edge_gc *]%r4 := [struct VEC_edge_gc *]*%r3
		ret [struct VEC_edge_gc *]%r4

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-VEC_edge_base_length.dot' [internal location]
static VEC_edge_base_length(%arg1: [struct VEC_edge_base *]vec_):
		goto L1

	L1:
		[bool]%r1 := ([struct VEC_edge_base *]vec_ != NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		[int]iftmp.93 := [int]vec_->num
		goto L4

	L3:
		[int]iftmp.93 := 0
		goto L4

	L4:
		[int]%r2 := [int]iftmp.93
		ret [int]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-ei_edge.dot' [internal location]
static ei_edge(%arg1: [struct edge_iterator]i):
		goto L1

	L1:
		[int]%r1 := [int]i.index
		[struct VEC_edge_gc *]%r2 := ei_container([struct edge_iterator]i)
		[bool]%r3 := ([struct VEC_edge_gc *]%r2 != NULL)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[struct VEC_edge_gc *]%r4 := ei_container([struct edge_iterator]i)
		[struct VEC_edge_base *]iftmp.94 := [struct VEC_edge_base *]&%r4->base
		goto L4

	L3:
		[struct VEC_edge_base *]iftmp.94 := NULL
		goto L4

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[struct edge_def *]%r5 := VEC_edge_base_index([struct VEC_edge_base *]iftmp.94, [int]%r1, "../gcc/basic-block.h", 756, [char *]&__FUNCTION__[0])
		ret [struct edge_def *]%r5

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-VEC_edge_base_index.dot' [internal location]
static VEC_edge_base_index(%arg1: [struct VEC_edge_base *]vec_, %arg2: [int]ix_, %arg3: [char *]file_, %arg4: [int]line_, %arg5: [char *]function_):
		goto L1

	L1:
		[bool]%r1 := ([struct VEC_edge_base *]vec_ == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L3:
		[int]%r2 := [int]vec_->num
		[bool]%r1 := ([int]%r2 <= [int]ix_)
		if ([bool]%r1)
			goto L2
		else
			goto L4

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		vec_assert_fail("index", "VEC(edge,base)", [char *]file_, [int]line_, [char *]function_)
		abort

	L4:
../gcc/basic-block.h:150:1: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[struct edge_def *]%r3 := [struct edge_def *]vec_->vec[-1]
		ret [struct edge_def *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_fnc_bb.dot' [internal location]
static handle_fnc_bb(%arg1: [struct basic_block_def *]bb):
		goto L1

	L1:
		[int]%r1 := [int]bb->index
		[int]%r2 := [int]%r1
		[char *]label := index_to_label([int]%r2)
		[struct cl_code_listener *]cl.95 := [struct cl_code_listener *]cl
		[fnc *]%r3 := [fnc *]cl.95->bb_open
		[struct cl_code_listener *]cl.96 := [struct cl_code_listener *]cl
		[fnc *]%r3([struct cl_code_listener *]cl.96, [char *]label)
		free([char *]label)
		[struct gimple_bb_info *]gimple := [struct gimple_bb_info *]bb->il.gimple
		[bool]%r4 := ([struct gimple_bb_info *]gimple == NULL)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
		[char *]%r5 := "%s:%d: warning: '%s' not handled in '%s' [internal location]
"
		[struct _IO_FILE *]stderr.97 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.98 := [struct _IO_FILE *]stderr.97
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.98, [char *]%r5, "data/slplug.c", 1046, "gimple not found", [char *]&__FUNCTION__[0])
		raise(5)
		goto L4

	L3:
		[struct gimple_seq_d *]%r6 := [struct gimple_seq_d *]gimple->seq
		handle_bb_gimple([struct gimple_seq_d *]%r6)
		[struct VEC_edge_gc **]%r7 := [struct VEC_edge_gc **]&bb->succs
		[struct edge_iterator]ei := ei_start_1([struct VEC_edge_gc **]%r7)
		[bool]%r8 := ei_cond([struct edge_iterator]ei, [struct edge_def **]&e)
		[bool]%r4 := ([bool]%r8 != false)
		if ([bool]%r4)
			goto L5
		else
			goto L4

	L5:
		[struct edge_def *]e.99 := [struct edge_def *]e
		[struct basic_block_def *]%r9 := [struct basic_block_def *]e.99->dest
		[bool]%r4 := ([struct basic_block_def *]%r9 != NULL)
		if ([bool]%r4)
			goto L6
		else
			goto L4

	L6:
		[struct edge_def *]e.100 := [struct edge_def *]e
		[int]%r10 := [int]e.100->flags
data/slplug.c:1055:36: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r11 := [int]%r12
		[bool]%r4 := ([bool]%r11 != false)
		if ([bool]%r4)
			goto L7
		else
			goto L4

	L7:
		[struct edge_def *]e.101 := [struct edge_def *]e
		handle_jmp_edge([struct edge_def *]e.101)
		goto L4

	L4:
		ret
data/slplug.c:1055:41: warning: unused register %r10
data/slplug.c:1055:36: error: uninitialized register %r12

data/slplug.c:1055:41: warning: unused register %r10
data/slplug.c:1055:36: error: uninitialized register %r12
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_jmp_edge.dot' [internal location]
static handle_jmp_edge(%arg1: [struct edge_def *]e):
		goto L1

	L1:
		[struct basic_block_def *]next := [struct basic_block_def *]e->dest
		[enum cl_insn_e]cli.code := [enum cl_insn_e]1
		[int]%r1 := [int]next->index
		[int]%r2 := [int]%r1
		[char *]%r3 := index_to_label([int]%r2)
		[char *]cli.data.insn_jmp.label := [char *]%r3
		[char *]cli.loc.file := NULL
		[int]cli.loc.line := (-1)
		[struct cl_code_listener *]cl.102 := [struct cl_code_listener *]cl
		[fnc *]%r4 := [fnc *]cl.102->insn
		[struct cl_code_listener *]cl.103 := [struct cl_code_listener *]cl
		[fnc *]%r4([struct cl_code_listener *]cl.103, [struct cl_insn *]&cli)
		[char *]%r5 := [char *]cli.data.insn_jmp.label
		free([char *]%r5)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_bb_gimple.dot' [internal location]
static handle_bb_gimple(%arg1: [struct gimple_seq_d *]body):
		goto L1

	L1:
		memset([struct walk_stmt_info *]&info, 0, 56)
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		walk_gimple_seq([struct gimple_seq_d *]body, cb_walk_gimple_stmt, NULL, [struct walk_stmt_info *]&info)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-cb_walk_gimple_stmt.dot' [internal location]
static cb_walk_gimple_stmt(%arg1: [struct gimple_stmt_iterator *]iter, %arg2: [bool *]subtree_done, %arg3: [struct walk_stmt_info *]info):
		goto L1

	L1:
		[union gimple_statement_d *]stmt := gsi_stmt([struct gimple_stmt_iterator]*iter)
		[int]verbose.104 := [int]verbose
data/slplug.c:956:42: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]show_gimple := ([int]%r1 != 0)
		[bool]%r2 := ([bool]show_gimple != false)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
		[char *]%r3 := "
		"
		printf([char *]%r3)
		[struct _IO_FILE *]stdout.105 := [struct _IO_FILE *]stdout
		print_gimple_stmt([struct _IO_FILE *]stdout.105, [union gimple_statement_d *]stmt, 0, 128)
		goto L3

	L3:
		[int]%r4 := [int]stmt->gsbase.code
		[enum gimple_code]code := [int]%r4
		switch ([enum gimple_code]code) {
			default: goto L4
			case 1: goto L5
			case 3: goto L6
			case 4: goto L7
			case 5: goto L8
			case 6: goto L9
			case 7: goto L10
			case 8: goto L11
			case 30: goto L12
		}

	L5:
		handle_stmt_cond([union gimple_statement_d *]stmt)
		goto L13

	L8:
		handle_stmt_assign([union gimple_statement_d *]stmt)
		goto L13

	L10:
		handle_stmt_call([union gimple_statement_d *]stmt)
		goto L13

	L11:
		handle_stmt_return([union gimple_statement_d *]stmt)
		goto L13

	L7:
		handle_stmt_switch([union gimple_statement_d *]stmt)
		goto L13

	L6:
		goto L13

	L9:
		[int]%r5 := [int]stmt->gsbase.location
		[struct expanded_location]%r6 := expand_location([int]%r5)
		[int]%r7 := [int]%r6.column
		[int]%r8 := [int]stmt->gsbase.location
		[struct expanded_location]%r9 := expand_location([int]%r8)
		[int]%r10 := [int]%r9.line
		[int]%r11 := [int]stmt->gsbase.location
		[struct expanded_location]%r12 := expand_location([int]%r11)
		[char *]%r13 := [char *]%r12.file
		[char *]%r14 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.106 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.107 := [struct _IO_FILE *]stderr.106
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.107, [char *]%r14, [char *]%r13, [int]%r10, [int]%r7, "GIMPLE_ASM", "data/slplug.c", 995, [char *]&__FUNCTION__[0])
		goto L13

	L12:
		[int]%r15 := [int]stmt->gsbase.location
		[struct expanded_location]%r16 := expand_location([int]%r15)
		[int]%r17 := [int]%r16.column
		[int]%r18 := [int]stmt->gsbase.location
		[struct expanded_location]%r19 := expand_location([int]%r18)
		[int]%r20 := [int]%r19.line
		[int]%r21 := [int]stmt->gsbase.location
		[struct expanded_location]%r22 := expand_location([int]%r21)
		[char *]%r23 := [char *]%r22.file
		[char *]%r24 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.108 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.109 := [struct _IO_FILE *]stderr.108
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.109, [char *]%r24, [char *]%r23, [int]%r20, [int]%r17, "GIMPLE_PREDICT", "data/slplug.c", 999, [char *]&__FUNCTION__[0])
		goto L13

	L4:
		raise(5)
		goto L13

	L13:
		[bool]%r2 := ([bool]show_gimple != false)
		if ([bool]%r2)
			goto L14
		else
			goto L15

	L14:
		__builtin_putchar(10)
		goto L15

	L15:
		[union tree_node *]%r25 := NULL
		ret [union tree_node *]%r25
data/slplug.c:956:10: error: uninitialized register %r1

data/slplug.c:956:10: error: uninitialized register %r1
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gsi_stmt.dot' [internal location]
static gsi_stmt(%arg1: [struct gimple_stmt_iterator]i):
		goto L1

	L1:
		[struct gimple_seq_node_d *]%r1 := [struct gimple_seq_node_d *]i.ptr
		[union gimple_statement_d *]%r2 := [union gimple_statement_d *]%r1->stmt
		ret [union gimple_statement_d *]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_switch.dot' [internal location]
static handle_stmt_switch(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[union tree_node *]%r1 := gimple_switch_index([union gimple_statement_d *]stmt)
		handle_operand([struct cl_operand *]&src, [union tree_node *]%r1)
		read_gimple_location([struct cl_location *]&loc, [union gimple_statement_d *]stmt)
		[struct cl_code_listener *]cl.110 := [struct cl_code_listener *]cl
		[fnc *]%r2 := [fnc *]cl.110->insn_switch_open
		[struct cl_code_listener *]cl.111 := [struct cl_code_listener *]cl
		[fnc *]%r2([struct cl_code_listener *]cl.111, [struct cl_location *]&loc, [struct cl_operand *]&src)
		free_cl_operand_data([struct cl_operand *]&src)
		[int]i := 0
		goto L2

	L3:
		[union tree_node *]case_decl := gimple_switch_label([union gimple_statement_d *]stmt, [int]i)
		[bool]%r3 := ([union tree_node *]case_decl == NULL)
		if ([bool]%r3)
			goto L4
		else
			goto L5

	L4:
		raise(5)
		goto L5

	L5:
		[union tree_node *]__t := [union tree_node *]case_decl
		[int]%r4 := [int]__t->base.code
		[bool]%r3 := ([int]%r4 != 139)
		if ([bool]%r3)
			goto L6
		else
			goto L7

	L6:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 916, [char *]&__FUNCTION__[0], 139, 0)
		abort

	L7:
		[union tree_node *]__t.112 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.112
		[int]%r5 := [int]__t->base.code
		[int]%r6 := [int]%r5
data/slplug.c:916:25: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r7 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r7
		[bool]%r3 := ([char]__c <= 3)
		if ([bool]%r3)
			goto L8
		else
			goto L9

	L9:
		[bool]%r3 := ([char]__c > 10)
		if ([bool]%r3)
			goto L8
		else
			goto L10

	L8:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 916, [char *]&__FUNCTION__[0])
		abort

	L10:
		[union tree_node *]__t.113 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.113
		[int]__i := 0
		[bool]%r3 := ([int]__i < 0)
		if ([bool]%r3)
			goto L11
		else
			goto L12

	L12:
		[int]%r8 := tree_operand_length([union tree_node *]__t)
		[bool]%r3 := ([int]%r8 <= [int]__i)
		if ([bool]%r3)
			goto L11
		else
			goto L13

	L11:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 916, [char *]&__FUNCTION__[0])
		abort

	L13:
		[union tree_node *[]*]%r9 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r10 := [union tree_node **]&*%r9[0]
		[int]%r11 := [int]__i
data/slplug.c:916:25: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:916:25: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]case_low := [union tree_node *]*%r12
		handle_operand([struct cl_operand *]&val_lo, [union tree_node *]case_low)
		[union tree_node *]__t := [union tree_node *]case_decl
		[int]%r13 := [int]__t->base.code
		[bool]%r3 := ([int]%r13 != 139)
		if ([bool]%r3)
			goto L14
		else
			goto L15

	L14:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 921, [char *]&__FUNCTION__[0], 139, 0)
		abort

	L15:
		[union tree_node *]__t.114 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.114
		[int]%r14 := [int]__t->base.code
		[int]%r15 := [int]%r14
data/slplug.c:921:26: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r16 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r16
		[bool]%r3 := ([char]__c <= 3)
		if ([bool]%r3)
			goto L16
		else
			goto L17

	L17:
		[bool]%r3 := ([char]__c > 10)
		if ([bool]%r3)
			goto L16
		else
			goto L18

	L16:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 921, [char *]&__FUNCTION__[0])
		abort

	L18:
		[union tree_node *]__t.115 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.115
		[int]__i := 1
		[bool]%r3 := ([int]__i < 0)
		if ([bool]%r3)
			goto L19
		else
			goto L20

	L20:
		[int]%r17 := tree_operand_length([union tree_node *]__t)
		[bool]%r3 := ([int]%r17 <= [int]__i)
		if ([bool]%r3)
			goto L19
		else
			goto L21

	L19:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 921, [char *]&__FUNCTION__[0])
		abort

	L21:
		[union tree_node *[]*]%r18 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r19 := [union tree_node **]&*%r18[0]
		[int]%r20 := [int]__i
data/slplug.c:921:26: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:921:26: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]case_high := [union tree_node *]*%r21
		[bool]%r3 := ([union tree_node *]case_high == NULL)
		if ([bool]%r3)
			goto L22
		else
			goto L23

	L22:
		[union tree_node *]case_high := [union tree_node *]case_low
		goto L23

	L23:
		handle_operand([struct cl_operand *]&val_hi, [union tree_node *]case_high)
		[union tree_node *]__t := [union tree_node *]case_decl
		[int]%r22 := [int]__t->base.code
		[bool]%r3 := ([int]%r22 != 139)
		if ([bool]%r3)
			goto L24
		else
			goto L25

	L24:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 928, [char *]&__FUNCTION__[0], 139, 0)
		abort

	L25:
		[union tree_node *]__t.116 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.116
		[int]%r23 := [int]__t->base.code
		[int]%r24 := [int]%r23
data/slplug.c:928:27: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r25 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r25
		[bool]%r3 := ([char]__c <= 3)
		if ([bool]%r3)
			goto L26
		else
			goto L27

	L27:
		[bool]%r3 := ([char]__c > 10)
		if ([bool]%r3)
			goto L26
		else
			goto L28

	L26:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 928, [char *]&__FUNCTION__[0])
		abort

	L28:
		[union tree_node *]__t.117 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.117
		[int]__i := 2
		[bool]%r3 := ([int]__i < 0)
		if ([bool]%r3)
			goto L29
		else
			goto L30

	L30:
		[int]%r26 := tree_operand_length([union tree_node *]__t)
		[bool]%r3 := ([int]%r26 <= [int]__i)
		if ([bool]%r3)
			goto L29
		else
			goto L31

	L29:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 928, [char *]&__FUNCTION__[0])
		abort

	L31:
		[union tree_node *[]*]%r27 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r28 := [union tree_node **]&*%r27[0]
		[int]%r29 := [int]__i
data/slplug.c:928:27: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:928:27: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]case_label := [union tree_node *]*%r30
		[bool]%r3 := ([union tree_node *]case_label == NULL)
		if ([bool]%r3)
			goto L32
		else
			goto L33

	L33:
		[int]%r31 := [int]case_label->base.code
		[bool]%r3 := ([int]%r31 != 30)
		if ([bool]%r3)
			goto L32
		else
			goto L34

	L32:
		raise(5)
		goto L34

	L34:
		[union tree_node *]__t := [union tree_node *]case_label
		[int]%r32 := [int]__t->base.code
		[bool]%r3 := ([int]%r32 != 30)
		if ([bool]%r3)
			goto L35
		else
			goto L36

	L35:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 933, [char *]&__FUNCTION__[0], 30, 0)
		abort

	L36:
		[union tree_node *]%r33 := [union tree_node *]__t
		[int]case_label_uid := [int]%r33->decl_common.label_decl_uid
		[int]label_uid := find_case_label_target([union gimple_statement_d *]stmt, [int]case_label_uid)
		[char *]label := index_to_label([int]label_uid)
		[int]%r34 := [int]case_decl->base.code
		[int]%r35 := [int]%r34
data/slplug.c:938:33: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r36 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r36 > 3)
		if ([bool]%r3)
			goto L37
		else
			goto L38

	L37:
		[int]%r37 := [int]case_decl->base.code
		[int]%r38 := [int]%r37
data/slplug.c:938:33: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r39 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r39 <= 10)
		if ([bool]%r3)
			goto L39
		else
			goto L38

	L39:
		[int]iftmp.118 := [int]case_decl->exp.locus
		goto L40

	L38:
		[int]iftmp.118 := 0
		goto L40

	L40:
		read_gcc_location([struct cl_location *]&loc, [int]iftmp.118)
		[struct cl_code_listener *]cl.119 := [struct cl_code_listener *]cl
		[fnc *]%r40 := [fnc *]cl.119->insn_switch_case
		[struct cl_code_listener *]cl.120 := [struct cl_code_listener *]cl
		[fnc *]%r40([struct cl_code_listener *]cl.120, [struct cl_location *]&loc, [struct cl_operand *]&val_lo, [struct cl_operand *]&val_hi, [char *]label)
		free_cl_operand_data([struct cl_operand *]&val_lo)
		free_cl_operand_data([struct cl_operand *]&val_hi)
		free([char *]label)
		[int]i := ([int]i + 1)
		goto L2

	L2:
		[int]%r41 := gimple_switch_num_labels([union gimple_statement_d *]stmt)
		[bool]%r3 := ([int]%r41 > [int]i)
		if ([bool]%r3)
			goto L3
		else
			goto L41

	L41:
		[struct cl_code_listener *]cl.121 := [struct cl_code_listener *]cl
		[fnc *]%r42 := [fnc *]cl.121->insn_switch_close
		[struct cl_code_listener *]cl.122 := [struct cl_code_listener *]cl
		[fnc *]%r42([struct cl_code_listener *]cl.122)
		ret
data/slplug.c:916:25: warning: unused register %r6
data/slplug.c:916:25: warning: unused register %r10
data/slplug.c:916:25: warning: unused register %r11
data/slplug.c:916:14: error: uninitialized register %r12
data/slplug.c:921:26: warning: unused register %r15
data/slplug.c:921:26: warning: unused register %r19
data/slplug.c:921:26: warning: unused register %r20
data/slplug.c:921:14: error: uninitialized register %r21
data/slplug.c:928:27: warning: unused register %r24
data/slplug.c:928:27: warning: unused register %r28
data/slplug.c:928:27: warning: unused register %r29
data/slplug.c:928:14: error: uninitialized register %r30
data/slplug.c:938:33: warning: unused register %r35
data/slplug.c:938:33: warning: unused register %r38

data/slplug.c:916:25: warning: unused register %r6
data/slplug.c:916:25: warning: unused register %r10
data/slplug.c:916:25: warning: unused register %r11
data/slplug.c:916:14: error: uninitialized register %r12
data/slplug.c:921:26: warning: unused register %r15
data/slplug.c:921:26: warning: unused register %r19
data/slplug.c:921:26: warning: unused register %r20
data/slplug.c:921:14: error: uninitialized register %r21
data/slplug.c:928:27: warning: unused register %r24
data/slplug.c:928:27: warning: unused register %r28
data/slplug.c:928:27: warning: unused register %r29
data/slplug.c:928:14: error: uninitialized register %r30
data/slplug.c:938:33: warning: unused register %r35
data/slplug.c:938:33: warning: unused register %r38
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_switch_index.dot' [internal location]
static gimple_switch_index(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 4)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 3097, [char *]&__FUNCTION__[0], [enum gimple_code]4, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 0)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_code.dot' [internal location]
static gimple_code(%arg1: [union gimple_statement_d *]g):
		goto L1

	L1:
		[int]%r1 := [int]g->gsbase.code
		[enum gimple_code]%r2 := [int]%r1
		ret [enum gimple_code]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_op.dot' [internal location]
static gimple_op(%arg1: [union gimple_statement_d *]gs, %arg2: [int]i):
		goto L1

	L1:
		[bool]%r1 := gimple_has_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ([bool]%r1 != false)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
		[int]%r3 := gimple_num_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ([int]%r3 <= [int]i)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/gimple.h", 1566, [char *]&__FUNCTION__[0])
		abort

	L5:
(null):0:0: warning: 'CONSTRUCTOR' not handled
slplug.c:431: note: raised from 'read_raw_operand' [internal location]
		[union <anon_type>]%r4 := CL_OPERAND_VOID
		[union gimple_statement_d *]%r4._q := [union gimple_statement_d *]gs
		[union gimple_statement_d *]%r5 := [union gimple_statement_d *]%r4._nq
		[union tree_node **]%r6 := gimple_ops([union gimple_statement_d *]%r5)
		[int]%r7 := [int]i
../gcc/gimple.h:1567:49: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
../gcc/gimple.h:1567:49: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]%r8 := [union tree_node *]*%r9
		goto L6

	L3:
		[union tree_node *]%r8 := NULL
		goto L6

	L6:
		ret [union tree_node *]%r8
../gcc/gimple.h:1567:25: warning: unused register %r6
../gcc/gimple.h:1567:49: warning: unused register %r7
../gcc/gimple.h:1567:7: error: uninitialized register %r9

../gcc/gimple.h:1567:25: warning: unused register %r6
../gcc/gimple.h:1567:49: warning: unused register %r7
../gcc/gimple.h:1567:7: error: uninitialized register %r9
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_has_ops.dot' [internal location]
static gimple_has_ops(%arg1: [union gimple_statement_d *]g):
		goto L1

	L1:
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]g)
		[bool]%r2 := ([enum gimple_code]%r1 != 0)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
		[enum gimple_code]%r3 := gimple_code([union gimple_statement_d *]g)
		[bool]%r2 := ([enum gimple_code]%r3 <= 8)
		if ([bool]%r2)
			goto L4
		else
			goto L3

	L4:
		[int]iftmp.123 := 1
		goto L5

	L3:
		[int]iftmp.123 := 0
		goto L5

	L5:
		[bool]%r4 := [int]iftmp.123
		ret [bool]%r4

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_num_ops.dot' [internal location]
static gimple_num_ops(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[int]%r1 := [int]gs->gsbase.num_ops
		ret [int]%r1

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_ops.dot' [internal location]
static gimple_ops(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[bool]%r1 := gimple_has_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ![bool]%r1
		[bool]%r3 := ([bool]%r2 != false)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[union tree_node **]%r4 := NULL
		goto L4

	L3:
		[union tree_node **]gs.124 := [union gimple_statement_d *]gs
		[enum gimple_code]%r5 := gimple_code([union gimple_statement_d *]gs)
../gcc/gimple.h:1555:53: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r6 := [int]gimple_ops_offset_[-1]
../gcc/gimple.h:1555:3: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		goto L4

	L4:
		ret [union tree_node **]%r4
../gcc/gimple.h:1555:66: warning: unused register %r5
../gcc/gimple.h:1555:53: warning: unused register %r6

../gcc/gimple.h:1555:66: warning: unused register %r5
../gcc/gimple.h:1555:53: warning: unused register %r6
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_switch_label.dot' [internal location]
static gimple_switch_label(%arg1: [union gimple_statement_d *]gs, %arg2: [int]index):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 4)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 3129, [char *]&__FUNCTION__[0], [enum gimple_code]4, [enum tree_code]0)
		abort

	L3:
		[int]%r3 := gimple_num_ops([union gimple_statement_d *]gs)
		[int]%r4 := ([int]index + 1)
		[bool]%r2 := ([int]%r3 <= [int]%r4)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/gimple.h", 3130, [char *]&__FUNCTION__[0])
		abort

	L5:
		[int]%r5 := ([int]index + 1)
		[union tree_node *]%r6 := gimple_op([union gimple_statement_d *]gs, [int]%r5)
		ret [union tree_node *]%r6

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-tree_operand_length.dot' [internal location]
static tree_operand_length(%arg1: [union tree_node *]node):
		goto L1

	L1:
		[int]%r1 := [int]node->base.code
		[int]%r2 := [int]%r1
../gcc/tree.h:5242:7: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r3 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r4 := ([enum tree_code_class]%r3 == 9)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
		[union tree_node *]__t := [union tree_node *]node
		[int]%r5 := [int]__t->base.code
		[int]%r6 := [int]%r5
../gcc/tree.h:5243:12: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r7 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r4 := ([enum tree_code_class]%r7 != 9)
		if ([bool]%r4)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]9, "../gcc/tree.h", 5243, [char *]&__FUNCTION__[0])
		abort

	L5:
		[union tree_node *]%r8 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]%r8->exp.operands[0]
		[int]%r9 := [int]__t->base.code
		[bool]%r4 := ([int]%r9 != 23)
		if ([bool]%r4)
			goto L6
		else
			goto L7

	L6:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "../gcc/tree.h", 5243, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L7:
		[union tree_node *]%r10 := [union tree_node *]__t
		[int]%r11 := [int]%r10->int_cst.int_cst.low
		[int]%r12 := [int]%r11
		goto L8

	L3:
		[int]%r13 := [int]node->base.code
		[int]%r14 := [int]%r13
../gcc/tree.h:5245:12: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r15 := [int]tree_code_length[-1]
		[int]%r12 := [int]%r15
		goto L8

	L8:
		ret [int]%r12
../gcc/tree.h:5242:7: warning: unused register %r2
../gcc/tree.h:5243:12: warning: unused register %r6
../gcc/tree.h:5245:12: warning: unused register %r14

../gcc/tree.h:5242:7: warning: unused register %r2
../gcc/tree.h:5243:12: warning: unused register %r6
../gcc/tree.h:5245:12: warning: unused register %r14
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_switch_num_labels.dot' [internal location]
static gimple_switch_num_labels(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 4)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 3075, [char *]&__FUNCTION__[0], [enum gimple_code]4, [enum tree_code]0)
		abort

	L3:
		[int]num_ops := gimple_num_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ([int]num_ops <= 1)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/gimple.h", 3077, [char *]&__FUNCTION__[0])
		abort

	L5:
		[int]%r3 := ([int]num_ops + (-1))
		ret [int]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-find_case_label_target.dot' [internal location]
static find_case_label_target(%arg1: [union gimple_statement_d *]stmt, %arg2: [int]label_decl_uid):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]stmt
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 4)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "data/slplug.c", 858, [char *]&__FUNCTION__[0], [enum gimple_code]4, [enum tree_code]0)
		abort

	L3:
		[struct basic_block_def *]switch_bb := [struct basic_block_def *]stmt->gsbase.bb
		[struct VEC_edge_gc **]%r3 := [struct VEC_edge_gc **]&switch_bb->succs
		[struct edge_iterator]ei := ei_start_1([struct VEC_edge_gc **]%r3)
		goto L4

	L5:
		[struct edge_def *]e.125 := [struct edge_def *]e
		[struct basic_block_def *]bb := [struct basic_block_def *]e.125->dest
		[bool]%r2 := ([struct basic_block_def *]bb == NULL)
		if ([bool]%r2)
			goto L6
		else
			goto L7

	L6:
		raise(5)
		goto L7

	L7:
		[struct gimple_bb_info *]bb_info := [struct gimple_bb_info *]bb->il.gimple
		[bool]%r2 := ([struct gimple_bb_info *]bb_info == NULL)
		if ([bool]%r2)
			goto L8
		else
			goto L9

	L9:
		[struct gimple_seq_d *]%r4 := [struct gimple_seq_d *]bb_info->seq
		[bool]%r2 := ([struct gimple_seq_d *]%r4 == NULL)
		if ([bool]%r2)
			goto L8
		else
			goto L10

	L10:
		[struct gimple_seq_d *]%r5 := [struct gimple_seq_d *]bb_info->seq
		[struct gimple_seq_node_d *]%r6 := [struct gimple_seq_node_d *]%r5->first
		[bool]%r2 := ([struct gimple_seq_node_d *]%r6 == NULL)
		if ([bool]%r2)
			goto L8
		else
			goto L11

	L8:
		raise(5)
		goto L11

	L11:
		[struct gimple_seq_d *]%r7 := [struct gimple_seq_d *]bb_info->seq
		[struct gimple_seq_node_d *]%r8 := [struct gimple_seq_node_d *]%r7->first
		[union gimple_statement_d *]bb_stmt := [union gimple_statement_d *]%r8->stmt
		[bool]%r2 := ([union gimple_statement_d *]bb_stmt == NULL)
		if ([bool]%r2)
			goto L12
		else
			goto L13

	L13:
		[int]%r9 := [int]bb_stmt->gsbase.code
		[bool]%r2 := ([int]%r9 != 3)
		if ([bool]%r2)
			goto L12
		else
			goto L14

	L12:
data/slplug.c:879:13: warning: 'GIMPLE_PREDICT' not handled
slplug.c:999: note: raised from 'cb_walk_gimple_stmt' [internal location]
		goto L15

	L14:
		[union tree_node *]label := gimple_label_label([union gimple_statement_d *]bb_stmt)
		[bool]%r2 := ([union tree_node *]label == NULL)
		if ([bool]%r2)
			goto L16
		else
			goto L17

	L16:
		raise(5)
		goto L17

	L17:
		[union tree_node *]__t := [union tree_node *]label
		[int]%r10 := [int]__t->base.code
		[bool]%r2 := ([int]%r10 != 30)
		if ([bool]%r2)
			goto L18
		else
			goto L19

	L18:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 886, [char *]&__FUNCTION__[0], 30, 0)
		abort

	L19:
		[union tree_node *]%r11 := [union tree_node *]__t
		[int]%r12 := [int]%r11->decl_common.label_decl_uid
		[bool]%r2 := ([int]%r12 == [int]label_decl_uid)
		if ([bool]%r2)
			goto L20
		else
			goto L15

	L20:
		[int]%r13 := [int]bb->index
		[int]%r14 := [int]%r13
		goto L21

	L15:
		ei_next([struct edge_iterator *]&ei)
		goto L4

	L4:
		[bool]%r15 := ei_cond([struct edge_iterator]ei, [struct edge_def **]&e)
		[bool]%r2 := ([bool]%r15 != false)
		if ([bool]%r2)
			goto L5
		else
			goto L22

	L22:
		raise(5)
		[int]%r14 := (-1)
		goto L21

	L21:
		ret [int]%r14

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_label_label.dot' [internal location]
static gimple_label_label(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 3)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 2403, [char *]&__FUNCTION__[0], [enum gimple_code]3, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 0)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-ei_next.dot' [internal location]
static ei_next(%arg1: [struct edge_iterator *]i):
		goto L1

	L1:
		[int]%r1 := [int]i->index
		[struct VEC_edge_gc *]%r2 := ei_container([struct edge_iterator]*i)
		[bool]%r3 := ([struct VEC_edge_gc *]%r2 != NULL)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		[struct VEC_edge_gc *]%r4 := ei_container([struct edge_iterator]*i)
		[struct VEC_edge_base *]iftmp.126 := [struct VEC_edge_base *]&%r4->base
		goto L4

	L3:
		[struct VEC_edge_base *]iftmp.126 := NULL
		goto L4

	L4:
		[int]%r5 := VEC_edge_base_length([struct VEC_edge_base *]iftmp.126)
		[bool]%r3 := ([int]%r1 >= [int]%r5)
		if ([bool]%r3)
			goto L5
		else
			goto L6

	L5:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/basic-block.h", 740, [char *]&__FUNCTION__[0])
		abort

	L6:
		[int]%r6 := [int]i->index
		[int]%r7 := ([int]%r6 + 1)
		[int]i->index := [int]%r7
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_cond.dot' [internal location]
static handle_stmt_cond(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]stmt
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 1)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "data/slplug.c", 823, [char *]&__FUNCTION__[0], [enum gimple_code]1, [enum tree_code]0)
		abort

	L3:
		[char *]label_true := NULL
		[char *]label_false := NULL
		[struct basic_block_def *]bb := [struct basic_block_def *]stmt->gsbase.bb
		[struct VEC_edge_gc **]%r3 := [struct VEC_edge_gc **]&bb->succs
		[struct edge_iterator]ei := ei_start_1([struct VEC_edge_gc **]%r3)
		goto L4

	L5:
		[struct edge_def *]e.127 := [struct edge_def *]e
		[int]%r4 := [int]e.127->flags
data/slplug.c:832:13: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r2 := ([int]%r5 != 0)
		if ([bool]%r2)
			goto L6
		else
			goto L7

	L6:
		[struct edge_def *]e.128 := [struct edge_def *]e
		[struct basic_block_def *]next := [struct basic_block_def *]e.128->dest
		[int]%r6 := [int]next->index
		[int]%r7 := [int]%r6
		[char *]label_true := index_to_label([int]%r7)
		goto L7

	L7:
		[struct edge_def *]e.129 := [struct edge_def *]e
		[int]%r8 := [int]e.129->flags
data/slplug.c:836:13: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r2 := ([int]%r9 != 0)
		if ([bool]%r2)
			goto L8
		else
			goto L9

	L8:
		[struct edge_def *]e.130 := [struct edge_def *]e
		[struct basic_block_def *]next := [struct basic_block_def *]e.130->dest
		[int]%r10 := [int]next->index
		[int]%r11 := [int]%r10
		[char *]label_false := index_to_label([int]%r11)
		goto L9

	L9:
		ei_next([struct edge_iterator *]&ei)
		goto L4

	L4:
		[bool]%r12 := ei_cond([struct edge_iterator]ei, [struct edge_def **]&e)
		[bool]%r2 := ([bool]%r12 != false)
		if ([bool]%r2)
			goto L5
		else
			goto L10

	L10:
		[bool]%r2 := ([char *]label_true == NULL)
		if ([bool]%r2)
			goto L11
		else
			goto L12

	L12:
		[bool]%r2 := ([char *]label_false == NULL)
		if ([bool]%r2)
			goto L11
		else
			goto L13

	L11:
		raise(5)
		goto L13

	L13:
		[union tree_node *]%r13 := gimple_cond_rhs([union gimple_statement_d *]stmt)
		[union tree_node *]%r14 := gimple_cond_lhs([union gimple_statement_d *]stmt)
		[enum tree_code]%r15 := gimple_cond_code([union gimple_statement_d *]stmt)
		handle_stmt_binop([union gimple_statement_d *]stmt, [enum tree_code]%r15, [struct cl_operand *]&stmt_cond_fixed_reg, [union tree_node *]%r14, [union tree_node *]%r13)
		handle_stmt_cond_br([union gimple_statement_d *]stmt, [char *]label_true, [char *]label_false)
		free([char *]label_true)
		free([char *]label_false)
		ret
data/slplug.c:832:14: warning: unused register %r4
data/slplug.c:832:12: error: uninitialized register %r5
data/slplug.c:836:14: warning: unused register %r8
data/slplug.c:836:12: error: uninitialized register %r9

data/slplug.c:832:14: warning: unused register %r4
data/slplug.c:832:12: error: uninitialized register %r5
data/slplug.c:836:14: warning: unused register %r8
data/slplug.c:836:12: error: uninitialized register %r9
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_cond_rhs.dot' [internal location]
static gimple_cond_rhs(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 1)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 2227, [char *]&__FUNCTION__[0], [enum gimple_code]1, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 1)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_cond_lhs.dot' [internal location]
static gimple_cond_lhs(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 1)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 2196, [char *]&__FUNCTION__[0], [enum gimple_code]1, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 0)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_cond_code.dot' [internal location]
static gimple_cond_code(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 1)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 2175, [char *]&__FUNCTION__[0], [enum gimple_code]1, [enum tree_code]0)
		abort

	L3:
		[int]%r3 := [int]gs->gsbase.subcode
		[enum tree_code]%r4 := [int]%r3
		ret [enum tree_code]%r4

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_cond_br.dot' [internal location]
static handle_stmt_cond_br(%arg1: [union gimple_statement_d *]stmt, %arg2: [char *]then_label, %arg3: [char *]else_label):
		goto L1

	L1:
		[enum cl_insn_e]cli.code := [enum cl_insn_e]2
		[struct cl_operand *]cli.data.insn_cond.src := [struct cl_operand *]&stmt_cond_fixed_reg
		[char *]cli.data.insn_cond.then_label := [char *]then_label
		[char *]cli.data.insn_cond.else_label := [char *]else_label
		read_gimple_location([struct cl_location *]&cli.loc, [union gimple_statement_d *]stmt)
		[struct cl_code_listener *]cl.131 := [struct cl_code_listener *]cl
		[fnc *]%r1 := [fnc *]cl.131->insn
		[struct cl_code_listener *]cl.132 := [struct cl_code_listener *]cl
		[fnc *]%r1([struct cl_code_listener *]cl.132, [struct cl_insn *]&cli)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_return.dot' [internal location]
static handle_stmt_return(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[union tree_node *]%r1 := gimple_return_retval([union gimple_statement_d *]stmt)
		handle_operand([struct cl_operand *]&src, [union tree_node *]%r1)
		[enum cl_insn_e]cli.code := [enum cl_insn_e]3
		[struct cl_operand *]cli.data.insn_ret.src := [struct cl_operand *]&src
		read_gimple_location([struct cl_location *]&cli.loc, [union gimple_statement_d *]stmt)
		[struct cl_code_listener *]cl.133 := [struct cl_code_listener *]cl
		[fnc *]%r2 := [fnc *]cl.133->insn
		[struct cl_code_listener *]cl.134 := [struct cl_code_listener *]cl
		[fnc *]%r2([struct cl_code_listener *]cl.134, [struct cl_insn *]&cli)
		free_cl_operand_data([struct cl_operand *]&src)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_return_retval.dot' [internal location]
static gimple_return_retval(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 8)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 4058, [char *]&__FUNCTION__[0], [enum gimple_code]8, [enum tree_code]0)
		abort

	L3:
		[int]%r3 := gimple_num_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ([int]%r3 != 1)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/gimple.h", 4059, [char *]&__FUNCTION__[0])
		abort

	L5:
		[union tree_node *]%r4 := gimple_op([union gimple_statement_d *]gs, 0)
		ret [union tree_node *]%r4

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_call.dot' [internal location]
static handle_stmt_call(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[union tree_node *]op0 := gimple_call_fn([union gimple_statement_d *]stmt)
		[int]%r1 := [int]op0->base.code
		[bool]%r2 := ([int]%r1 == 121)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
		[union tree_node *]__t := [union tree_node *]op0
		[int]%r3 := [int]__t->base.code
		[int]%r4 := [int]%r3
data/slplug.c:730:15: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r5 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r5
		[bool]%r2 := ([char]__c <= 3)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L5:
		[bool]%r2 := ([char]__c > 10)
		if ([bool]%r2)
			goto L4
		else
			goto L6

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 730, [char *]&__FUNCTION__[0])
		abort

	L6:
		[union tree_node *]__t.135 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.135
		[int]__i := 0
		[bool]%r2 := ([int]__i < 0)
		if ([bool]%r2)
			goto L7
		else
			goto L8

	L8:
		[int]%r6 := tree_operand_length([union tree_node *]__t)
		[bool]%r2 := ([int]%r6 <= [int]__i)
		if ([bool]%r2)
			goto L7
		else
			goto L9

	L7:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 730, [char *]&__FUNCTION__[0])
		abort

	L9:
		[union tree_node *[]*]%r7 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r8 := [union tree_node **]&*%r7[0]
		[int]%r9 := [int]__i
data/slplug.c:730:15: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:730:15: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]op0 := [union tree_node *]*%r10
		goto L3

	L3:
		[bool]%r2 := ([union tree_node *]op0 == NULL)
		if ([bool]%r2)
			goto L10
		else
			goto L11

	L10:
		raise(5)
		goto L11

	L11:
		[union tree_node *]%r11 := gimple_call_lhs([union gimple_statement_d *]stmt)
		handle_operand([struct cl_operand *]&dst, [union tree_node *]%r11)
		handle_operand([struct cl_operand *]&fnc, [union tree_node *]op0)
		read_gimple_location([struct cl_location *]&loc, [union gimple_statement_d *]stmt)
		[struct cl_code_listener *]cl.136 := [struct cl_code_listener *]cl
		[fnc *]%r12 := [fnc *]cl.136->insn_call_open
		[struct cl_code_listener *]cl.137 := [struct cl_code_listener *]cl
		[fnc *]%r12([struct cl_code_listener *]cl.137, [struct cl_location *]&loc, [struct cl_operand *]&dst, [struct cl_operand *]&fnc)
		free_cl_operand_data([struct cl_operand *]&dst)
		free_cl_operand_data([struct cl_operand *]&fnc)
		handle_stmt_call_args([union gimple_statement_d *]stmt)
		[struct cl_code_listener *]cl.138 := [struct cl_code_listener *]cl
		[fnc *]%r13 := [fnc *]cl.138->insn_call_close
		[struct cl_code_listener *]cl.139 := [struct cl_code_listener *]cl
		[fnc *]%r13([struct cl_code_listener *]cl.139)
		[int]%r14 := gimple_call_flags([union gimple_statement_d *]stmt)
data/slplug.c:754:9: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r2 := ([int]%r15 != 0)
		if ([bool]%r2)
			goto L12
		else
			goto L13

	L12:
		[enum cl_insn_e]cli.code := [enum cl_insn_e]4
		[struct cl_location]cli.loc := [struct cl_location]loc
		[struct cl_code_listener *]cl.140 := [struct cl_code_listener *]cl
		[fnc *]%r16 := [fnc *]cl.140->insn
		[struct cl_code_listener *]cl.141 := [struct cl_code_listener *]cl
		[fnc *]%r16([struct cl_code_listener *]cl.141, [struct cl_insn *]&cli)
		goto L13

	L13:
		ret
data/slplug.c:730:15: warning: unused register %r4
data/slplug.c:730:15: warning: unused register %r8
data/slplug.c:730:15: warning: unused register %r9
data/slplug.c:730:13: error: uninitialized register %r10
data/slplug.c:754:41: warning: unused register %r14
data/slplug.c:754:8: error: uninitialized register %r15

data/slplug.c:730:15: warning: unused register %r4
data/slplug.c:730:15: warning: unused register %r8
data/slplug.c:730:15: warning: unused register %r9
data/slplug.c:730:13: error: uninitialized register %r10
data/slplug.c:754:41: warning: unused register %r14
data/slplug.c:754:8: error: uninitialized register %r15
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_call_fn.dot' [internal location]
static gimple_call_fn(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 7)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1852, [char *]&__FUNCTION__[0], [enum gimple_code]7, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 1)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_call_lhs.dot' [internal location]
static gimple_call_lhs(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 7)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1818, [char *]&__FUNCTION__[0], [enum gimple_code]7, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 0)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_call_args.dot' [internal location]
static handle_stmt_call_args(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[int]%r1 := gimple_call_num_args([union gimple_statement_d *]stmt)
		[int]argc := [int]%r1
		[int]i := 0
		goto L2

	L3:
		[int]i.142 := [int]i
		[union tree_node *]%r2 := gimple_call_arg([union gimple_statement_d *]stmt, [int]i.142)
		handle_operand([struct cl_operand *]&src, [union tree_node *]%r2)
		[struct cl_code_listener *]cl.143 := [struct cl_code_listener *]cl
		[fnc *]%r3 := [fnc *]cl.143->insn_call_arg
		[int]%r4 := ([int]i + 1)
		[struct cl_code_listener *]cl.144 := [struct cl_code_listener *]cl
		[fnc *]%r3([struct cl_code_listener *]cl.144, [int]%r4, [struct cl_operand *]&src)
		free_cl_operand_data([struct cl_operand *]&src)
		[int]i := ([int]i + 1)
		goto L2

	L2:
		[bool]%r5 := ([int]i < [int]argc)
		if ([bool]%r5)
			goto L3
		else
			goto L4

	L4:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_call_num_args.dot' [internal location]
static gimple_call_num_args(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 7)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1966, [char *]&__FUNCTION__[0], [enum gimple_code]7, [enum tree_code]0)
		abort

	L3:
		[int]num_ops := gimple_num_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ([int]num_ops <= 2)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/gimple.h", 1968, [char *]&__FUNCTION__[0])
		abort

	L5:
		[int]%r3 := ([int]num_ops + (-3))
		ret [int]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_call_arg.dot' [internal location]
static gimple_call_arg(%arg1: [union gimple_statement_d *]gs, %arg2: [int]index):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 7)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1978, [char *]&__FUNCTION__[0], [enum gimple_code]7, [enum tree_code]0)
		abort

	L3:
		[int]%r3 := ([int]index + 3)
		[union tree_node *]%r4 := gimple_op([union gimple_statement_d *]gs, [int]%r3)
		ret [union tree_node *]%r4

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_assign.dot' [internal location]
static handle_stmt_assign(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[union tree_node *]%r1 := gimple_assign_lhs([union gimple_statement_d *]stmt)
		handle_operand([struct cl_operand *]&dst, [union tree_node *]%r1)
		[int]%r2 := gimple_num_ops([union gimple_statement_d *]stmt)
		switch ([int]%r2) {
			default: goto L2
			case 2: goto L3
			case 3: goto L4
		}

	L3:
		[union tree_node *]%r3 := gimple_assign_rhs1([union gimple_statement_d *]stmt)
		[enum tree_code]%r4 := gimple_assign_rhs_code([union gimple_statement_d *]stmt)
		handle_stmt_unop([union gimple_statement_d *]stmt, [enum tree_code]%r4, [struct cl_operand *]&dst, [union tree_node *]%r3)
		goto L5

	L4:
		[union tree_node *]%r5 := gimple_assign_rhs2([union gimple_statement_d *]stmt)
		[union tree_node *]%r6 := gimple_assign_rhs1([union gimple_statement_d *]stmt)
		[enum tree_code]%r7 := gimple_assign_rhs_code([union gimple_statement_d *]stmt)
		handle_stmt_binop([union gimple_statement_d *]stmt, [enum tree_code]%r7, [struct cl_operand *]&dst, [union tree_node *]%r6, [union tree_node *]%r5)
		goto L5

	L2:
		raise(5)
		goto L5

	L5:
		free_cl_operand_data([struct cl_operand *]&dst)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_assign_lhs.dot' [internal location]
static gimple_assign_lhs(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 5)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1623, [char *]&__FUNCTION__[0], [enum gimple_code]5, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 0)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_assign_rhs2.dot' [internal location]
static gimple_assign_rhs2(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 5)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1696, [char *]&__FUNCTION__[0], [enum gimple_code]5, [enum tree_code]0)
		abort

	L3:
		[int]%r3 := gimple_num_ops([union gimple_statement_d *]gs)
		[bool]%r2 := ([int]%r3 > 2)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
		[union tree_node *]%r4 := gimple_op([union gimple_statement_d *]gs, 2)
		goto L6

	L5:
		[union tree_node *]%r4 := NULL
		goto L6

	L6:
		ret [union tree_node *]%r4

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_assign_rhs1.dot' [internal location]
static gimple_assign_rhs1(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 5)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1657, [char *]&__FUNCTION__[0], [enum gimple_code]5, [enum tree_code]0)
		abort

	L3:
		[union tree_node *]%r3 := gimple_op([union gimple_statement_d *]gs, 1)
		ret [union tree_node *]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_assign_rhs_code.dot' [internal location]
static gimple_assign_rhs_code(%arg1: [union gimple_statement_d *]gs):
		goto L1

	L1:
		[union gimple_statement_d *]__gs := [union gimple_statement_d *]gs
		[enum gimple_code]%r1 := gimple_code([union gimple_statement_d *]__gs)
		[bool]%r2 := ([enum gimple_code]%r1 != 5)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		gimple_check_failed([union gimple_statement_d *]__gs, "../gcc/gimple.h", 1756, [char *]&__FUNCTION__[0], [enum gimple_code]5, [enum tree_code]0)
		abort

	L3:
		[enum tree_code]code := gimple_expr_code([union gimple_statement_d *]gs)
		[enum gimple_rhs_class]%r3 := get_gimple_rhs_class([enum tree_code]code)
		[bool]%r2 := ([enum gimple_rhs_class]%r3 == 3)
		if ([bool]%r2)
			goto L4
		else
			goto L5

	L4:
		[union tree_node *]%r4 := gimple_assign_rhs1([union gimple_statement_d *]gs)
		[int]%r5 := [int]%r4->base.code
		[enum tree_code]code := [int]%r5
		goto L5

	L5:
		[enum tree_code]%r6 := [enum tree_code]code
		ret [enum tree_code]%r6

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-gimple_expr_code.dot' [internal location]
static gimple_expr_code(%arg1: [union gimple_statement_d *]stmt):
		goto L1

	L1:
		[enum gimple_code]code := gimple_code([union gimple_statement_d *]stmt)
		[bool]%r1 := ([enum gimple_code]code == 5)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L3:
		[bool]%r1 := ([enum gimple_code]code == 1)
		if ([bool]%r1)
			goto L2
		else
			goto L4

	L2:
		[int]%r2 := [int]stmt->gsbase.subcode
		[enum tree_code]%r3 := [int]%r2
		goto L5

	L4:
		[bool]%r1 := ([enum gimple_code]code == 7)
		if ([bool]%r1)
			goto L6
		else
			goto L7

	L6:
		[enum tree_code]%r3 := [enum tree_code]60
		goto L5

	L7:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("../gcc/gimple.h", 1379, [char *]&__FUNCTION__[0])
		abort

	L5:
		ret [enum tree_code]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-get_gimple_rhs_class.dot' [internal location]
static get_gimple_rhs_class(%arg1: [enum tree_code]code):
		goto L1

	L1:
		[int]code.145 := [enum tree_code]code
../gcc/gimple.h:1615:56: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r1 := [int]gimple_rhs_class_table[-1]
		[enum gimple_rhs_class]%r2 := [int]%r1
		ret [enum gimple_rhs_class]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_binop.dot' [internal location]
static handle_stmt_binop(%arg1: [union gimple_statement_d *]stmt, %arg2: [enum tree_code]code, %arg3: [struct cl_operand *]dst, %arg4: [union tree_node *]src1_tree, %arg5: [union tree_node *]src2_tree):
		goto L1

	L1:
		handle_operand([struct cl_operand *]&src1, [union tree_node *]src1_tree)
		handle_operand([struct cl_operand *]&src2, [union tree_node *]src2_tree)
		[enum cl_insn_e]cli.code := [enum cl_insn_e]6
		[struct cl_operand *]cli.data.insn_binop.dst := [struct cl_operand *]dst
		[struct cl_operand *]cli.data.insn_binop.src1 := [struct cl_operand *]&src1
		[struct cl_operand *]cli.data.insn_binop.src2 := [struct cl_operand *]&src2
		read_gimple_location([struct cl_location *]&cli.loc, [union gimple_statement_d *]stmt)
		[enum cl_binop_e *]ptype := [enum cl_binop_e *]&cli.data.insn_binop.code
		switch ([enum tree_code]code) {
			default: goto L2
			case 64: goto L3
			case 65: goto L4
			case 66: goto L5
			case 67: goto L6
			case 68: goto L7
			case 72: goto L8
			case 76: goto L9
			case 77: goto L10
			case 81: goto L11
			case 82: goto L12
			case 84: goto L13
			case 85: goto L14
			case 86: goto L15
			case 87: goto L16
			case 88: goto L17
			case 89: goto L18
			case 90: goto L19
			case 94: goto L20
			case 95: goto L21
			case 96: goto L22
			case 98: goto L23
			case 99: goto L24
			case 100: goto L25
			case 101: goto L26
			case 102: goto L27
			case 103: goto L28
		}

	L27:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]0
		goto L29

	L28:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]1
		goto L29

	L23:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]2
		goto L29

	L25:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]3
		goto L29

	L24:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]4
		goto L29

	L26:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]5
		goto L29

	L3:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]9
		goto L29

	L4:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]10
		goto L29

	L7:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]11
		goto L29

	L8:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]12
		goto L29

	L9:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]13
		goto L29

	L11:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]14
		goto L29

	L12:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]15
		goto L29

	L20:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]6
		goto L29

	L21:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]7
		goto L29

	L22:
		[enum cl_binop_e]*ptype := [enum cl_binop_e]8
		goto L29

	L10:
		[int]%r1 := [int]stmt->gsbase.location
		[struct expanded_location]%r2 := expand_location([int]%r1)
		[int]%r3 := [int]%r2.column
		[int]%r4 := [int]stmt->gsbase.location
		[struct expanded_location]%r5 := expand_location([int]%r4)
		[int]%r6 := [int]%r5.line
		[int]%r7 := [int]stmt->gsbase.location
		[struct expanded_location]%r8 := expand_location([int]%r7)
		[char *]%r9 := [char *]%r8.file
		[char *]%r10 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.146 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.147 := [struct _IO_FILE *]stderr.146
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.147, [char *]%r10, [char *]%r9, [int]%r6, [int]%r3, "EXACT_DIV_EXPR", "data/slplug.c", 664, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L19:
		[int]%r11 := [int]stmt->gsbase.location
		[struct expanded_location]%r12 := expand_location([int]%r11)
		[int]%r13 := [int]%r12.column
		[int]%r14 := [int]stmt->gsbase.location
		[struct expanded_location]%r15 := expand_location([int]%r14)
		[int]%r16 := [int]%r15.line
		[int]%r17 := [int]stmt->gsbase.location
		[struct expanded_location]%r18 := expand_location([int]%r17)
		[char *]%r19 := [char *]%r18.file
		[char *]%r20 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.148 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.149 := [struct _IO_FILE *]stderr.148
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.149, [char *]%r20, [char *]%r19, [int]%r16, [int]%r13, "BIT_AND_EXPR", "data/slplug.c", 665, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L17:
		[int]%r21 := [int]stmt->gsbase.location
		[struct expanded_location]%r22 := expand_location([int]%r21)
		[int]%r23 := [int]%r22.column
		[int]%r24 := [int]stmt->gsbase.location
		[struct expanded_location]%r25 := expand_location([int]%r24)
		[int]%r26 := [int]%r25.line
		[int]%r27 := [int]stmt->gsbase.location
		[struct expanded_location]%r28 := expand_location([int]%r27)
		[char *]%r29 := [char *]%r28.file
		[char *]%r30 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.150 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.151 := [struct _IO_FILE *]stderr.150
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.151, [char *]%r30, [char *]%r29, [int]%r26, [int]%r23, "BIT_IOR_EXPR", "data/slplug.c", 666, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L18:
		[int]%r31 := [int]stmt->gsbase.location
		[struct expanded_location]%r32 := expand_location([int]%r31)
		[int]%r33 := [int]%r32.column
		[int]%r34 := [int]stmt->gsbase.location
		[struct expanded_location]%r35 := expand_location([int]%r34)
		[int]%r36 := [int]%r35.line
		[int]%r37 := [int]stmt->gsbase.location
		[struct expanded_location]%r38 := expand_location([int]%r37)
		[char *]%r39 := [char *]%r38.file
		[char *]%r40 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.152 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.153 := [struct _IO_FILE *]stderr.152
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.153, [char *]%r40, [char *]%r39, [int]%r36, [int]%r33, "BIT_XOR_EXPR", "data/slplug.c", 667, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L13:
		[int]%r41 := [int]stmt->gsbase.location
		[struct expanded_location]%r42 := expand_location([int]%r41)
		[int]%r43 := [int]%r42.column
		[int]%r44 := [int]stmt->gsbase.location
		[struct expanded_location]%r45 := expand_location([int]%r44)
		[int]%r46 := [int]%r45.line
		[int]%r47 := [int]stmt->gsbase.location
		[struct expanded_location]%r48 := expand_location([int]%r47)
		[char *]%r49 := [char *]%r48.file
		[char *]%r50 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.154 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.155 := [struct _IO_FILE *]stderr.154
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.155, [char *]%r50, [char *]%r49, [int]%r46, [int]%r43, "LSHIFT_EXPR", "data/slplug.c", 668, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L14:
		[int]%r51 := [int]stmt->gsbase.location
		[struct expanded_location]%r52 := expand_location([int]%r51)
		[int]%r53 := [int]%r52.column
		[int]%r54 := [int]stmt->gsbase.location
		[struct expanded_location]%r55 := expand_location([int]%r54)
		[int]%r56 := [int]%r55.line
		[int]%r57 := [int]stmt->gsbase.location
		[struct expanded_location]%r58 := expand_location([int]%r57)
		[char *]%r59 := [char *]%r58.file
		[char *]%r60 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.156 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.157 := [struct _IO_FILE *]stderr.156
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.157, [char *]%r60, [char *]%r59, [int]%r56, [int]%r53, "RSHIFT_EXPR", "data/slplug.c", 669, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L15:
		[int]%r61 := [int]stmt->gsbase.location
		[struct expanded_location]%r62 := expand_location([int]%r61)
		[int]%r63 := [int]%r62.column
		[int]%r64 := [int]stmt->gsbase.location
		[struct expanded_location]%r65 := expand_location([int]%r64)
		[int]%r66 := [int]%r65.line
		[int]%r67 := [int]stmt->gsbase.location
		[struct expanded_location]%r68 := expand_location([int]%r67)
		[char *]%r69 := [char *]%r68.file
		[char *]%r70 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.158 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.159 := [struct _IO_FILE *]stderr.158
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.159, [char *]%r70, [char *]%r69, [int]%r66, [int]%r63, "LROTATE_EXPR", "data/slplug.c", 670, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L16:
		[int]%r71 := [int]stmt->gsbase.location
		[struct expanded_location]%r72 := expand_location([int]%r71)
		[int]%r73 := [int]%r72.column
		[int]%r74 := [int]stmt->gsbase.location
		[struct expanded_location]%r75 := expand_location([int]%r74)
		[int]%r76 := [int]%r75.line
		[int]%r77 := [int]stmt->gsbase.location
		[struct expanded_location]%r78 := expand_location([int]%r77)
		[char *]%r79 := [char *]%r78.file
		[char *]%r80 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.160 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.161 := [struct _IO_FILE *]stderr.160
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.161, [char *]%r80, [char *]%r79, [int]%r76, [int]%r73, "RROTATE_EXPR", "data/slplug.c", 671, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L5:
		[int]%r81 := [int]stmt->gsbase.location
		[struct expanded_location]%r82 := expand_location([int]%r81)
		[int]%r83 := [int]%r82.column
		[int]%r84 := [int]stmt->gsbase.location
		[struct expanded_location]%r85 := expand_location([int]%r84)
		[int]%r86 := [int]%r85.line
		[int]%r87 := [int]stmt->gsbase.location
		[struct expanded_location]%r88 := expand_location([int]%r87)
		[char *]%r89 := [char *]%r88.file
		[char *]%r90 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.162 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.163 := [struct _IO_FILE *]stderr.162
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.163, [char *]%r90, [char *]%r89, [int]%r86, [int]%r83, "MULT_EXPR", "data/slplug.c", 672, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L6:
		[int]%r91 := [int]stmt->gsbase.location
		[struct expanded_location]%r92 := expand_location([int]%r91)
		[int]%r93 := [int]%r92.column
		[int]%r94 := [int]stmt->gsbase.location
		[struct expanded_location]%r95 := expand_location([int]%r94)
		[int]%r96 := [int]%r95.line
		[int]%r97 := [int]stmt->gsbase.location
		[struct expanded_location]%r98 := expand_location([int]%r97)
		[char *]%r99 := [char *]%r98.file
		[char *]%r100 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.164 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.165 := [struct _IO_FILE *]stderr.164
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.165, [char *]%r100, [char *]%r99, [int]%r96, [int]%r93, "POINTER_PLUS_EXPR", "data/slplug.c", 673, [char *]&__FUNCTION__[0])
		[enum cl_insn_e]cli.code := [enum cl_insn_e]0
		goto L29

	L2:
		raise(5)
		goto L29

	L29:
		[enum cl_insn_e]%r101 := [enum cl_insn_e]cli.code
		[bool]%r102 := ([enum cl_insn_e]%r101 != 0)
		if ([bool]%r102)
			goto L30
		else
			goto L31

	L30:
		[struct cl_code_listener *]cl.166 := [struct cl_code_listener *]cl
		[fnc *]%r103 := [fnc *]cl.166->insn
		[struct cl_code_listener *]cl.167 := [struct cl_code_listener *]cl
		[fnc *]%r103([struct cl_code_listener *]cl.167, [struct cl_insn *]&cli)
		goto L31

	L31:
		free_cl_operand_data([struct cl_operand *]&src1)
		free_cl_operand_data([struct cl_operand *]&src2)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_stmt_unop.dot' [internal location]
static handle_stmt_unop(%arg1: [union gimple_statement_d *]stmt, %arg2: [enum tree_code]code, %arg3: [struct cl_operand *]dst, %arg4: [union tree_node *]src_tree):
		goto L1

	L1:
		handle_operand([struct cl_operand *]&src, [union tree_node *]src_tree)
		[enum cl_insn_e]cli.code := [enum cl_insn_e]5
		[enum cl_unop_e]cli.data.insn_unop.code := [enum cl_unop_e]0
		[struct cl_operand *]cli.data.insn_unop.dst := [struct cl_operand *]dst
		[struct cl_operand *]cli.data.insn_unop.src := [struct cl_operand *]&src
		read_gimple_location([struct cl_location *]&cli.loc, [union gimple_statement_d *]stmt)
		[int]%r1 := [int]src_tree->base.code
		[int]%r2 := [int]%r1
		[bool]%r3 := ([int]%r2 != [enum tree_code]code)
		if ([bool]%r3)
			goto L2
		else
			goto L3

	L2:
		switch ([enum tree_code]code) {
			default: goto L4
			case 32: goto L5
			case 97: goto L6
			case 114: goto L5
			case 116: goto L5
		}

	L5:
		goto L3

	L6:
		[enum cl_unop_e]cli.data.insn_unop.code := [enum cl_unop_e]1
		goto L3

	L4:
		raise(5)
		goto L3

	L3:
		[struct cl_code_listener *]cl.168 := [struct cl_code_listener *]cl
		[fnc *]%r4 := [fnc *]cl.168->insn
		[struct cl_code_listener *]cl.169 := [struct cl_code_listener *]cl
		[fnc *]%r4([struct cl_code_listener *]cl.169, [struct cl_insn *]&cli)
		free_cl_operand_data([struct cl_operand *]&src)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_operand.dot' [internal location]
static handle_operand(%arg1: [struct cl_operand *]op, %arg2: [union tree_node *]t):
		goto L1

	L1:
		[enum cl_operand_e]op->code := [enum cl_operand_e]0
		[char *]op->loc.file := NULL
		[int]op->loc.line := (-1)
		[enum cl_scope_e]op->scope := [enum cl_scope_e]0
		[struct cl_type *]op->type := NULL
		[struct cl_accessor *]op->accessor := NULL
		[union tree_node *]t.170 := [union tree_node *]t
		[bool]%r1 := ([union tree_node *]t.170 == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		goto L4

	L3:
		[union tree_node *]t.171 := [union tree_node *]t
		[int]type := add_type_if_needed([union tree_node *]t.171)
		[struct htab *]type_db.172 := [struct htab *]type_db
		[struct cl_type *]%r2 := type_db_lookup([struct htab *]type_db.172, [int]type)
		[struct cl_type *]op->type := [struct cl_type *]%r2
		[struct cl_accessor *]op->accessor := NULL
		goto L5

	L5:
		[struct cl_accessor **]%r3 := [struct cl_accessor **]&op->accessor
		[bool]%r4 := handle_accessor([struct cl_accessor **]%r3, [union tree_node **]&t)
		[bool]%r1 := ([bool]%r4 != false)
		if ([bool]%r1)
			goto L5
		else
			goto L6

	L6:
		[union tree_node *]t.173 := [union tree_node *]t
		[bool]%r1 := ([union tree_node *]t.173 == NULL)
		if ([bool]%r1)
			goto L7
		else
			goto L8

	L7:
		raise(5)
		goto L8

	L8:
		[union tree_node *]t.174 := [union tree_node *]t
		read_raw_operand([struct cl_operand *]op, [union tree_node *]t.174)
		goto L4

	L4:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-free_cl_operand_data.dot' [internal location]
static free_cl_operand_data(%arg1: [struct cl_operand *]op):
		goto L1

	L1:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_accessor.dot' [internal location]
static handle_accessor(%arg1: [struct cl_accessor **]ac, %arg2: [union tree_node **]pt):
		goto L1

	L1:
		[union tree_node *]t := [union tree_node *]*pt
		[bool]%r1 := ([union tree_node *]t == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		raise(5)
		goto L3

	L3:
		[int]%r2 := [int]t->base.code
		[enum tree_code]code := [int]%r2
		switch ([enum tree_code]code) {
			default: goto L4
			case 40: goto L5
			case 41: goto L6
			case 44: goto L7
			case 46: goto L8
			case 121: goto L9
		}

	L9:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r3 := [int]__t->base.code
		[int]%r4 := [int]%r3
data/slplug.c:529:31: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r5 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r5
		[bool]%r1 := ([char]__c <= 3)
		if ([bool]%r1)
			goto L10
		else
			goto L11

	L11:
		[bool]%r1 := ([char]__c > 10)
		if ([bool]%r1)
			goto L10
		else
			goto L12

	L10:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 529, [char *]&__FUNCTION__[0])
		abort

	L12:
		[union tree_node *]__t.175 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.175
		[int]__i := 0
		[bool]%r1 := ([int]__i < 0)
		if ([bool]%r1)
			goto L13
		else
			goto L14

	L14:
		[int]%r6 := tree_operand_length([union tree_node *]__t)
		[bool]%r1 := ([int]%r6 <= [int]__i)
		if ([bool]%r1)
			goto L13
		else
			goto L15

	L13:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 529, [char *]&__FUNCTION__[0])
		abort

	L15:
		[union tree_node *[]*]%r7 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r8 := [union tree_node **]&*%r7[0]
		[int]%r9 := [int]__i
data/slplug.c:529:31: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:529:31: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]%r10 := [union tree_node *]*%r11
		[int]%r12 := [int]%r10->base.code
		[bool]%r1 := ([int]%r12 != 28)
		if ([bool]%r1)
			goto L16
		else
			goto L17

	L16:
		chain_accessor([struct cl_accessor **]ac, [enum cl_type_e]0)
		goto L17

	L17:
		goto L18

	L7:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r13 := [int]__t->base.code
		[int]%r14 := [int]%r13
data/slplug.c:534:31: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r15 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r15
		[bool]%r1 := ([char]__c <= 3)
		if ([bool]%r1)
			goto L19
		else
			goto L20

	L20:
		[bool]%r1 := ([char]__c > 10)
		if ([bool]%r1)
			goto L19
		else
			goto L21

	L19:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 534, [char *]&__FUNCTION__[0])
		abort

	L21:
		[union tree_node *]__t.176 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.176
		[int]__i := 0
		[bool]%r1 := ([int]__i < 0)
		if ([bool]%r1)
			goto L22
		else
			goto L23

	L23:
		[int]%r16 := tree_operand_length([union tree_node *]__t)
		[bool]%r1 := ([int]%r16 <= [int]__i)
		if ([bool]%r1)
			goto L22
		else
			goto L24

	L22:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 534, [char *]&__FUNCTION__[0])
		abort

	L24:
		[union tree_node *[]*]%r17 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r18 := [union tree_node **]&*%r17[0]
		[int]%r19 := [int]__i
data/slplug.c:534:31: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:534:31: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]%r20 := [union tree_node *]*%r21
		[int]%r22 := [int]%r20->base.code
		[bool]%r1 := ([int]%r22 != 28)
		if ([bool]%r1)
			goto L25
		else
			goto L26

	L25:
		handle_accessor_array_ref([struct cl_accessor **]ac, [union tree_node *]t)
		goto L26

	L26:
		goto L18

	L8:
		handle_accessor_indirect_ref([struct cl_accessor **]ac, [union tree_node *]t)
		goto L18

	L5:
		handle_accessor_component_ref([struct cl_accessor **]ac, [union tree_node *]t)
		goto L18

	L6:
		[int]%r23 := [int]t->base.code
		[int]%r24 := [int]%r23
data/slplug.c:547:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r25 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r25 > 3)
		if ([bool]%r1)
			goto L27
		else
			goto L28

	L27:
		[int]%r26 := [int]t->base.code
		[int]%r27 := [int]%r26
data/slplug.c:547:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r28 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r28 <= 10)
		if ([bool]%r1)
			goto L29
		else
			goto L28

	L29:
		[int]iftmp.177 := [int]t->exp.locus
		goto L30

	L28:
		[int]iftmp.177 := 0
		goto L30

	L30:
		[struct expanded_location]%r29 := expand_location([int]iftmp.177)
		[int]%r30 := [int]%r29.column
		[int]%r31 := [int]t->base.code
		[int]%r32 := [int]%r31
data/slplug.c:547:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r33 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r33 > 3)
		if ([bool]%r1)
			goto L31
		else
			goto L32

	L31:
		[int]%r34 := [int]t->base.code
		[int]%r35 := [int]%r34
data/slplug.c:547:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r36 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r36 <= 10)
		if ([bool]%r1)
			goto L33
		else
			goto L32

	L33:
		[int]iftmp.178 := [int]t->exp.locus
		goto L34

	L32:
		[int]iftmp.178 := 0
		goto L34

	L34:
		[struct expanded_location]%r37 := expand_location([int]iftmp.178)
		[int]%r38 := [int]%r37.line
		[int]%r39 := [int]t->base.code
		[int]%r40 := [int]%r39
data/slplug.c:547:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r41 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r41 > 3)
		if ([bool]%r1)
			goto L35
		else
			goto L36

	L35:
		[int]%r42 := [int]t->base.code
		[int]%r43 := [int]%r42
data/slplug.c:547:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r44 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r44 <= 10)
		if ([bool]%r1)
			goto L37
		else
			goto L36

	L37:
		[int]iftmp.179 := [int]t->exp.locus
		goto L38

	L36:
		[int]iftmp.179 := 0
		goto L38

	L38:
		[struct expanded_location]%r45 := expand_location([int]iftmp.179)
		[char *]%r46 := [char *]%r45.file
		[char *]%r47 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.180 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.181 := [struct _IO_FILE *]stderr.180
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.181, [char *]%r47, [char *]%r46, [int]%r38, [int]%r30, "BIT_FIELD_REF", "data/slplug.c", 547, [char *]&__FUNCTION__[0])
		[int]verbose.182 := [int]verbose
data/slplug.c:547:13: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r1 := ([int]%r48 != 0)
		if ([bool]%r1)
			goto L39
		else
			goto L40

	L39:
		debug_tree([union tree_node *]t)
		goto L40

	L40:
		goto L18

	L4:
		[bool]%r49 := false
		goto L41

	L18:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r50 := [int]__t->base.code
		[int]%r51 := [int]%r50
data/slplug.c:554:11: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r52 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r52
		[bool]%r1 := ([char]__c <= 3)
		if ([bool]%r1)
			goto L42
		else
			goto L43

	L43:
		[bool]%r1 := ([char]__c > 10)
		if ([bool]%r1)
			goto L42
		else
			goto L44

	L42:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 554, [char *]&__FUNCTION__[0])
		abort

	L44:
		[union tree_node *]__t.183 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.183
		[int]__i := 0
		[bool]%r1 := ([int]__i < 0)
		if ([bool]%r1)
			goto L45
		else
			goto L46

	L46:
		[int]%r53 := tree_operand_length([union tree_node *]__t)
		[bool]%r1 := ([int]%r53 <= [int]__i)
		if ([bool]%r1)
			goto L45
		else
			goto L47

	L45:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 554, [char *]&__FUNCTION__[0])
		abort

	L47:
		[union tree_node *[]*]%r54 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r55 := [union tree_node **]&*%r54[0]
		[int]%r56 := [int]__i
data/slplug.c:554:11: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:554:11: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]%r57 := [union tree_node *]*%r58
		[union tree_node *]*pt := [union tree_node *]%r57
		[bool]%r49 := true
		goto L41

	L41:
		ret [bool]%r49
data/slplug.c:529:31: warning: unused register %r4
data/slplug.c:529:31: warning: unused register %r8
data/slplug.c:529:31: warning: unused register %r9
data/slplug.c:529:31: error: uninitialized register %r11
data/slplug.c:534:31: warning: unused register %r14
data/slplug.c:534:31: warning: unused register %r18
data/slplug.c:534:31: warning: unused register %r19
data/slplug.c:534:31: error: uninitialized register %r21
data/slplug.c:547:13: warning: unused register %r24
data/slplug.c:547:13: warning: unused register %r27
data/slplug.c:547:13: warning: unused register %r32
data/slplug.c:547:13: warning: unused register %r35
data/slplug.c:547:13: warning: unused register %r40
data/slplug.c:547:13: warning: unused register %r43
data/slplug.c:547:13: error: uninitialized register %r48
data/slplug.c:554:11: warning: unused register %r51
data/slplug.c:554:11: warning: unused register %r55
data/slplug.c:554:11: warning: unused register %r56
data/slplug.c:554:11: error: uninitialized register %r58

data/slplug.c:529:31: warning: unused register %r4
data/slplug.c:529:31: warning: unused register %r8
data/slplug.c:529:31: warning: unused register %r9
data/slplug.c:529:31: error: uninitialized register %r11
data/slplug.c:534:31: warning: unused register %r14
data/slplug.c:534:31: warning: unused register %r18
data/slplug.c:534:31: warning: unused register %r19
data/slplug.c:534:31: error: uninitialized register %r21
data/slplug.c:547:13: warning: unused register %r24
data/slplug.c:547:13: warning: unused register %r27
data/slplug.c:547:13: warning: unused register %r32
data/slplug.c:547:13: warning: unused register %r35
data/slplug.c:547:13: warning: unused register %r40
data/slplug.c:547:13: warning: unused register %r43
data/slplug.c:547:13: error: uninitialized register %r48
data/slplug.c:554:11: warning: unused register %r51
data/slplug.c:554:11: warning: unused register %r55
data/slplug.c:554:11: warning: unused register %r56
data/slplug.c:554:11: error: uninitialized register %r58
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_accessor_component_ref.dot' [internal location]
static handle_accessor_component_ref(%arg1: [struct cl_accessor **]ac, %arg2: [union tree_node *]t):
		goto L1

	L1:
		chain_accessor([struct cl_accessor **]ac, [enum cl_type_e]3)
		[struct cl_type *]type := operand_type_lookup([union tree_node *]t)
		[struct cl_accessor *]%r1 := [struct cl_accessor *]*ac
		[struct cl_type *]%r1->type := [struct cl_type *]type
		[struct cl_accessor *]%r2 := [struct cl_accessor *]*ac
		[union tree_node *]__t := [union tree_node *]t
		[int]%r3 := [int]__t->base.code
		[int]%r4 := [int]%r3
data/slplug.c:517:46: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r5 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r5
		[bool]%r6 := ([char]__c <= 3)
		if ([bool]%r6)
			goto L2
		else
			goto L3

	L3:
		[bool]%r6 := ([char]__c > 10)
		if ([bool]%r6)
			goto L2
		else
			goto L4

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 517, [char *]&__FUNCTION__[0])
		abort

	L4:
		[union tree_node *]__t.184 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.184
		[int]__i := 1
		[bool]%r6 := ([int]__i < 0)
		if ([bool]%r6)
			goto L5
		else
			goto L6

	L6:
		[int]%r7 := tree_operand_length([union tree_node *]__t)
		[bool]%r6 := ([int]%r7 <= [int]__i)
		if ([bool]%r6)
			goto L5
		else
			goto L7

	L5:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 517, [char *]&__FUNCTION__[0])
		abort

	L7:
		[union tree_node *[]*]%r8 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r9 := [union tree_node **]&*%r8[0]
		[int]%r10 := [int]__i
data/slplug.c:517:46: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:517:46: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]%r11 := [union tree_node *]*%r12
		[int]%r13 := accessor_item_lookup([struct cl_type *]type, [union tree_node *]%r11)
		[int]%r2->item := [int]%r13
		ret
data/slplug.c:517:46: warning: unused register %r4
data/slplug.c:517:46: warning: unused register %r9
data/slplug.c:517:46: warning: unused register %r10
data/slplug.c:517:39: error: uninitialized register %r12

data/slplug.c:517:46: warning: unused register %r4
data/slplug.c:517:46: warning: unused register %r9
data/slplug.c:517:46: warning: unused register %r10
data/slplug.c:517:39: error: uninitialized register %r12
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_accessor_indirect_ref.dot' [internal location]
static handle_accessor_indirect_ref(%arg1: [struct cl_accessor **]ac, %arg2: [union tree_node *]t):
		goto L1

	L1:
		chain_accessor([struct cl_accessor **]ac, [enum cl_type_e]1)
		[struct cl_accessor *]%r1 := [struct cl_accessor *]*ac
		[struct cl_type *]%r2 := operand_type_lookup([union tree_node *]t)
		[struct cl_type *]%r1->type := [struct cl_type *]%r2
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-handle_accessor_array_ref.dot' [internal location]
static handle_accessor_array_ref(%arg1: [struct cl_accessor **]ac, %arg2: [union tree_node *]t):
		goto L1

	L1:
		chain_accessor([struct cl_accessor **]ac, [enum cl_type_e]2)
		[struct cl_accessor *]%r1 := [struct cl_accessor *]*ac
		[struct cl_type *]%r2 := operand_type_lookup([union tree_node *]t)
		[struct cl_type *]%r1->type := [struct cl_type *]%r2
		[union tree_node *]__t := [union tree_node *]t
		[int]%r3 := [int]__t->base.code
		[int]%r4 := [int]%r3
data/slplug.c:489:16: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r5 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r5
		[bool]%r6 := ([char]__c <= 3)
		if ([bool]%r6)
			goto L2
		else
			goto L3

	L3:
		[bool]%r6 := ([char]__c > 10)
		if ([bool]%r6)
			goto L2
		else
			goto L4

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 489, [char *]&__FUNCTION__[0])
		abort

	L4:
		[union tree_node *]__t.185 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.185
		[int]__i := 1
		[bool]%r6 := ([int]__i < 0)
		if ([bool]%r6)
			goto L5
		else
			goto L6

	L6:
		[int]%r7 := tree_operand_length([union tree_node *]__t)
		[bool]%r6 := ([int]%r7 <= [int]__i)
		if ([bool]%r6)
			goto L5
		else
			goto L7

	L5:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 489, [char *]&__FUNCTION__[0])
		abort

	L7:
		[union tree_node *[]*]%r8 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r9 := [union tree_node **]&*%r8[0]
		[int]%r10 := [int]__i
data/slplug.c:489:16: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:489:16: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]op1 := [union tree_node *]*%r11
		[bool]%r6 := ([union tree_node *]op1 == NULL)
		if ([bool]%r6)
			goto L8
		else
			goto L9

	L8:
		raise(5)
		goto L9

	L9:
		[int]%r12 := [int]op1->base.code
		[enum tree_code]code := [int]%r12
		[bool]%r6 := ([enum tree_code]code != 23)
		if ([bool]%r6)
			goto L10
		else
			goto L11

	L10:
		[int]%r13 := [int]t->base.code
		[int]%r14 := [int]%r13
data/slplug.c:495:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r15 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r6 := ([enum tree_code_class]%r15 > 3)
		if ([bool]%r6)
			goto L12
		else
			goto L13

	L12:
		[int]%r16 := [int]t->base.code
		[int]%r17 := [int]%r16
data/slplug.c:495:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r18 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r6 := ([enum tree_code_class]%r18 <= 10)
		if ([bool]%r6)
			goto L14
		else
			goto L13

	L14:
		[int]iftmp.186 := [int]t->exp.locus
		goto L15

	L13:
		[int]iftmp.186 := 0
		goto L15

	L15:
		[struct expanded_location]%r19 := expand_location([int]iftmp.186)
		[int]%r20 := [int]%r19.column
		[int]%r21 := [int]t->base.code
		[int]%r22 := [int]%r21
data/slplug.c:495:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r23 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r6 := ([enum tree_code_class]%r23 > 3)
		if ([bool]%r6)
			goto L16
		else
			goto L17

	L16:
		[int]%r24 := [int]t->base.code
		[int]%r25 := [int]%r24
data/slplug.c:495:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r26 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r6 := ([enum tree_code_class]%r26 <= 10)
		if ([bool]%r6)
			goto L18
		else
			goto L17

	L18:
		[int]iftmp.187 := [int]t->exp.locus
		goto L19

	L17:
		[int]iftmp.187 := 0
		goto L19

	L19:
		[struct expanded_location]%r27 := expand_location([int]iftmp.187)
		[int]%r28 := [int]%r27.line
		[int]%r29 := [int]t->base.code
		[int]%r30 := [int]%r29
data/slplug.c:495:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r31 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r6 := ([enum tree_code_class]%r31 > 3)
		if ([bool]%r6)
			goto L20
		else
			goto L21

	L20:
		[int]%r32 := [int]t->base.code
		[int]%r33 := [int]%r32
data/slplug.c:495:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r34 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r6 := ([enum tree_code_class]%r34 <= 10)
		if ([bool]%r6)
			goto L22
		else
			goto L21

	L22:
		[int]iftmp.188 := [int]t->exp.locus
		goto L23

	L21:
		[int]iftmp.188 := 0
		goto L23

	L23:
		[struct expanded_location]%r35 := expand_location([int]iftmp.188)
		[char *]%r36 := [char *]%r35.file
		[char *]%r37 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.189 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.190 := [struct _IO_FILE *]stderr.189
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.190, [char *]%r37, [char *]%r36, [int]%r28, [int]%r20, "non-constant array accessor", "data/slplug.c", 495, [char *]&__FUNCTION__[0])
		[int]verbose.191 := [int]verbose
data/slplug.c:495:9: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r6 := ([int]%r38 != 0)
		if ([bool]%r6)
			goto L24
		else
			goto L25

	L24:
		debug_tree([union tree_node *]t)
		goto L25

	L25:
		[struct cl_accessor *]%r39 := [struct cl_accessor *]*ac
		[int]%r39->item := (-1)
		goto L26

	L11:
		[union tree_node *]__t := [union tree_node *]op1
		[int]%r40 := [int]__t->base.code
		[bool]%r6 := ([int]%r40 != 23)
		if ([bool]%r6)
			goto L27
		else
			goto L28

	L27:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 500, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L28:
		[union tree_node *]%r41 := [union tree_node *]__t
		[int]%r42 := [int]%r41->int_cst.int_cst.high
		[bool]%r6 := ([int]%r42 != 0)
		if ([bool]%r6)
			goto L29
		else
			goto L30

	L29:
		raise(5)
		goto L30

	L30:
		[struct cl_accessor *]%r43 := [struct cl_accessor *]*ac
		[union tree_node *]__t := [union tree_node *]op1
		[int]%r44 := [int]__t->base.code
		[bool]%r6 := ([int]%r44 != 23)
		if ([bool]%r6)
			goto L31
		else
			goto L32

	L31:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 502, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L32:
		[union tree_node *]%r45 := [union tree_node *]__t
		[int]%r46 := [int]%r45->int_cst.int_cst.low
		[int]%r47 := [int]%r46
		[int]%r43->item := [int]%r47
		goto L26

	L26:
		ret
data/slplug.c:489:16: warning: unused register %r4
data/slplug.c:489:16: warning: unused register %r9
data/slplug.c:489:16: warning: unused register %r10
data/slplug.c:489:10: error: uninitialized register %r11
data/slplug.c:495:9: warning: unused register %r14
data/slplug.c:495:9: warning: unused register %r17
data/slplug.c:495:9: warning: unused register %r22
data/slplug.c:495:9: warning: unused register %r25
data/slplug.c:495:9: warning: unused register %r30
data/slplug.c:495:9: warning: unused register %r33
data/slplug.c:495:9: error: uninitialized register %r38

data/slplug.c:489:16: warning: unused register %r4
data/slplug.c:489:16: warning: unused register %r9
data/slplug.c:489:16: warning: unused register %r10
data/slplug.c:489:10: error: uninitialized register %r11
data/slplug.c:495:9: warning: unused register %r14
data/slplug.c:495:9: warning: unused register %r17
data/slplug.c:495:9: warning: unused register %r22
data/slplug.c:495:9: warning: unused register %r25
data/slplug.c:495:9: warning: unused register %r30
data/slplug.c:495:9: warning: unused register %r33
data/slplug.c:495:9: error: uninitialized register %r38
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-operand_type_lookup.dot' [internal location]
static operand_type_lookup(%arg1: [union tree_node *]t):
		goto L1

	L1:
		[bool]%r1 := ([union tree_node *]t == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		raise(5)
		goto L3

	L3:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r2 := [int]__t->base.code
		[int]%r3 := [int]%r2
data/slplug.c:477:16: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r4 := [enum tree_code_class]tree_code_type[-1]
		[char]__c := [enum tree_code_class]%r4
		[bool]%r1 := ([char]__c <= 3)
		if ([bool]%r1)
			goto L4
		else
			goto L5

	L5:
		[bool]%r1 := ([char]__c > 10)
		if ([bool]%r1)
			goto L4
		else
			goto L6

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]10, "data/slplug.c", 477, [char *]&__FUNCTION__[0])
		abort

	L6:
		[union tree_node *]__t.192 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]__t.192
		[int]__i := 0
		[bool]%r1 := ([int]__i < 0)
		if ([bool]%r1)
			goto L7
		else
			goto L8

	L8:
		[int]%r5 := tree_operand_length([union tree_node *]__t)
		[bool]%r1 := ([int]%r5 <= [int]__i)
		if ([bool]%r1)
			goto L7
		else
			goto L9

	L7:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_operand_check_failed([int]__i, [union tree_node *]__t, "data/slplug.c", 477, [char *]&__FUNCTION__[0])
		abort

	L9:
		[union tree_node *[]*]%r6 := [union tree_node *[]*]&__t->exp.operands
		[union tree_node **]%r7 := [union tree_node **]&*%r6[0]
		[int]%r8 := [int]__i
data/slplug.c:477:16: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:477:16: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[union tree_node *]op0 := [union tree_node *]*%r9
		[bool]%r1 := ([union tree_node *]op0 == NULL)
		if ([bool]%r1)
			goto L10
		else
			goto L11

	L10:
		raise(5)
		goto L11

	L11:
		[int]%r10 := add_type_if_needed([union tree_node *]op0)
		[struct htab *]type_db.193 := [struct htab *]type_db
		[struct cl_type *]%r11 := type_db_lookup([struct htab *]type_db.193, [int]%r10)
		ret [struct cl_type *]%r11
data/slplug.c:477:16: warning: unused register %r3
data/slplug.c:477:16: warning: unused register %r7
data/slplug.c:477:16: warning: unused register %r8
data/slplug.c:477:10: error: uninitialized register %r9

data/slplug.c:477:16: warning: unused register %r3
data/slplug.c:477:16: warning: unused register %r7
data/slplug.c:477:16: warning: unused register %r8
data/slplug.c:477:10: error: uninitialized register %r9
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-accessor_item_lookup.dot' [internal location]
static accessor_item_lookup(%arg1: [struct cl_type *]type, %arg2: [union tree_node *]t):
		goto L1

	L1:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r1 := [int]__t->base.code
		[int]%r2 := [int]%r1
data/slplug.c:454:17: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r3 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r3 != 1)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 454, [char *]&__FUNCTION__[0])
		abort

	L3:
		[union tree_node *]%r5 := [union tree_node *]__t
		[union tree_node *]name := [union tree_node *]%r5->decl_minimal.name
		[bool]%r4 := ([union tree_node *]name == NULL)
		if ([bool]%r4)
			goto L4
		else
			goto L5

	L4:
		raise(5)
		goto L5

	L5:
		[union tree_node *]__t := [union tree_node *]name
		[int]%r6 := [int]__t->base.code
		[bool]%r4 := ([int]%r6 != 1)
		if ([bool]%r4)
			goto L6
		else
			goto L7

	L6:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 459, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L7:
		[union tree_node *]%r7 := [union tree_node *]__t
		[int *]%r8 := [int *]%r7->identifier.id.str
		[char *]ident := [int *]%r8
		[struct cl_type_item *]items := [struct cl_type_item *]type->items
		[int]i := 0
		goto L8

	L9:
		[int]%r9 := [int]i
data/slplug.c:464:13: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:464:13: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[char *]%r10 := [char *]%r11->name
		[int]%r12 := strcmp([char *]ident, [char *]%r10)
		[bool]%r4 := ([int]%r12 == 0)
		if ([bool]%r4)
			goto L10
		else
			goto L11

	L10:
		[int]%r13 := [int]i
		goto L12

	L11:
		[int]i := ([int]i + 1)
		goto L8

	L8:
		[int]%r14 := [int]type->item_cnt
		[bool]%r4 := ([int]%r14 > [int]i)
		if ([bool]%r4)
			goto L9
		else
			goto L13

	L13:
		raise(5)
		[int]%r13 := (-1)
		goto L12

	L12:
		ret [int]%r13
data/slplug.c:454:17: warning: unused register %r2
data/slplug.c:464:13: warning: unused register %r9
data/slplug.c:464:13: error: uninitialized register %r11

data/slplug.c:454:17: warning: unused register %r2
data/slplug.c:464:13: warning: unused register %r9
data/slplug.c:464:13: error: uninitialized register %r11
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-chain_accessor.dot' [internal location]
static chain_accessor(%arg1: [struct cl_accessor **]ac, %arg2: [enum cl_type_e]code):
		goto L1

	L1:
		[void *]%r1 := ggc_alloc_cleared_stat(32)
		[struct cl_accessor *]ac_new := [void *]%r1
		[bool]%r2 := ([struct cl_accessor *]ac_new == NULL)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 444, [char *]&__FUNCTION__[0])
		abort

	L3:
		[enum cl_accessor_e]ac_new->code := [enum cl_type_e]code
		[struct cl_accessor *]%r3 := [struct cl_accessor *]*ac
		[struct cl_accessor *]ac_new->next := [struct cl_accessor *]%r3
		[struct cl_accessor *]*ac := [struct cl_accessor *]ac_new
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-read_raw_operand.dot' [internal location]
static read_raw_operand(%arg1: [struct cl_operand *]op, %arg2: [union tree_node *]t):
		goto L1

	L1:
		[int]%r1 := [int]t->base.code
		[enum tree_code]code := [int]%r1
		switch ([enum tree_code]code) {
			default: goto L2
			case 23: goto L3
			case 24: goto L4
			case 28: goto L5
			case 29: goto L6
			case 32: goto L6
			case 34: goto L6
			case 52: goto L7
		}

	L6:
		read_operand_decl([struct cl_operand *]op, [union tree_node *]t)
		goto L8

	L3:
		[enum cl_operand_e]op->code := [enum cl_operand_e]1
		[union tree_node *]__t := [union tree_node *]t
		[int]%r2 := [int]__t->base.code
		[bool]%r3 := ([int]%r2 != 23)
		if ([bool]%r3)
			goto L9
		else
			goto L10

	L9:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 415, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L10:
		[union tree_node *]%r4 := [union tree_node *]__t
		[int]%r5 := [int]%r4->int_cst.int_cst.high
		[bool]%r3 := ([int]%r5 != 0)
		if ([bool]%r3)
			goto L11
		else
			goto L12

	L11:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r6 := [int]__t->base.code
		[bool]%r3 := ([int]%r6 != 23)
		if ([bool]%r3)
			goto L13
		else
			goto L14

	L13:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 416, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L14:
		[union tree_node *]%r7 := [union tree_node *]__t
		[int]%r8 := [int]%r7->int_cst.int_cst.high
		[int]iftmp.194 := [int]%r8
		goto L15

	L12:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r9 := [int]__t->base.code
		[bool]%r3 := ([int]%r9 != 23)
		if ([bool]%r3)
			goto L16
		else
			goto L17

	L16:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 417, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L17:
		[union tree_node *]%r10 := [union tree_node *]__t
		[int]%r11 := [int]%r10->int_cst.int_cst.low
		[int]iftmp.194 := [int]%r11
		goto L15

	L15:
		[int]op->data.cst_int.value := [int]iftmp.194
		goto L8

	L5:
		[enum cl_operand_e]op->code := [enum cl_operand_e]1
		[struct cl_type *]op->type := [struct cl_type *]&builtin_string_type
		[union tree_node *]__t := [union tree_node *]t
		[int]%r12 := [int]__t->base.code
		[bool]%r3 := ([int]%r12 != 28)
		if ([bool]%r3)
			goto L18
		else
			goto L19

	L18:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 423, [char *]&__FUNCTION__[0], 28, 0)
		abort

	L19:
		[union tree_node *]%r13 := [union tree_node *]__t
		[char []*]%r14 := [char []*]&%r13->string.str
		[char *]%r15 := [char *]&*%r14[0]
		[char *]op->data.cst_string.value := [char *]%r15
		goto L8

	L4:
		[int]%r16 := [int]t->base.code
		[int]%r17 := [int]%r16
data/slplug.c:427:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r18 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r18 > 3)
		if ([bool]%r3)
			goto L20
		else
			goto L21

	L20:
		[int]%r19 := [int]t->base.code
		[int]%r20 := [int]%r19
data/slplug.c:427:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r21 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r21 <= 10)
		if ([bool]%r3)
			goto L22
		else
			goto L21

	L22:
		[int]iftmp.195 := [int]t->exp.locus
		goto L23

	L21:
		[int]iftmp.195 := 0
		goto L23

	L23:
		[struct expanded_location]%r22 := expand_location([int]iftmp.195)
		[int]%r23 := [int]%r22.column
		[int]%r24 := [int]t->base.code
		[int]%r25 := [int]%r24
data/slplug.c:427:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r26 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r26 > 3)
		if ([bool]%r3)
			goto L24
		else
			goto L25

	L24:
		[int]%r27 := [int]t->base.code
		[int]%r28 := [int]%r27
data/slplug.c:427:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r29 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r29 <= 10)
		if ([bool]%r3)
			goto L26
		else
			goto L25

	L26:
		[int]iftmp.196 := [int]t->exp.locus
		goto L27

	L25:
		[int]iftmp.196 := 0
		goto L27

	L27:
		[struct expanded_location]%r30 := expand_location([int]iftmp.196)
		[int]%r31 := [int]%r30.line
		[int]%r32 := [int]t->base.code
		[int]%r33 := [int]%r32
data/slplug.c:427:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r34 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r34 > 3)
		if ([bool]%r3)
			goto L28
		else
			goto L29

	L28:
		[int]%r35 := [int]t->base.code
		[int]%r36 := [int]%r35
data/slplug.c:427:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r37 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r37 <= 10)
		if ([bool]%r3)
			goto L30
		else
			goto L29

	L30:
		[int]iftmp.197 := [int]t->exp.locus
		goto L31

	L29:
		[int]iftmp.197 := 0
		goto L31

	L31:
		[struct expanded_location]%r38 := expand_location([int]iftmp.197)
		[char *]%r39 := [char *]%r38.file
		[char *]%r40 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.198 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.199 := [struct _IO_FILE *]stderr.198
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.199, [char *]%r40, [char *]%r39, [int]%r31, [int]%r23, "REAL_CST", "data/slplug.c", 427, [char *]&__FUNCTION__[0])
		[int]verbose.200 := [int]verbose
data/slplug.c:427:13: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r3 := ([int]%r41 != 0)
		if ([bool]%r3)
			goto L32
		else
			goto L33

	L32:
		debug_tree([union tree_node *]t)
		goto L33

	L33:
		goto L8

	L7:
		[int]%r42 := [int]t->base.code
		[int]%r43 := [int]%r42
data/slplug.c:431:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r44 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r44 > 3)
		if ([bool]%r3)
			goto L34
		else
			goto L35

	L34:
		[int]%r45 := [int]t->base.code
		[int]%r46 := [int]%r45
data/slplug.c:431:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r47 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r47 <= 10)
		if ([bool]%r3)
			goto L36
		else
			goto L35

	L36:
		[int]iftmp.201 := [int]t->exp.locus
		goto L37

	L35:
		[int]iftmp.201 := 0
		goto L37

	L37:
		[struct expanded_location]%r48 := expand_location([int]iftmp.201)
		[int]%r49 := [int]%r48.column
		[int]%r50 := [int]t->base.code
		[int]%r51 := [int]%r50
data/slplug.c:431:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r52 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r52 > 3)
		if ([bool]%r3)
			goto L38
		else
			goto L39

	L38:
		[int]%r53 := [int]t->base.code
		[int]%r54 := [int]%r53
data/slplug.c:431:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r55 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r55 <= 10)
		if ([bool]%r3)
			goto L40
		else
			goto L39

	L40:
		[int]iftmp.202 := [int]t->exp.locus
		goto L41

	L39:
		[int]iftmp.202 := 0
		goto L41

	L41:
		[struct expanded_location]%r56 := expand_location([int]iftmp.202)
		[int]%r57 := [int]%r56.line
		[int]%r58 := [int]t->base.code
		[int]%r59 := [int]%r58
data/slplug.c:431:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r60 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r60 > 3)
		if ([bool]%r3)
			goto L42
		else
			goto L43

	L42:
		[int]%r61 := [int]t->base.code
		[int]%r62 := [int]%r61
data/slplug.c:431:13: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r63 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r3 := ([enum tree_code_class]%r63 <= 10)
		if ([bool]%r3)
			goto L44
		else
			goto L43

	L44:
		[int]iftmp.203 := [int]t->exp.locus
		goto L45

	L43:
		[int]iftmp.203 := 0
		goto L45

	L45:
		[struct expanded_location]%r64 := expand_location([int]iftmp.203)
		[char *]%r65 := [char *]%r64.file
		[char *]%r66 := "%s:%d:%d: warning: '%s' not handled
%s:%d: note: raised from '%s' [internal location]
"
		[struct _IO_FILE *]stderr.204 := [struct _IO_FILE *]stderr
		[struct FILE *]stderr.205 := [struct _IO_FILE *]stderr.204
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fprintf([struct FILE *]stderr.205, [char *]%r66, [char *]%r65, [int]%r57, [int]%r49, "CONSTRUCTOR", "data/slplug.c", 431, [char *]&__FUNCTION__[0])
		[int]verbose.206 := [int]verbose
data/slplug.c:431:13: warning: 'BIT_AND_EXPR' not handled
slplug.c:665: note: raised from 'handle_stmt_binop' [internal location]
		[bool]%r3 := ([int]%r67 != 0)
		if ([bool]%r3)
			goto L46
		else
			goto L47

	L46:
		debug_tree([union tree_node *]t)
		goto L47

	L47:
		goto L8

	L2:
		raise(5)
		goto L8

	L8:
		ret
data/slplug.c:427:13: warning: unused register %r17
data/slplug.c:427:13: warning: unused register %r20
data/slplug.c:427:13: warning: unused register %r25
data/slplug.c:427:13: warning: unused register %r28
data/slplug.c:427:13: warning: unused register %r33
data/slplug.c:427:13: warning: unused register %r36
data/slplug.c:427:13: error: uninitialized register %r41
data/slplug.c:431:13: warning: unused register %r43
data/slplug.c:431:13: warning: unused register %r46
data/slplug.c:431:13: warning: unused register %r51
data/slplug.c:431:13: warning: unused register %r54
data/slplug.c:431:13: warning: unused register %r59
data/slplug.c:431:13: warning: unused register %r62
data/slplug.c:431:13: error: uninitialized register %r67

data/slplug.c:427:13: warning: unused register %r17
data/slplug.c:427:13: warning: unused register %r20
data/slplug.c:427:13: warning: unused register %r25
data/slplug.c:427:13: warning: unused register %r28
data/slplug.c:427:13: warning: unused register %r33
data/slplug.c:427:13: warning: unused register %r36
data/slplug.c:427:13: error: uninitialized register %r41
data/slplug.c:431:13: warning: unused register %r43
data/slplug.c:431:13: warning: unused register %r46
data/slplug.c:431:13: warning: unused register %r51
data/slplug.c:431:13: warning: unused register %r54
data/slplug.c:431:13: warning: unused register %r59
data/slplug.c:431:13: warning: unused register %r62
data/slplug.c:431:13: error: uninitialized register %r67
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-read_operand_decl.dot' [internal location]
static read_operand_decl(%arg1: [struct cl_operand *]op, %arg2: [union tree_node *]t):
		goto L1

	L1:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r1 := [int]__t->base.code
		[int]%r2 := [int]%r1
data/slplug.c:367:33: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r3 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r3 != 1)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 367, [char *]&__FUNCTION__[0])
		abort

	L3:
		[union tree_node *]%r5 := [union tree_node *]__t
		[int]%r6 := [int]%r5->decl_minimal.locus
		[struct cl_location *]%r7 := [struct cl_location *]&op->loc
		read_gcc_location([struct cl_location *]%r7, [int]%r6)
		[union tree_node *]__t := [union tree_node *]t
		[int]%r8 := [int]__t->base.code
		[int]%r9 := [int]%r8
data/slplug.c:370:9: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r10 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r10 != 1)
		if ([bool]%r4)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 370, [char *]&__FUNCTION__[0])
		abort

	L5:
		[union tree_node *]%r11 := [union tree_node *]__t
		[union tree_node *]%r12 := [union tree_node *]%r11->decl_minimal.name
		[bool]%r4 := ([union tree_node *]%r12 != NULL)
		if ([bool]%r4)
			goto L6
		else
			goto L7

	L6:
		[int]%r13 := [int]t->base.code
		[bool]%r4 := ([int]%r13 == 29)
		if ([bool]%r4)
			goto L8
		else
			goto L9

	L8:
		[enum cl_operand_e]op->code := [enum cl_operand_e]1
		[struct cl_type *]op->type := [struct cl_type *]&builtin_fnc_type
		[union tree_node *]__t := [union tree_node *]t
		[int]%r14 := [int]__t->base.code
		[int]%r15 := [int]%r14
data/slplug.c:374:43: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r16 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r16 != 1)
		if ([bool]%r4)
			goto L10
		else
			goto L11

	L10:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 374, [char *]&__FUNCTION__[0])
		abort

	L11:
		[union tree_node *]%r17 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]%r17->decl_minimal.name
		[int]%r18 := [int]__t->base.code
		[bool]%r4 := ([int]%r18 != 1)
		if ([bool]%r4)
			goto L12
		else
			goto L13

	L12:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 374, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L13:
		[union tree_node *]%r19 := [union tree_node *]__t
		[int *]%r20 := [int *]%r19->identifier.id.str
		[char *]%r21 := [int *]%r20
		[char *]op->data.cst_fnc.name := [char *]%r21
		[union tree_node *]__t := [union tree_node *]t
		[int]%r22 := [int]__t->base.code
		[int]%r23 := [int]%r22
data/slplug.c:375:43: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r24 := [int]tree_contains_struct[-1][10]
		[bool]%r4 := ([int]%r24 != 1)
		if ([bool]%r4)
			goto L14
		else
			goto L15

	L14:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]10, "data/slplug.c", 375, [char *]&__FUNCTION__[0])
		abort

	L15:
		[union tree_node *]%r25 := [union tree_node *]__t
		[int]%r26 := [int]%r25->decl_common.decl_flag_1
		[int]%r27 := [int]%r26
		[bool]%r28 := ([int]%r27 != 0)
		[bool]op->data.cst_fnc.is_extern := [bool]%r28
		goto L16

	L9:
		[enum cl_operand_e]op->code := [enum cl_operand_e]2
		[union tree_node *]__t := [union tree_node *]t
		[int]%r29 := [int]__t->base.code
		[int]%r30 := [int]%r29
data/slplug.c:379:43: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r31 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r31 != 1)
		if ([bool]%r4)
			goto L17
		else
			goto L18

	L17:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 379, [char *]&__FUNCTION__[0])
		abort

	L18:
		[union tree_node *]%r32 := [union tree_node *]__t
		[union tree_node *]__t := [union tree_node *]%r32->decl_minimal.name
		[int]%r33 := [int]__t->base.code
		[bool]%r4 := ([int]%r33 != 1)
		if ([bool]%r4)
			goto L19
		else
			goto L20

	L19:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 379, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L20:
		[union tree_node *]%r34 := [union tree_node *]__t
		[int *]%r35 := [int *]%r34->identifier.id.str
		[char *]%r36 := [int *]%r35
		[char *]op->data.var.name := [char *]%r36
		goto L16

	L16:
		goto L21

	L7:
		[enum cl_operand_e]op->code := [enum cl_operand_e]4
		[union tree_node *]__t := [union tree_node *]t
		[int]%r37 := [int]__t->base.code
		[int]%r38 := [int]%r37
data/slplug.c:385:31: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r39 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r39 != 1)
		if ([bool]%r4)
			goto L22
		else
			goto L23

	L22:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 385, [char *]&__FUNCTION__[0])
		abort

	L23:
		[union tree_node *]%r40 := [union tree_node *]__t
		[int]%r41 := [int]%r40->decl_minimal.uid
		[int]%r42 := [int]%r41
		[int]op->data.reg.id := [int]%r42
		goto L21

	L21:
		ret
data/slplug.c:367:33: warning: unused register %r2
data/slplug.c:370:9: warning: unused register %r9
data/slplug.c:374:43: warning: unused register %r15
data/slplug.c:375:43: warning: unused register %r23
data/slplug.c:379:43: warning: unused register %r30
data/slplug.c:385:31: warning: unused register %r38

data/slplug.c:367:33: warning: unused register %r2
data/slplug.c:370:9: warning: unused register %r9
data/slplug.c:374:43: warning: unused register %r15
data/slplug.c:375:43: warning: unused register %r23
data/slplug.c:379:43: warning: unused register %r30
data/slplug.c:385:31: warning: unused register %r38
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-add_type_if_needed.dot' [internal location]
static add_type_if_needed(%arg1: [union tree_node *]t):
		goto L1

	L1:
		[bool]%r1 := ([union tree_node *]t == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		[int]%r2 := (-1)
		goto L4

	L3:
		[union tree_node *]__t := [union tree_node *]t
		[union tree_node **]%r3 := [union tree_node **]&__t->common.type
		[union tree_node *]type := [union tree_node *]*%r3
		[bool]%r1 := ([union tree_node *]type == NULL)
		if ([bool]%r1)
			goto L5
		else
			goto L6

	L5:
		[int]%r2 := (-1)
		goto L4

	L6:
		[union tree_node *]__t := [union tree_node *]type
		[int]%r4 := [int]__t->base.code
		[int]%r5 := [int]%r4
data/slplug.c:331:25: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r6 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r1 := ([enum tree_code_class]%r6 != 2)
		if ([bool]%r1)
			goto L7
		else
			goto L8

	L7:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]2, "data/slplug.c", 331, [char *]&__FUNCTION__[0])
		abort

	L8:
		[union tree_node *]%r7 := [union tree_node *]__t
		[int]%r8 := [int]%r7->type.uid
		[int]uid := [int]%r8
		[struct htab *]type_db.207 := [struct htab *]type_db
		[struct cl_type *]clt := type_db_lookup([struct htab *]type_db.207, [int]uid)
		[bool]%r1 := ([struct cl_type *]clt != NULL)
		if ([bool]%r1)
			goto L9
		else
			goto L10

	L9:
		[int]%r2 := [int]uid
		goto L4

	L10:
		[void *]%r9 := ggc_alloc_cleared_stat(64)
		[struct cl_type *]clt := [void *]%r9
		[int]clt->uid := [int]uid
		[struct htab *]type_db.208 := [struct htab *]type_db
		type_db_insert([struct htab *]type_db.208, [struct cl_type *]clt)
		read_base_type([struct cl_type *]clt, [union tree_node *]type)
		read_specific_type([struct cl_type *]clt, [union tree_node *]type)
		[enum cl_type_e]%r10 := [enum cl_type_e]clt->code
		[bool]%r1 := ([enum cl_type_e]%r10 == 6)
		if ([bool]%r1)
			goto L11
		else
			goto L12

	L11:
		[char *]%r11 := [char *]clt->name
		[bool]%r1 := ([char *]%r11 != NULL)
		if ([bool]%r1)
			goto L13
		else
			goto L12

	L13:
		[char *]%r12 := [char *]clt->name
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[int]%r13 := strcmp("char", [char *]%r12)
		[bool]%r1 := ([int]%r13 == 0)
		if ([bool]%r1)
			goto L14
		else
			goto L12

	L14:
		[enum cl_type_e]clt->code := [enum cl_type_e]7
		goto L12

	L12:
		[int]%r2 := [int]uid
		goto L4

	L4:
		ret [int]%r2
data/slplug.c:331:25: warning: unused register %r5

data/slplug.c:331:25: warning: unused register %r5
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-read_specific_type.dot' [internal location]
static read_specific_type(%arg1: [struct cl_type *]clt, %arg2: [union tree_node *]type):
		goto L1

	L1:
		[int]%r1 := [int]type->base.code
		[enum tree_code]code := [int]%r1
		switch ([enum tree_code]code) {
			default: goto L2
			case 6: goto L3
			case 7: goto L4
			case 8: goto L5
			case 10: goto L6
			case 15: goto L7
			case 16: goto L8
			case 17: goto L9
			case 19: goto L10
			case 20: goto L11
		}

	L10:
		[enum cl_type_e]clt->code := [enum cl_type_e]0
		goto L12

	L6:
		[enum cl_type_e]clt->code := [enum cl_type_e]1
		[int]clt->item_cnt := 1
		[void *]%r2 := ggc_alloc_cleared_stat(16)
		[struct cl_type_item *]%r3 := [void *]%r2
		[struct cl_type_item *]clt->items := [struct cl_type_item *]%r3
		[struct cl_type_item *]%r4 := [struct cl_type_item *]clt->items
		[int]%r5 := add_type_if_needed([union tree_node *]type)
		[int]%r4->type := [int]%r5
		goto L12

	L8:
		[enum cl_type_e]clt->code := [enum cl_type_e]2
		dig_record_type([struct cl_type *]clt, [union tree_node *]type)
		goto L12

	L9:
		[enum cl_type_e]clt->code := [enum cl_type_e]3
		dig_record_type([struct cl_type *]clt, [union tree_node *]type)
		goto L12

	L7:
		[enum cl_type_e]clt->code := [enum cl_type_e]4
		[int]%r6 := get_fixed_array_size([union tree_node *]type)
		[int]clt->item_cnt := [int]%r6
		[void *]%r7 := ggc_alloc_cleared_stat(16)
		[struct cl_type_item *]%r8 := [void *]%r7
		[struct cl_type_item *]clt->items := [struct cl_type_item *]%r8
		[struct cl_type_item *]%r9 := [struct cl_type_item *]clt->items
		[int]%r10 := add_type_if_needed([union tree_node *]type)
		[int]%r9->type := [int]%r10
		goto L12

	L11:
		[enum cl_type_e]clt->code := [enum cl_type_e]5
		goto L12

	L5:
		[enum cl_type_e]clt->code := [enum cl_type_e]6
		goto L12

	L4:
		[enum cl_type_e]clt->code := [enum cl_type_e]8
		goto L12

	L3:
		[enum cl_type_e]clt->code := [enum cl_type_e]9
		goto L12

	L2:
		raise(5)
		goto L12

	L12:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-dig_record_type.dot' [internal location]
static dig_record_type(%arg1: [struct cl_type *]clt, %arg2: [union tree_node *]t):
		goto L1

	L1:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r1 := [int]__t->base.code
		[bool]%r2 := ([int]%r1 != 16)
		if ([bool]%r2)
			goto L2
		else
			goto L3

	L2:
		[int]%r3 := [int]__t->base.code
		[bool]%r2 := ([int]%r3 != 17)
		if ([bool]%r2)
			goto L4
		else
			goto L3

	L4:
		[int]%r4 := [int]__t->base.code
		[bool]%r2 := ([int]%r4 != 18)
		if ([bool]%r2)
			goto L5
		else
			goto L3

	L5:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 251, [char *]&__FUNCTION__[0], 16, 17, 18, 0)
		abort

	L3:
		[union tree_node *]%r5 := [union tree_node *]__t
		[union tree_node *]t := [union tree_node *]%r5->type.values
		goto L6

	L7:
		[int]%r6 := [int]clt->item_cnt
		[int]%r7 := ([int]%r6 + 1)
		[int]%r8 := [int]%r7
data/slplug.c:254:22: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
		[struct cl_type_item *]%r9 := [struct cl_type_item *]clt->items
		[void *]%r10 := ggc_realloc_stat([struct cl_type_item *]%r9, [int]%r11)
		[struct cl_type_item *]%r12 := [void *]%r10
		[struct cl_type_item *]clt->items := [struct cl_type_item *]%r12
		[struct cl_type_item *]%r13 := [struct cl_type_item *]clt->items
		[int]%r14 := [int]clt->item_cnt
		[int]%r15 := [int]%r14
data/slplug.c:257:48: warning: 'MULT_EXPR' not handled
slplug.c:672: note: raised from 'handle_stmt_binop' [internal location]
data/slplug.c:257:30: warning: 'POINTER_PLUS_EXPR' not handled
slplug.c:673: note: raised from 'handle_stmt_binop' [internal location]
		[int]%r16 := ([int]%r14 + 1)
		[int]clt->item_cnt := [int]%r16
		[int]%r17 := add_type_if_needed([union tree_node *]t)
		[int]item->type := [int]%r17
		[char *]item->name := NULL
		[union tree_node *]__t := [union tree_node *]t
		[int]%r18 := [int]__t->base.code
		[int]%r19 := [int]%r18
data/slplug.c:261:21: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r20 := [int]tree_contains_struct[-1][9]
		[bool]%r2 := ([int]%r20 != 1)
		if ([bool]%r2)
			goto L8
		else
			goto L9

	L8:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 261, [char *]&__FUNCTION__[0])
		abort

	L9:
		[union tree_node *]%r21 := [union tree_node *]__t
		[union tree_node *]name := [union tree_node *]%r21->decl_minimal.name
		[bool]%r2 := ([union tree_node *]name != NULL)
		if ([bool]%r2)
			goto L10
		else
			goto L11

	L10:
		[union tree_node *]__t := [union tree_node *]name
		[int]%r22 := [int]__t->base.code
		[bool]%r2 := ([int]%r22 != 1)
		if ([bool]%r2)
			goto L12
		else
			goto L13

	L12:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 263, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L13:
		[union tree_node *]%r23 := [union tree_node *]__t
		[int *]%r24 := [int *]%r23->identifier.id.str
		[char *]%r25 := [int *]%r24
		[char *]item->name := [char *]%r25
		goto L11

	L11:
		[union tree_node *]__t := [union tree_node *]t
		[union tree_node **]%r26 := [union tree_node **]&__t->common.chain
		[union tree_node *]t := [union tree_node *]*%r26
		goto L6

	L6:
		[bool]%r2 := ([union tree_node *]t != NULL)
		if ([bool]%r2)
			goto L7
		else
			goto L14

	L14:
		ret
data/slplug.c:254:22: warning: unused register %r8
data/slplug.c:254:22: error: uninitialized register %r11
data/slplug.c:257:41: warning: unused register %r13
data/slplug.c:257:37: warning: unused register %r15
data/slplug.c:261:21: warning: unused register %r19

data/slplug.c:254:22: warning: unused register %r8
data/slplug.c:254:22: error: uninitialized register %r11
data/slplug.c:257:41: warning: unused register %r13
data/slplug.c:257:37: warning: unused register %r15
data/slplug.c:261:21: warning: unused register %r19
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-get_fixed_array_size.dot' [internal location]
static get_fixed_array_size(%arg1: [union tree_node *]t):
		goto L1

	L1:
		[bool]%r1 := ([union tree_node *]t == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
		[int]%r2 := 0
		goto L4

	L3:
		[union tree_node *]__t := [union tree_node *]t
		[union tree_node **]%r3 := [union tree_node **]&__t->common.type
		[union tree_node *]type := [union tree_node *]*%r3
		[bool]%r1 := ([union tree_node *]type == NULL)
		if ([bool]%r1)
			goto L5
		else
			goto L6

	L5:
		[int]%r2 := 0
		goto L4

	L6:
		[int]item_size := get_type_sizeof([union tree_node *]type)
		[bool]%r1 := ([int]item_size == 0)
		if ([bool]%r1)
			goto L7
		else
			goto L8

	L7:
		[int]%r2 := 0
		goto L4

	L8:
		[int]%r4 := get_type_sizeof([union tree_node *]t)
		[int]%r2 := ([int]%r4 / [int]item_size)
		goto L4

	L4:
		ret [int]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-read_base_type.dot' [internal location]
static read_base_type(%arg1: [struct cl_type *]clt, %arg2: [union tree_node *]type):
		goto L1

	L1:
		[int]%r1 := get_type_sizeof([union tree_node *]type)
		[int]clt->size := [int]%r1
		[union tree_node *]__t := [union tree_node *]type
		[int]%r2 := [int]__t->base.code
		[int]%r3 := [int]%r2
data/slplug.c:213:17: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r4 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r5 := ([enum tree_code_class]%r4 != 2)
		if ([bool]%r5)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]2, "data/slplug.c", 213, [char *]&__FUNCTION__[0])
		abort

	L3:
		[union tree_node *]%r6 := [union tree_node *]__t
		[union tree_node *]name := [union tree_node *]%r6->type.name
		[bool]%r5 := ([union tree_node *]name == NULL)
		if ([bool]%r5)
			goto L4
		else
			goto L5

	L4:
		goto L6

	L5:
		[int]%r7 := [int]name->base.code
		[bool]%r5 := ([int]%r7 == 1)
		if ([bool]%r5)
			goto L7
		else
			goto L8

	L7:
		[union tree_node *]__t := [union tree_node *]name
		[int]%r8 := [int]__t->base.code
		[bool]%r5 := ([int]%r8 != 1)
		if ([bool]%r5)
			goto L9
		else
			goto L10

	L9:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 219, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L10:
		[union tree_node *]%r9 := [union tree_node *]__t
		[int *]%r10 := [int *]%r9->identifier.id.str
		[char *]%r11 := [int *]%r10
		[char *]clt->name := [char *]%r11
		goto L6

	L8:
		[union tree_node *]__t := [union tree_node *]name
		[int]%r12 := [int]__t->base.code
		[int]%r13 := [int]%r12
data/slplug.c:222:38: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r14 := [int]tree_contains_struct[-1][9]
		[bool]%r5 := ([int]%r14 != 1)
		if ([bool]%r5)
			goto L11
		else
			goto L12

	L11:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 222, [char *]&__FUNCTION__[0])
		abort

	L12:
		[union tree_node *]%r15 := [union tree_node *]__t
		[int]%r16 := [int]%r15->decl_minimal.locus
		[struct cl_location *]%r17 := [struct cl_location *]&clt->loc
		read_gcc_location([struct cl_location *]%r17, [int]%r16)
		[union tree_node *]__t := [union tree_node *]name
		[int]%r18 := [int]__t->base.code
		[int]%r19 := [int]%r18
data/slplug.c:223:16: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r20 := [int]tree_contains_struct[-1][9]
		[bool]%r5 := ([int]%r20 != 1)
		if ([bool]%r5)
			goto L13
		else
			goto L14

	L13:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 223, [char *]&__FUNCTION__[0])
		abort

	L14:
		[union tree_node *]%r21 := [union tree_node *]__t
		[union tree_node *]name := [union tree_node *]%r21->decl_minimal.name
		[bool]%r5 := ([union tree_node *]name != NULL)
		if ([bool]%r5)
			goto L15
		else
			goto L6

	L15:
		[union tree_node *]__t := [union tree_node *]name
		[int]%r22 := [int]__t->base.code
		[bool]%r5 := ([int]%r22 != 1)
		if ([bool]%r5)
			goto L16
		else
			goto L17

	L16:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 225, [char *]&__FUNCTION__[0], 1, 0)
		abort

	L17:
		[union tree_node *]%r23 := [union tree_node *]__t
		[int *]%r24 := [int *]%r23->identifier.id.str
		[char *]%r25 := [int *]%r24
		[char *]clt->name := [char *]%r25
		goto L6

	L6:
		ret
data/slplug.c:213:17: warning: unused register %r3
data/slplug.c:222:38: warning: unused register %r13
data/slplug.c:223:16: warning: unused register %r19

data/slplug.c:213:17: warning: unused register %r3
data/slplug.c:222:38: warning: unused register %r13
data/slplug.c:223:16: warning: unused register %r19
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-get_type_sizeof.dot' [internal location]
static get_type_sizeof(%arg1: [union tree_node *]t):
		goto L1

	L1:
		[union tree_node *]__t := [union tree_node *]t
		[int]%r1 := [int]__t->base.code
		[int]%r2 := [int]%r1
data/slplug.c:198:17: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[enum tree_code_class]%r3 := [enum tree_code_class]tree_code_type[-1]
		[bool]%r4 := ([enum tree_code_class]%r3 != 2)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_class_check_failed([union tree_node *]__t, [enum tree_code_class]2, "data/slplug.c", 198, [char *]&__FUNCTION__[0])
		abort

	L3:
		[union tree_node *]%r5 := [union tree_node *]__t
		[union tree_node *]size := [union tree_node *]%r5->type.size_unit
		[bool]%r4 := ([union tree_node *]size == NULL)
		if ([bool]%r4)
			goto L4
		else
			goto L5

	L4:
		[int]%r6 := 0
		goto L6

	L5:
		[union tree_node *]__t := [union tree_node *]size
		[int]%r7 := [int]__t->base.code
		[bool]%r4 := ([int]%r7 != 23)
		if ([bool]%r4)
			goto L7
		else
			goto L8

	L7:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 202, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L8:
		[union tree_node *]%r8 := [union tree_node *]__t
		[int]%r9 := [int]%r8->int_cst.int_cst.high
		[bool]%r4 := ([int]%r9 != 0)
		if ([bool]%r4)
			goto L9
		else
			goto L10

	L9:
		raise(5)
		goto L10

	L10:
		[union tree_node *]__t := [union tree_node *]size
		[int]%r10 := [int]__t->base.code
		[bool]%r4 := ([int]%r10 != 23)
		if ([bool]%r4)
			goto L11
		else
			goto L12

	L11:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_check_failed([union tree_node *]__t, "data/slplug.c", 205, [char *]&__FUNCTION__[0], 23, 0)
		abort

	L12:
		[union tree_node *]%r11 := [union tree_node *]__t
		[int]%r12 := [int]%r11->int_cst.int_cst.low
		[int]%r6 := [int]%r12
		goto L6

	L6:
		ret [int]%r6
data/slplug.c:198:17: warning: unused register %r2

data/slplug.c:198:17: warning: unused register %r2
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-index_to_label.dot' [internal location]
static index_to_label(%arg1: [int]idx):
		goto L1

	L1:
		[union tree_node *]__t := [union tree_node *]current_function_decl
		[int]%r1 := [int]__t->base.code
		[int]%r2 := [int]%r1
data/slplug.c:191:40: warning: 'non-constant array accessor' not handled
slplug.c:495: note: raised from 'handle_accessor_array_ref' [internal location]
		[int]%r3 := [int]tree_contains_struct[-1][9]
		[bool]%r4 := ([int]%r3 != 1)
		if ([bool]%r4)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		tree_contains_struct_check_failed([union tree_node *]__t, [enum tree_node_structure_enum]9, "data/slplug.c", 191, [char *]&__FUNCTION__[0])
		abort

	L3:
		[union tree_node *]%r5 := [union tree_node *]__t
		[int]%r6 := [int]%r5->decl_minimal.uid
		[char *]%r7 := "%u:%u"
		[int]rv := asprintf([char **]&label, [char *]%r7, [int]%r6, [int]idx)
		[bool]%r4 := ([int]rv <= 0)
		if ([bool]%r4)
			goto L4
		else
			goto L5

	L4:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 192, [char *]&__FUNCTION__[0])
		abort

	L5:
		[char *]%r8 := [char *]label
		ret [char *]%r8
data/slplug.c:191:40: warning: unused register %r2

data/slplug.c:191:40: warning: unused register %r2
cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-read_gimple_location.dot' [internal location]
static read_gimple_location(%arg1: [struct cl_location *]loc, %arg2: [union gimple_statement_d *]g):
		goto L1

	L1:
		[int]%r1 := [int]g->gsbase.location
		read_gcc_location([struct cl_location *]loc, [int]%r1)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-read_gcc_location.dot' [internal location]
static read_gcc_location(%arg1: [struct cl_location *]loc, %arg2: [int]gcc_loc):
		goto L1

	L1:
		[struct expanded_location]exp_loc := expand_location([int]gcc_loc)
		[char *]%r1 := [char *]exp_loc.file
		[char *]loc->file := [char *]%r1
		[int]%r2 := [int]exp_loc.line
		[int]loc->line := [int]%r2
		[int]%r3 := [int]exp_loc.column
		[int]loc->column := [int]%r3
		[bool]%r4 := [bool]exp_loc.sysp
		[bool]loc->sysp := [bool]%r4
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-register_type_db.dot' [internal location]
static register_type_db(%arg1: [struct cl_code_listener *]cl, %arg2: [struct htab *]db):
		goto L1

	L1:
		[fnc *]%r1 := [fnc *]cl->reg_type_db
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[fnc *]%r1([struct cl_code_listener *]cl, cb_type_db_lookup, [struct htab *]db)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-cb_type_db_lookup.dot' [internal location]
static cb_type_db_lookup(%arg1: [int]uid, %arg2: [void *]user_data):
		goto L1

	L1:
		[struct htab *]db := [void *]user_data
		[struct cl_type *]%r1 := type_db_lookup([struct htab *]db, [int]uid)
		ret [struct cl_type *]%r1

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_insert.dot' [internal location]
static type_db_insert(%arg1: [struct htab *]db, %arg2: [struct cl_type *]type):
		goto L1

	L1:
		[void **]slot := htab_find_slot([struct htab *]db, [struct cl_type *]type, [enum insert_option]1)
		[bool]%r1 := ([void **]slot == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 160, [char *]&__FUNCTION__[0])
		abort

	L3:
		[void *]*slot := [struct cl_type *]type
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_lookup.dot' [internal location]
static type_db_lookup(%arg1: [struct htab *]db, %arg2: [int]uid):
		goto L1

	L1:
		[int]type.uid := [int]uid
		[void *]%r1 := htab_find([struct htab *]db, [struct cl_type *]&type)
		[struct cl_type *]%r2 := [void *]%r1
		ret [struct cl_type *]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_destroy.dot' [internal location]
static type_db_destroy(%arg1: [struct htab *]db):
		goto L1

	L1:
		htab_delete([struct htab *]db)
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_create.dot' [internal location]
static type_db_create():
		goto L1

	L1:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		[struct htab *]db := htab_create_alloc(256, type_db_hash, type_db_eq, type_db_free, xcalloc, free)
		[bool]%r1 := ([struct htab *]db == NULL)
		if ([bool]%r1)
			goto L2
		else
			goto L3

	L2:
cl_pp.cc:429: debug: CL_OPERAND_CST with op->accessor [internal location]
		fancy_abort("data/slplug.c", 140, [char *]&__FUNCTION__[0])
		abort

	L3:
		[struct htab *]%r2 := [struct htab *]db
		ret [struct htab *]%r2

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_free.dot' [internal location]
static type_db_free(%arg1: [void *]p):
		goto L1

	L1:
		ret

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_eq.dot' [internal location]
static type_db_eq(%arg1: [void *]p1, %arg2: [void *]p2):
		goto L1

	L1:
		[struct cl_type *]type1 := [void *]p1
		[struct cl_type *]type2 := [void *]p2
		[int]%r1 := [int]type1->uid
		[int]%r2 := [int]type2->uid
		[int]%r3 := ([int]%r1 == [int]%r2)
		ret [int]%r3

cl_dotgen.cc:196: debug: ClDotGenerator: created dot file 'data/slplug.c-type_db_hash.dot' [internal location]
static type_db_hash(%arg1: [void *]p):
		goto L1

	L1:
		[struct cl_type *]type := [void *]p
		[int]%r1 := [int]type->uid
		[int]%r2 := [int]%r1
		ret [int]%r2

slplug.c:1200: ./slplug.so: cb_finish_unit

slplug.c:1178: ./slplug.so: cb_finish
