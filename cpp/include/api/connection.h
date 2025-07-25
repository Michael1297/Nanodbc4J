﻿#pragma once
#include <nanodbc/nanodbc.h>
#include "struct/error_info.h"

extern "C" { 

    nanodbc::connection* connection(const char16_t* connection_string, NativeError* error);

    nanodbc::connection* connection_with_timeout(const char16_t* connection_string, long timeout, NativeError* error);

    nanodbc::connection* connection_with_user_pass_timeout(const char16_t* dsn, const char16_t* user, const char16_t* pass, long timeout, NativeError* error);

    bool is_connected(const nanodbc::connection* conn, NativeError* error);

    nanodbc::statement* create_statement(nanodbc::connection* conn, NativeError* error);

    void disconnect(nanodbc::connection* conn, NativeError* error);

}