/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"

namespace slib {
namespace expr {

// surrogate values for multi-char operators
static const int OP_LTE = 500;
static const int OP_GTE = 501;
static const int OP_EQ = 502;
static const int OP_NEQ = 503;

std::unique_ptr<String> ExpressionEvaluator::strExpressionValue(const std::shared_ptr<ExpressionInputStream> &input,
																const std::shared_ptr<Resolver> &resolver) {
	std::shared_ptr<Value> val = expressionValue(input, resolver);
	Value::checkNil(*val);
	if (val->_value->instanceof<Number>()) {
		double d = Class::cast<Number>(val->_value)->doubleValue();
		if (Number::isMathematicalInteger(d))
			return Long::toString((long)d);
		else
			return Double::toString(d);
	} else
		return val->_value->toString();
}

std::shared_ptr<Value> ExpressionEvaluator::expressionValue(std::shared_ptr<ExpressionInputStream> const& input,
															std::shared_ptr<Resolver> const& resolver) {
	input->skipBlanks();

	std::shared_ptr<Value> val = prefixTermValue(input, resolver);
	input->skipBlanks();
	while (input->peek() == '+' || input->peek() == '-' ||
		   input->peek() == '&' || input->peek() == '|' ||
		   input->peek() == '<' || input->peek() == '>' || input->peek() == '~' ||
		   input->peek() == '=' || input->peek() == '(') {
		int op = input->readChar();
		if (op == '<') {
			if (input->peek() == '=') {
				input->readChar();
				op = OP_LTE;
			}
		} else if (op == '>') {
			if (input->peek() == '=') {
				input->readChar();
				op = OP_GTE;
			}
		} else if (op == '=') {
			if (input->peek() == '=') {
				input->readChar();
				op = OP_EQ;
			}
		} else if (op == '~') {
			if (input->peek() == '=') {
				input->readChar();
				op = OP_NEQ;
			} else
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '~{}'", input->peek()).c_str());
		}

		if (op == '(') {
			// TODO: function
		} else {
			input->skipBlanks();
			std::shared_ptr<Value> nextVal = prefixTermValue(input, resolver);
			switch (op) {
				case '+':
					val = val->add(nextVal);
					break;
				case '-':
					val = val->subtract(nextVal);
					break;
				case '&':
					val = val->logicalAnd(nextVal);
					break;
				case '|':
					val = val->logicalOr(nextVal);
					break;
				case '<':
					val = std::make_shared<Value>(std::make_shared<Integer>(val->lt(nextVal) ? 1 : 0));
					break;
				case OP_LTE:
					val = std::make_shared<Value>(std::make_shared<Integer>(val->lte(nextVal) ? 1 : 0));
					break;
				case '>':
					val = std::make_shared<Value>(std::make_shared<Integer>(val->gt(nextVal) ? 1 : 0));
					break;
				case OP_GTE:
					val = std::make_shared<Value>(std::make_shared<Integer>(val->gte(nextVal) ? 1 : 0));
					break;
				case OP_EQ:
					val = std::make_shared<Value>(std::make_shared<Integer>(val->eq(nextVal) ? 1 : 0));
					break;
				case OP_NEQ:
					val = std::make_shared<Value>(std::make_shared<Integer>(val->eq(nextVal) ? 0 : 1));
					break;
				default:
					throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '{}'", (char)op).c_str());
			}
		}

		input->skipBlanks();
	}

	return val;
}

std::shared_ptr<Value> ExpressionEvaluator::prefixTermValue(std::shared_ptr<ExpressionInputStream> const& input,
															std::shared_ptr<Resolver> const& resolver) {
	bool negative = false;
	bool negate = false;

	if (input->peek() == '-') {
		input->readChar();
		negative = true;
	} else if (input->peek() == '!') {
		input->readChar();
		negate = true;
	}

	std::shared_ptr<Value> val = termValue(input, resolver);
	if (negative)
		val = val->inverse();
	else if (negate)
		val = val->logicalNegate();

	return val;
}

std::shared_ptr<Value> ExpressionEvaluator::termValue(std::shared_ptr<ExpressionInputStream> const& input,
													  std::shared_ptr<Resolver> const& resolver) {
	input->skipBlanks();
	std::shared_ptr<Value> val = factorValue(input, resolver);
	input->skipBlanks();
	while (input->peek() == '*' || input->peek() == '/' || input->peek() == '%') {
		char op = input->readChar();
		std::shared_ptr<Value> nextVal = factorValue(input, resolver);
		if (op == '*')
			val = val->multiply(nextVal);
		else if (op == '/')
			val = val->divide(nextVal);
		else
			val = val->remainder(nextVal);
		input->skipBlanks();
	}
	return val;
}

std::shared_ptr<Value> ExpressionEvaluator::factorValue(std::shared_ptr<ExpressionInputStream> const& input,
														std::shared_ptr<Resolver> const& resolver) {
	input->skipBlanks();
	std::shared_ptr<Value> val = primaryValue(input, resolver);

	bool inFactor = true;
	while (inFactor) {
		input->skipBlanks();
		char nextChar = input->peek();
		switch (nextChar) {
			case '[':
				{
					input->readChar();
					std::shared_ptr<Value> arg = expressionValue(input, resolver);
					input->skipBlanks();
					if (input->peek() != ']')
						throw SyntaxErrorException(_HERE_, "Missing right bracket after array argument");
					input->readChar();
					val = val->index(arg);
				}
				break;
			case '.':
				{
					input->readChar();
					std::shared_ptr<String> name = input->readName();
					val = val->member(name, resolver);
				}
				break;
			default:
				inFactor = false;
				break;
		}
	}
	input ->skipBlanks();
	return val;
}

std::shared_ptr<Value> ExpressionEvaluator::primaryValue(std::shared_ptr<ExpressionInputStream> const& input,
														 std::shared_ptr<Resolver> const& resolver) {
	input->skipBlanks();
	char ch = input->peek();
	if (std::isdigit(ch))
		return input->readNumber();
	else if (ExpressionInputStream::isIdentifierStart(ch)) {
		// symbol
		return evaluateSymbol(input, resolver);
	} else if (ch == '(') {
		input->readChar();
		std::shared_ptr<Value> val = expressionValue(input, resolver);
		input->skipBlanks();
		if (input->peek() != ')')
			throw SyntaxErrorException(_HERE_, "Missing right paranthesis");
		input->readChar();
		return val;
	} else if (ch == '\'' || ch == '\"') {
		return input->readString();
	} else if (ch == CharacterIterator::DONE)
		throw SyntaxErrorException(_HERE_, "Unexpected end of line");
	else if (ch == ')')
		throw SyntaxErrorException(_HERE_, "Extra right paranthesis");
	else if (ch == '+' || ch == '-' || ch == '&' || ch == '|' || ch == '*' || ch == '/' || ch == '%')
		throw SyntaxErrorException(_HERE_, fmt::format("Misplaced operator '{}'", ch).c_str());
	else
		throw SyntaxErrorException(_HERE_, fmt::format("Unexpected character '{}' encountered", ch).c_str());
}

std::shared_ptr<Value> ExpressionEvaluator::evaluateSymbol(std::shared_ptr<ExpressionInputStream> const& input,
														   std::shared_ptr<Resolver> const& resolver) {
	std::unique_ptr<String> symbolName = input->readName();
	std::shared_ptr<Object> value = resolver->getVar(*symbolName);
	if (value)
		return std::make_shared<Value>(value, std::move(symbolName));
	return Value::Nil(std::move(symbolName));
}


} // namespace expr
} // namespace slib
