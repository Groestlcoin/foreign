/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/err.h>
#include <openssl/randerr.h>

#ifndef OPENSSL_NO_ERR

static const ERR_STRING_DATA RAND_str_reasons[] = {
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ADDITIONAL_INPUT_TOO_LONG),
    "additional input too long"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ALREADY_INSTANTIATED),
    "already instantiated"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ARGUMENT_OUT_OF_RANGE),
    "argument out of range"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_CANNOT_OPEN_FILE), "Cannot open file"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_DERIVATION_FUNCTION_MANDATORY_FOR_FIPS),
    "derivation function mandatory for fips"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_DRBG_ALREADY_INITIALIZED),
    "drbg already initialized"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_DRBG_NOT_INITIALISED),
    "drbg not initialised"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ENTROPY_INPUT_TOO_LONG),
    "entropy input too long"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ENTROPY_OUT_OF_RANGE),
    "entropy out of range"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ERROR_ENTROPY_POOL_WAS_IGNORED),
    "error entropy pool was ignored"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ERROR_INITIALISING_DRBG),
    "error initialising drbg"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ERROR_INSTANTIATING_DRBG),
    "error instantiating drbg"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ERROR_RETRIEVING_ADDITIONAL_INPUT),
    "error retrieving additional input"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ERROR_RETRIEVING_ENTROPY),
    "error retrieving entropy"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_ERROR_RETRIEVING_NONCE),
    "error retrieving nonce"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_FAILED_TO_CREATE_LOCK),
    "failed to create lock"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_FUNC_NOT_IMPLEMENTED),
    "Function not implemented"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_FWRITE_ERROR), "Error writing file"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_GENERATE_ERROR), "generate error"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_INTERNAL_ERROR), "internal error"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_IN_ERROR_STATE), "in error state"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_NOT_A_REGULAR_FILE),
    "Not a regular file"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_NOT_INSTANTIATED), "not instantiated"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_NO_DRBG_IMPLEMENTATION_SELECTED),
    "no drbg implementation selected"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_PARENT_LOCKING_NOT_ENABLED),
    "parent locking not enabled"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_PARENT_STRENGTH_TOO_WEAK),
    "parent strength too weak"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_PERSONALISATION_STRING_TOO_LONG),
    "personalisation string too long"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_PREDICTION_RESISTANCE_NOT_SUPPORTED),
    "prediction resistance not supported"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_PRNG_NOT_SEEDED), "PRNG not seeded"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_RANDOM_POOL_OVERFLOW),
    "random pool overflow"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_RANDOM_POOL_UNDERFLOW),
    "random pool underflow"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_REQUEST_TOO_LARGE_FOR_DRBG),
    "request too large for drbg"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_RESEED_ERROR), "reseed error"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_SELFTEST_FAILURE), "selftest failure"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_TOO_LITTLE_NONCE_REQUESTED),
    "too little nonce requested"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_TOO_MUCH_NONCE_REQUESTED),
    "too much nonce requested"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_UNSUPPORTED_DRBG_FLAGS),
    "unsupported drbg flags"},
    {ERR_PACK(ERR_LIB_RAND, 0, RAND_R_UNSUPPORTED_DRBG_TYPE),
    "unsupported drbg type"},
    {0, NULL}
};

#endif

int ERR_load_RAND_strings(void)
{
#ifndef OPENSSL_NO_ERR
    if (ERR_func_error_string(RAND_str_reasons[0].error) == NULL)
        ERR_load_strings_const(RAND_str_reasons);
#endif
    return 1;
}
