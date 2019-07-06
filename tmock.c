/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_tmock.h"



#define tmmalloc    malloc
#define tmcalloc    calloc
#define tmrealloc   realloc
#define tmfree      free
#define tmstrdup    strdup

#define TMOCK_CALLBACK_BLOCK    1
#define TMOCK_CALLBACK_SKIP     0

#define TMOCK_EXECUTE_INTERNAL  1
#define TMOCK_EXECUTE_EXTERNAL  2

/* If you declare any globals in php_tmock.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(tmock)
*/
ZEND_DECLARE_MODULE_GLOBALS(tmock)

/* True global resources - no need for thread safety here */
static int le_tmock;


void tmock_clean();

void (*tmock_old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void tmock_execute_ex(zend_execute_data *execute_data TSRMLS_DC);

void (*tmock_old_execute_internal)(zend_execute_data *current_execute_data, zval *return_value);
void tmock_execute_internal(zend_execute_data *current_execute_data, zval *return_value);

int tmock_execute_ex_main(zend_execute_data *zdata, zval *return_value, int type TSRMLS_DC);

char *tmock_get_fname(zend_execute_data *edata);

int tmock_function_name_is_closure(char *fname);

char *tmock_sprintf(const char* fmt, ...);

int tmock_get_parameters_array_ex(zval *argument_array, zend_execute_data *edata TSRMLS_DC);

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("tmock.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_tmock_globals, tmock_globals)
    STD_PHP_INI_ENTRY("tmock.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_tmock_globals, tmock_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_tmock_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_tmock_compiled)
{
    char *arg = NULL;
    size_t arg_len, len;
    zend_string *strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
        return;
    }

    strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "tmock", arg);

    RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/

PHP_FUNCTION(tmock_start)
{
    zval *arr, *mock_array, mock_array_init;
    int num;


    zval *entry;
    zend_string *string_key;
    zend_ulong num_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &arr) == FAILURE) {
        RETURN_BOOL(0)
        return ;
    }

    tmock_clean();

    array_init(&mock_array_init);
    ZVAL_NEW_REF(&TMOCK_G(mock_array), &mock_array_init);

    mock_array = Z_REFVAL(TMOCK_G(mock_array));

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), num_key, string_key, entry) {
        if (string_key) {
            entry = zend_hash_add_new(Z_ARRVAL_P(mock_array), string_key, entry);
        } else {
            entry = zend_hash_index_add_new(Z_ARRVAL_P(mock_array), num_key, entry);
        }
        zval_add_ref(entry);
    } ZEND_HASH_FOREACH_END();

    if (!TMOCK_G(do_mock_functions)) {

        // tmock_old_execute_ex = zend_execute_ex;
        // zend_execute_ex = tmock_execute_ex;

        // tmock_old_execute_internal = zend_execute_internal;
        // zend_execute_internal = tmock_execute_internal;

        TMOCK_G(do_mock_functions) = 1;
    }

    RETURN_BOOL(1);
}

PHP_FUNCTION(tmock_get)
{
    zval *arr, *mock_array;
    int num;


    zval *entry;
    zend_string *string_key;
    zend_ulong num_key;

    mock_array = Z_REFVAL(TMOCK_G(mock_array));
    if (Z_ISREF_P(&TMOCK_G(mock_array)) && Z_TYPE_P(Z_REFVAL(TMOCK_G(mock_array))) == IS_ARRAY) {
        array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(mock_array)));
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(mock_array), num_key, string_key, entry) {
            if (string_key) {
                entry = zend_hash_add_new(Z_ARRVAL_P(return_value), string_key, entry);
            } else {
                entry = zend_hash_index_add_new(Z_ARRVAL_P(return_value), num_key, entry);
            }
            zval_add_ref(entry);
        } ZEND_HASH_FOREACH_END();
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(tmock_end)
{
    tmock_clean();

    if (TMOCK_G(do_mock_functions)) {

        // zend_execute_ex = tmock_old_execute_ex;
        // tmock_old_execute_ex = NULL;

        // zend_execute_internal = tmock_old_execute_internal;
        // tmock_old_execute_internal = NULL;

        TMOCK_G(do_mock_functions) = 0;
    }

    RETURN_BOOL(1);
}

PHP_FUNCTION(tmock_debug_start)
{
    if (!TMOCK_G(debug_mock_functions)) {
        TMOCK_G(debug_mock_functions) = 1;
    }

    RETURN_BOOL(1);
}

PHP_FUNCTION(tmock_debug_end)
{
    if (TMOCK_G(debug_mock_functions)) {
        TMOCK_G(debug_mock_functions) = 0;
    }

    RETURN_BOOL(1);
}


void tmock_clean()
{
    if (Z_ISREF_P(&TMOCK_G(mock_array)) && Z_TYPE_P(Z_REFVAL(TMOCK_G(mock_array))) == IS_ARRAY) {
        zval *sess_var = Z_REFVAL(TMOCK_G(mock_array));
        SEPARATE_ARRAY(sess_var);
        zend_hash_clean(Z_ARRVAL_P(sess_var));
    }
}


