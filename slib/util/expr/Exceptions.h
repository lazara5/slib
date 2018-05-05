/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXCEPTIONS_H
#define H_SLIB_UTIL_EXPR_EXCEPTIONS_H

#include "slib/exception/Exception.h"
#include "slib/String.h"

#include <memory>

namespace slib {
namespace expr {

class EvaluationException : public Exception {
protected:
	EvaluationException(const char *where, const char *className, const char *msg)
	:Exception(where, className, msg) {}

	EvaluationException(const char *where, const char *className, const char *msg, Exception const& error)
	:Exception(where, className, msg, error) {}
public:
	EvaluationException(const char *where, const char *msg)
	:EvaluationException(where, "EvaluationException", msg) {}

	EvaluationException(const char *where, const char *msg, Exception const& error)
	:EvaluationException(where, "EvaluationException", msg, error) {}

	EvaluationException(const char *where, const char *oper, Class const* op1)
	:EvaluationException(where, "EvaluationException", fmt::format("Operator '{}' not applicable for '{}'", oper, op1->getName()).c_str()) {}

	EvaluationException(const char *where, const char *oper, Class const* op1, Class const* op2)
	:EvaluationException(where, "EvaluationException", fmt::format("Operator '{}' not applicable for '{}' and '{}'",
																   oper, op1->getName(), op2->getName()).c_str()) {}
};

class SyntaxErrorException : public EvaluationException {
public:
	SyntaxErrorException(const char *where, const char *msg)
	:EvaluationException(where, "SyntaxErrorException", msg) {}
};

class MissingSymbolException : public EvaluationException {
private:
	std::shared_ptr<String> _name;
public:
	MissingSymbolException(const char *where, std::shared_ptr<String> const& name)
	:EvaluationException(where, "MissingSymbolException",
						 fmt::format("Symbol '{}' could not be located", name->c_str()).c_str())
	,_name(name) {}

	std::shared_ptr<String> getSymbolName() const {
		return _name;
	}
};

class NilValueException : public EvaluationException {
public:
	NilValueException(const char *where)
	:EvaluationException(where, "NilValueException", "Nil value") {}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXCEPTIONS_H
