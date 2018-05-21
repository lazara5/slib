/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

SPtr<Object> ExpressionEvaluator::InternalResolver::getVar(const String &key) {
	SPtr<Object> value = _externalResolver->getVar(key);

	if (!value)
		value = _builtins->get(key);

	return value;
}

ExpressionEvaluator::LoopResolver::~LoopResolver() {}

UPtr<String> ExpressionEvaluator::strExpressionValue(SPtr<BasicString> const& input,
													 SPtr<Resolver> const& resolver) {
	return strExpressionValue(std::make_shared<ExpressionInputStream>(input),
							  std::make_shared<InternalResolver>(resolver));
}

SPtr<Object> ExpressionEvaluator::expressionValue(const SPtr<BasicString> &input, const SPtr<Resolver> &resolver) {
	SPtr<Value> val = expressionValue(std::make_shared<ExpressionInputStream>(input),
									  std::make_shared<InternalResolver>(resolver));
	if (val->_value) {
		if (instanceof<Double>(val->_value)) {
			double d = Class::cast<Double>(val->_value)->doubleValue();
			if (Number::isMathematicalInteger(d)) {
				int32_t i = (int32_t)d;
				if (i == d)
					return std::make_shared<Integer>(i);
				return std::make_shared<Long>((uint64_t)d);
			}
		}
	}
	return val->_value;
}

// surrogate values for multi-char operators
static const int OP_LTE = 500;
static const int OP_GTE = 501;
static const int OP_EQ = 502;
static const int OP_NEQ = 503;

UPtr<String> ExpressionEvaluator::strExpressionValue(const SPtr<ExpressionInputStream> &input,
													 const SPtr<Resolver> &resolver) {
	SPtr<Value> val = expressionValue(input, resolver);
	Value::checkNil(*val);
	if (instanceof<Number>(val->_value)) {
		double d = Class::cast<Number>(val->_value)->doubleValue();
		if (Number::isMathematicalInteger(d))
			return Long::toString((long)d);
		else
			return Double::toString(d);
	} else
		return val->_value->toString();
}

SPtr<Value> ExpressionEvaluator::expressionValue(SPtr<ExpressionInputStream> const& input,
												 SPtr<Resolver> const& resolver) {
	input->skipBlanks();

	SPtr<Value> val = prefixTermValue(input, resolver);
	input->skipBlanks();
	while (input->peek() == '+' || input->peek() == '-' ||
		   input->peek() == '&' || input->peek() == '|' ||
		   input->peek() == '<' || input->peek() == '>' || input->peek() == '~' ||
		   input->peek() == '=' ) {
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

		input->skipBlanks();
		SPtr<Value> nextVal = prefixTermValue(input, resolver);
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

		input->skipBlanks();
	}

	return val;
}

SPtr<Value> ExpressionEvaluator::prefixTermValue(SPtr<ExpressionInputStream> const& input,
												 SPtr<Resolver> const& resolver) {
	bool negative = false;
	bool negate = false;

	if (input->peek() == '-') {
		input->readChar();
		negative = true;
	} else if (input->peek() == '!') {
		input->readChar();
		negate = true;
	}

	SPtr<Value> val = termValue(input, resolver);
	if (negative)
		val = val->inverse();
	else if (negate)
		val = val->logicalNegate();

	return val;
}

SPtr<Value> ExpressionEvaluator::termValue(SPtr<ExpressionInputStream> const& input,
										   SPtr<Resolver> const& resolver) {
	input->skipBlanks();
	SPtr<Value> val = factorValue(input, resolver);
	input->skipBlanks();
	while (input->peek() == '*' || input->peek() == '/' || input->peek() == '%') {
		char op = input->readChar();
		SPtr<Value> nextVal = factorValue(input, resolver);
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

SPtr<Value> ExpressionEvaluator::factorValue(SPtr<ExpressionInputStream> const& input,
											 SPtr<Resolver> const& resolver) {
	input->skipBlanks();
	SPtr<Value> val = primaryValue(input, resolver);

	bool inFactor = true;
	while (inFactor) {
		input->skipBlanks();
		char nextChar = input->peek();
		switch (nextChar) {
			case '[':
				{
					input->readChar();
					SPtr<Value> arg = expressionValue(input, resolver);
					input->skipBlanks();
					if (input->peek() != ']')
						throw SyntaxErrorException(_HERE_, "Missing right bracket after array argument");
					input->readChar();
					val = val->index(arg);
				}
				break;
			case '(':
				{
					if (!instanceof<Function>(val->_value))
						throw EvaluationException(_HERE_, "Not a function");

					input->readChar();

					// evaluate function
					SPtr<Function> func = Class::cast<Function>(val->_value);
					SPtr<String> symbolName = val->getName();
					if (!symbolName)
						symbolName = std::make_shared<String>("<unknown>");
					FunctionArgs params(func, symbolName);

					// check for 0 parameters
					input->skipBlanks();
					if (input->peek() == ')')
						input->readChar();
					else {
						do {
							Class const* argClass = params.peek();
							if (*argClass == *EXPRESSIONCLASS())
								params.add(input->readArg());
							else
								params.add(expressionValue(input, resolver)->_value);
							input->skipBlanks();
							if (input->peek() == ',') {
								input->readChar();
								continue;
							} else if (input->peek() == ')') {
								input->readChar();
								break;
							} else
								throw SyntaxErrorException(_HERE_, fmt::format("Missing right paranthesis after function arguments ({})", *symbolName).c_str());
						} while (true);
					}
					try {
						val = func->evaluate(resolver, params);
					} catch (ClassCastException const& e) {
						throw CastException(_HERE_, fmt::format("Cast exception in function {}()", *symbolName).c_str(), e);
					}
				}
				break;
			case '.':
				{
					input->readChar();
					SPtr<String> name = input->readName();
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

SPtr<Value> ExpressionEvaluator::primaryValue(SPtr<ExpressionInputStream> const& input,
											  SPtr<Resolver> const& resolver) {
	input->skipBlanks();
	char ch = input->peek();
	if (std::isdigit(ch))
		return input->readNumber();
	else if (ExpressionInputStream::isIdentifierStart(ch)) {
		// symbol
		return evaluateSymbol(input, resolver);
	} else if (ch == '(') {
		input->readChar();
		SPtr<Value> val = expressionValue(input, resolver);
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

SPtr<Value> ExpressionEvaluator::evaluateSymbol(SPtr<ExpressionInputStream> const& input,
												SPtr<Resolver> const& resolver) {
	UPtr<String> symbolName = input->readName();
	SPtr<Object> value = resolver->getVar(*symbolName);
	if (value)
		return std::make_shared<Value>(value, std::move(symbolName));
	return Value::Nil(std::move(symbolName));
}

} // namespace expr
} // namespace slib