void tmock_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
    if (tmock_execute_ex_main(execute_data, NULL, TMOCK_EXECUTE_EXTERNAL TSRMLS_CC) == TMOCK_CALLBACK_SKIP) {
        tmock_old_execute_ex(execute_data TSRMLS_CC);
    }
}


void tmock_execute_internal(zend_execute_data *current_execute_data, zval *return_value)
{
    if (tmock_execute_ex_main(current_execute_data, return_value, TMOCK_EXECUTE_INTERNAL TSRMLS_CC) == TMOCK_CALLBACK_SKIP) {
        if (tmock_old_execute_internal) {
            tmock_old_execute_internal(current_execute_data, return_value TSRMLS_CC);
        } else {
            execute_internal(current_execute_data, return_value TSRMLS_CC);
        }
    }
}


int tmock_execute_ex_main(zend_execute_data *edata, zval *return_value, int type TSRMLS_DC)
{
    if (
        TMOCK_G(do_mock_functions)
        && !TMOCK_G(in_mock_callback)
        && edata && edata->func
    ) {

        char *fname = tmock_get_fname(edata);

        zval *mock_array;
        zval *callback;
        zend_string *sfname = strpprintf(0, "%s", fname);

        if (TMOCK_G(debug_mock_functions)) {
            php_printf("fname %s, type %d\n", fname, type);
        }


        mock_array = Z_REFVAL(TMOCK_G(mock_array));


        if (
            Z_ISREF_P(&TMOCK_G(mock_array)) && Z_TYPE_P(Z_REFVAL(TMOCK_G(mock_array))) == IS_ARRAY
            && zend_hash_num_elements(Z_ARRVAL_P(mock_array)) > 0
            && (callback = zend_hash_find(Z_ARRVAL_P(mock_array), sfname))
        ) {
            if (TMOCK_G(debug_mock_functions)) {
                php_printf("fname %s match\n", fname);
            }

            zend_string *callback_name;
            if (zend_is_callable(callback, 0, &callback_name)) {
                if (TMOCK_G(debug_mock_functions)) {
                    php_printf("fname %s is_callable\n", fname);
                }

                zval params[3];
                ZVAL_STR_COPY(&params[0], sfname); // 函数名v
                tmock_get_parameters_array_ex(&params[1], edata TSRMLS_CC);
                if (edata->This.value.obj) {
                    // ZVAL_OBJ(&params[2], edata->This.value.obj); // 类
                    ZVAL_COPY(&params[2], &edata->This);
                } else {
                    ZVAL_NULL(&params[2]);
                }


                zval retval;
                TMOCK_G(in_mock_callback) = 1;


                zend_fcall_info fci;
                zend_fcall_info_cache fcc;

                // fci.size = sizeof(fci);
                // fci.object = NULL;
                // ZVAL_COPY_VALUE(&fci.function_name, callback);
                // fci.retval = &retval;
                // fci.param_count = 3;
                // fci.params = params;
                // fci.no_separation = 0;
                // if (fci.symbol_table) {
                //     fci.symbol_table = NULL;
                // }


                if (zend_fcall_info_init(callback, 0, &fci, &fcc, NULL, NULL) == SUCCESS) {
                    if (TMOCK_G(debug_mock_functions)) {
                        php_printf("fname %s fcall init SUCCESS\n", fname);
                    }
                    fci.retval = &retval;
                    fci.param_count = 3;
                    fci.params = params;
                    

                    // if (call_user_function_ex(EG(function_table), NULL, callback, &retval, 3, params, 0, NULL) == SUCCESS) {
                    if (zend_call_function(&fci, NULL) == SUCCESS) {
                        if (TMOCK_G(debug_mock_functions)) {
                            php_printf("fname %s callable SUCCESS\n", fname);
                        }
                        if (type == TMOCK_EXECUTE_EXTERNAL) {
                            ZVAL_COPY_VALUE((edata)->return_value, &retval);
                        } else {
                            ZVAL_COPY_VALUE(return_value, &retval);
                        }
                    }
                    TMOCK_G(in_mock_callback) = 0;
                    return TMOCK_CALLBACK_BLOCK;
                }

            }
        }
    }

    return TMOCK_CALLBACK_SKIP;
}


char *tmock_get_fname(zend_execute_data *edata)
{
    char *class;
    char *function;
    char *fname;

    if (
        edata->func->common.function_name
        && !tmock_function_name_is_closure(edata->func->common.function_name->val)
    ) {
        function = tmstrdup(edata->func->common.function_name->val);
    }

    if (edata->This.value.obj && edata->This.value.obj->ce) {
        class = tmstrdup(edata->This.value.obj->ce->name->val);
        fname = (tmock_sprintf("%s%s%s",
                        class ? class : "?",
                        "->",
                        function ? function : "?"
                    ));
    } else if (edata->func->common.scope) {
        class = tmstrdup(edata->func->common.scope->name->val);
        fname = (tmock_sprintf("%s%s%s",
                        class ? class : "?",
                        "::",
                        function ? function : "?"
                    ));
    } else {
        fname = tmstrdup(function ? function : "?");
    }

    return fname;
}


