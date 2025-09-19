#include "core/connection.hpp"
#include <wtypes.h>
#include <sqlext.h>

#ifdef NANODBC_ENABLE_UNICODE
#define NANODBC_FUNC(f) f##W
#define NANODBC_SQLCHAR SQLWCHAR
#else
#define NANODBC_FUNC(f) f
#define NANODBC_SQLCHAR SQLCHAR
#endif

// Helper: Throws if ODBC call failed.
inline static void check_odbc_result(SQLRETURN rc, const std::string& operation) {
    if (!SQL_SUCCEEDED(rc)) {
        throw std::runtime_error("ODBC error in " + operation);
    }
}

void Connection::set_catalog(const nanodbc::string& catalog) {
    if (!connected()) {
        throw std::runtime_error("Cannot set isolation level: connection is not active");
    }

    if (catalog.empty()) {
        throw std::runtime_error("Catalog name cannot be empty.");
    }

    SQLRETURN ret = ::SQLSetConnectAttr(
        this->native_dbc_handle(),
        SQL_ATTR_CURRENT_CATALOG,
        (NANODBC_SQLCHAR*) catalog.c_str(),
        SQL_NTS
    );

    check_odbc_result(ret, "SQLSetConnectAttr(SQL_ATTR_CURRENT_CATALOG) - Driver may not support changing catalog at runtime.");
}

void Connection::set_isolation_level(IsolationLevel level) {
    if (!connected()) {
        throw std::runtime_error("Cannot set isolation level: connection is not active");
    }

    SQLUINTEGER odbc_level = level.to_odbc();
    SQLRETURN rc = SQLSetConnectAttr(
        native_dbc_handle(),
        SQL_ATTR_TXN_ISOLATION,
        (SQLPOINTER)(std::intptr_t)odbc_level,
        0
    );

    check_odbc_result(rc, "SQLSetConnectAttr(SQL_ATTR_TXN_ISOLATION)");
}

IsolationLevel Connection::get_isolation_level() const {
    if (!connected()) {
        throw std::runtime_error("Cannot get isolation level: connection is not active");
    }

    SQLUINTEGER current_level = 0;
    SQLRETURN rc = SQLGetConnectAttr(
        native_dbc_handle(),
        SQL_ATTR_TXN_ISOLATION,
        &current_level,
        0,
        nullptr
    );

    check_odbc_result(rc, "SQLGetConnectAttr(SQL_ATTR_TXN_ISOLATION)");

    return IsolationLevel::from_odbc(current_level);
}

void Connection::set_auto_commit(bool auto_commit) {
    if (!this->connected()) {
        throw std::runtime_error("Cannot set auto-commit: connection is not active");
    }

    if (auto_commit) {
        // If enabling auto-commit, finalize current transaction (if any)
        if (transaction_) {
            transaction_->commit(); // Commit pending changes
            transaction_.reset();   // Clear transaction — auto-commit now active
        }
        // transaction_ == nullptr → auto-commit is ON
    } else {
        // If disabling auto-commit, start transaction (if not already started)
        if (!transaction_) {
            transaction_ = std::make_unique<nanodbc::transaction>(*this);
        }
        // transaction_ != nullptr → auto-commit is OFF
    }
}

void Connection::commit() {
    if (!this->connected()) {
        throw std::runtime_error("Cannot commit: connection is not active");
    } if (!transaction_) {
        throw std::runtime_error("Cannot commit: auto-commit is enabled or no active transaction");
    }

    transaction_->commit();
    transaction_.reset(); // Transaction complete → auto-commit implicitly restored
}

void Connection::rollback() {
    if (!this->connected()) {
        throw std::runtime_error("Cannot rollback: connection is not active");
    } if (!transaction_) {
        throw std::runtime_error("Cannot rollback: auto-commit is enabled or no active transaction");
    }

    transaction_->rollback();
    transaction_.reset(); // Transaction complete → auto-commit implicitly restored
}

bool Connection::get_auto_commit() const {
    return transaction_ == nullptr;
}