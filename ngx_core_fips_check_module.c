/*
 * Copyright (C) Owen Garrett
 *
 * License: Apache2
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include <openssl/crypto.h>

#if (OPENSSL_VERSION_NUMBER >= 0x30000000L && !defined FIPS_mode)
# define FIPS_mode() EVP_default_properties_is_fips_enabled(NULL)
#endif

typedef enum {
    UNKNOWN,
    DISABLED,
    ENABLED
} fips_state_t;

static fips_state_t fips_state = UNKNOWN;


static ngx_int_t ngx_fips_check(ngx_cycle_t *cycle);

static ngx_core_module_t ngx_fips_check_module_ctx = {
    ngx_string("fips_check"),
    NULL,
    NULL
};


ngx_module_t ngx_fips_check_module = {
    NGX_MODULE_V1,
    &ngx_fips_check_module_ctx,            /* module context */
    NULL,                                  /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    ngx_fips_check,                        /* init module */
    ngx_fips_check,                        /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_fips_check(ngx_cycle_t *cycle)
{
    int mode = FIPS_mode();

    //ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "Checking FIPS mode: %d, %d", fips_state, mode);

    // First time we run this check, at init master
    if( fips_state == UNKNOWN ) {
        if( mode == 0 ) {
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "OpenSSL FIPS Mode is not enabled");
            fips_state = DISABLED;
        } else {
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "OpenSSL FIPS Mode is enabled");
            fips_state = ENABLED;
        }
        return NGX_OK;
    }

    // Subsequent checks, at init worker
    // These checks are probably not necessary, as it should not be possible to switch the 
    // FIPS state once OpenSSL has been initialized in master, but are included out of 
    // caution

    if( fips_state == DISABLED && mode != 0 ) {
        ngx_log_abort( 0, "EMERG: OpenSSL FIPS mode was not enabled at startup, but is unexpectedly enabled" );
        return NGX_ERROR;
    }

    if( fips_state == ENABLED && mode == 0 ) {
        ngx_log_abort( 0, "EMERG: OpenSSL FIPS mode was enabled at startup, but is unexpectedly disabled" );
        return NGX_ERROR;
    }

    return NGX_OK;
}