int tmock_function_name_is_closure(char *fname)
{
    int length = strlen(fname);
    int closure_length = strlen("{closure}");

    if (length < closure_length) {
        return 0;
    }

    if (strcmp(fname + length - closure_length, "{closure}") == 0) {
        return 1;
    }

    return 0;
}

char *tmock_sprintf(const char* fmt, ...)
{
    char   *new_str;
    int     size = 1;
    va_list args;

    new_str = (char *) tmmalloc(size);

    for (;;) {
        int n;

        va_start(args, fmt);
        n = vsnprintf(new_str, size, fmt, args);
        va_end(args);

        if (n > -1 && n < size) {
            break;
        }
        if (n < 0) {
            size *= 2;
        } else {
            size = n + 1;
        }
        new_str = (char *) tmrealloc(new_str, size);
    }

    return new_str;
}



int tmock_get_parameters_array_ex(zval *argument_array, zend_execute_data *edata TSRMLS_DC)
{
    zval *p, *q;
    uint32_t arg_count, first_extra_arg;
    uint32_t i;

    arg_count = ZEND_CALL_NUM_ARGS(edata);
    
    array_init_size(argument_array, arg_count);
    if (arg_count) {
        first_extra_arg = edata->func->op_array.num_args;
        zend_hash_real_init(Z_ARRVAL_P(argument_array), 1);
        ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(argument_array)) {
            i = 0;
            p = ZEND_CALL_ARG(edata, 1);
            if (arg_count > first_extra_arg) {
                while (i < first_extra_arg) {
                    q = p;
                    if (EXPECTED(Z_TYPE_INFO_P(q) != IS_UNDEF)) {
                        // ZVAL_DEREF(q);
                        if (Z_OPT_REFCOUNTED_P(q)) { 
                            Z_ADDREF_P(q);
                        }
                    } else {
                        q = &EG(uninitialized_zval);
                    }
                    ZEND_HASH_FILL_ADD(q);
                    p++;
                    i++;
                }
                p = ZEND_CALL_VAR_NUM(edata, edata->func->op_array.last_var + edata->func->op_array.T);
            }
            while (i < arg_count) {
                q = p;
                if (EXPECTED(Z_TYPE_INFO_P(q) != IS_UNDEF)) {
                    // ZVAL_DEREF(q);
                    if (Z_OPT_REFCOUNTED_P(q)) { 
                        Z_ADDREF_P(q);
                    }
                } else {
                    q = &EG(uninitialized_zval);
                }
                ZEND_HASH_FILL_ADD(q);
                p++;
                i++;
            }
        } ZEND_HASH_FILL_END();
        Z_ARRVAL_P(argument_array)->nNumOfElements = arg_count;
    }

    return SUCCESS;
}


/* {{{ php_tmock_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_tmock_init_globals(zend_tmock_globals *tmock_globals)
{
    tmock_globals->global_value = 0;
    tmock_globals->global_string = NULL;
}
*/
/* }}} */
static void php_tmock_init_globals(zend_tmock_globals *tmock_globals)
{
    tmock_globals->in_mock_callback = 0;
    tmock_globals->do_mock_functions = 0;
    tmock_globals->debug_mock_functions = 0;
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(tmock)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */

    tmock_old_execute_ex = zend_execute_ex;
    zend_execute_ex = tmock_execute_ex;

    tmock_old_execute_internal = zend_execute_internal;
    zend_execute_internal = tmock_execute_internal;

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(tmock)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(tmock)
{
#if defined(COMPILE_DL_TMOCK) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(tmock)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(tmock)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "tmock support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ tmock_functions[]
 *
 * Every user visible function must have an entry in tmock_functions[].
 */
const zend_function_entry tmock_functions[] = {
    PHP_FE(confirm_tmock_compiled,  NULL)       /* For testing, remove later. */
    PHP_FE(tmock_start, NULL)
    PHP_FE(tmock_get, NULL)
    PHP_FE(tmock_end, NULL)
    PHP_FE(tmock_debug_start, NULL)
    PHP_FE(tmock_debug_end, NULL)
    PHP_FE_END  /* Must be the last line in tmock_functions[] */
};
/* }}} */

/* {{{ tmock_module_entry
 */
zend_module_entry tmock_module_entry = {
    STANDARD_MODULE_HEADER,
    "tmock",
    tmock_functions,
    PHP_MINIT(tmock),
    PHP_MSHUTDOWN(tmock),
    PHP_RINIT(tmock),       /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(tmock),   /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(tmock),
    PHP_TMOCK_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TMOCK
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(tmock)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
