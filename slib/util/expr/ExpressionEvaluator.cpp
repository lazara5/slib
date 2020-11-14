/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

SPtr<Object> ExpressionEvaluator::InternalResolver::getVar(const String &key) const {
	SPtr<Object> value = _externalResolver.getVar(key);

	/*HashMap<String, Object> *b = (HashMap<String, Object>*)_builtins.get();
	b->forEach([](String const& key1, SPtr<Object> const& val) {
		fmt::print("b[{}, {}] = {}\n", key1, key1.hashCode(), val ? val->getClass().getName().c_str() : "null");
		return true;
	});*/

	if (!value)
		value = _builtins->get(key);

	//fmt::print("{}, {} -> {}\n", key, key.hashCode(), value ? value->getClass().getName().c_str() : "null");

	return value;
}

ExpressionEvaluator::LoopResolver::~LoopResolver() {}

UPtr<String> ExpressionEvaluator::strExpressionValue(SPtr<BasicString> const& input, Resolver const& resolver) {
	return strExpressionValue(std::make_shared<ExpressionInputStream>(input), InternalResolver(resolver));
}

SPtr<Object> ExpressionEvaluator::expressionValue(const SPtr<BasicString> &input, Resolver const& resolver) {
	SPtr<Value> val = expressionValue(std::make_shared<ExpressionInputStream>(input), InternalResolver(resolver));
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

UPtr<String> ExpressionEvaluator::strExpressionValue(const SPtr<ExpressionInputStream> &input, Resolver const& resolver) {
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

SPtr<Value> ExpressionEvaluator::expressionValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver) {
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

/**
 * States used by the string interpolator
 */
enum class InterState { APPEND, DOLLAR, READEXPR, STRING };

/** @throws EvaluationException */
UPtr<String> ExpressionEvaluator::interpolate(String const& pattern, Resolver const& resolver, bool ignoreMissing) {
	StringBuilder result;

	InterState state = InterState::APPEND;
	size_t pos = 0;

	size_t dollarBegin = 0;
	char delim = 0;

	while (pos < pattern.length()) {
		char c = pattern.charAt(pos);
		switch (state) {
			case InterState::APPEND:
				if (c == '$') {
					state = InterState::DOLLAR;
					dollarBegin = pos;
					break;
				}
				result.add(c);
				break;
			case InterState::DOLLAR:
				if (c == '$') {
					state = InterState::APPEND;
					result.add('$');
					break;
				} else if (c == '{') {
					state = InterState::READEXPR;
					break;
				}
				state = InterState::APPEND;
				result.add('$').add(c);
				break;
			case InterState::READEXPR:
				if (c == '}') {
					SPtr<String> expr = pattern.substring(dollarBegin + 2, pos);
					try {
						UPtr<String> exprValue = strExpressionValue(std::make_shared<ExpressionInputStream>(expr), resolver);
						result.add(*exprValue);
					} catch (MissingSymbolException const& e) {
						if (ignoreMissing) {
							result.add("${").add(*expr).add('}');
						} else
							throw;
					}

					state = InterState::APPEND;
				} else if (c == '"' || c == '\'') {
					delim = c;
					state = InterState::STRING;
				}
				break;
			case InterState::STRING:
				if (c == delim) {
					delim = 0;
					state = InterState::READEXPR;
				}
				break;
		}
		pos++;
	}

	return result.toString();
}

class ResultHolder {
private:
	SPtr<Value> _result;
	UPtr<StringBuilder> _strResult;

	/** @throws EvaluationException */
	void convertToString() {
		_strResult = std::make_unique<StringBuilder>();
		if (_result) {
			_strResult->add(*_result->asString());
			_result = nullptr;
		}
	}
public:
	/** @throws EvaluationException */
	void appendExpr(SPtr<String> expr, Resolver const& resolver) {
		if (_result)
			convertToString();

		if (_strResult)
			_strResult->add(ExpressionEvaluator::strExpressionValue(std::make_shared<ExpressionInputStream>(expr), resolver));
		else
			_result = ExpressionEvaluator::expressionValue(std::make_shared<ExpressionInputStream>(expr), ExpressionEvaluator::InternalResolver(resolver));
	}

	/** @throws EvaluationException */
	ResultHolder& add(char c) {
		if (!_strResult)
			convertToString();
		_strResult->add(c);
		return *this;
	}

	/** @throws EvaluationException */
	ResultHolder& add(const char *str) {
		if (!_strResult)
			convertToString();
		_strResult->add(str);
		return *this;
	}

	/** @throws EvaluationException */
	ResultHolder& add(String const& str) {
		if (!_strResult)
			convertToString();
		_strResult->add(str);
		return *this;
	}

	SPtr<Object> toObject() {
		if (!_strResult) {
			if (!_result)
				return std::make_shared<String>("");
			return _result->_value;
		} else
			return _strResult->toString();
	}
};

/** @throws EvaluationException */
SPtr<Object> ExpressionEvaluator::smartInterpolate(String const& pattern, Resolver const& resolver, bool ignoreMissing) {
		ResultHolder result;

		InterState state = InterState::APPEND;
		size_t pos = 0;

		size_t dollarBegin = 0;
		char delim = 0;

		while (pos < pattern.length()) {
			char c = pattern.charAt(pos);
			switch (state) {
				case InterState::APPEND:
					if (c == '$') {
						state = InterState::DOLLAR;
						dollarBegin = pos;
						break;
					}
					result.add(c);
					break;
				case InterState::DOLLAR:
					if (c == '$') {
						state = InterState::APPEND;
						result.add('$');
						break;
					} else if (c == '{') {
						state = InterState::READEXPR;
						break;
					}
					state = InterState::APPEND;
					result.add('$').add(c);
					break;
				case InterState::READEXPR:
					if (c == '}') {
						UPtr<String> expr = pattern.substring(dollarBegin + 2, pos);
						try {
							result.appendExpr(std::move(expr), resolver);
						} catch (MissingSymbolException const& e) {
							if (ignoreMissing)
								result.add("${").add(*expr).add('}');
							else
								throw;
						}
						state = InterState::APPEND;
					} else if (c == '"' || c == '\'') {
						delim = c;
						state = InterState::STRING;
					}
					break;
				case InterState::STRING:
					if (c == delim) {
						delim = 0;
						state = InterState::READEXPR;
					}
					break;
			}
			pos++;
		}

		return result.toObject();
	}

SPtr<Value> ExpressionEvaluator::prefixTermValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver) {
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

SPtr<Value> ExpressionEvaluator::termValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver) {
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

SPtr<Value> ExpressionEvaluator::factorValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver) {
	input->skipBlanks();

	PrimaryType primaryType = PrimaryType::NONE;
	SPtr<Value> val = primaryValue(input, resolver, primaryType);

	char functionClose = ')';

	// override next char to skip to function
	char peekOverride = '\0';

	switch(primaryType) {
		case OBJ_CONSTRUCTOR:
			functionClose = '}';
			peekOverride = '(';
			break;
		case ARRAY_CONSTRUCTOR:
			functionClose = ']';
			peekOverride = '(';
			break;
		default:
			break;
	}

	bool inFactor = true;
	while (inFactor) {
		input->skipBlanks();
		char nextChar = input->peek();
		if (peekOverride != '\0') {
			nextChar = peekOverride;
			peekOverride = '\0';
		}
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
					if (input->peek() == functionClose) {
						functionClose = ')';
						input->readChar();
					} else {
						do {
							Class const& argClass = params.peek();
							if (argClass == classOf<Expression>::_class())
								params.add(input->readArg());
							else
								params.add(expressionValue(input, resolver)->_value);
							input->skipBlanks();
							if (input->peek() == ',') {
								input->readChar();
								continue;
							} else if (input->peek() == functionClose) {
								functionClose = ')';
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
			case '=':
				{
					if (primaryType != PrimaryType::LITERAL)
						throw SyntaxErrorException(_HERE_, "Object key must be literal");
					input->readChar();
					SPtr<Value> tupleVal = expressionValue(input, resolver);
					val = Value::of(newS<KeyValueTuple<String>>(Class::cast<String>(val->getValue()), tupleVal->getValue()));
					inFactor = false;
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

SPtr<Value> ExpressionEvaluator::primaryValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver, PrimaryType &type) {
	type = PrimaryType::NONE;

	input->skipBlanks();
	char ch = input->peek();
	if (std::isdigit(ch)) {
		type = PrimaryType::VALUE;
		return input->readNumber();
	} else if (ExpressionInputStream::isIdentifierStart(ch)) {
		type = PrimaryType::VALUE;
		return evaluateSymbol(input, resolver, type);
	} else if (ch == '(') {
		type = PrimaryType::VALUE;
		input->readChar();
		SPtr<Value> val = expressionValue(input, resolver);
		input->skipBlanks();
		if (input->peek() != ')')
			throw SyntaxErrorException(_HERE_, "Missing right paranthesis");
		input->readChar();
		return val;
	} else if (ch == '\'' || ch == '\"') {
		type = PrimaryType::VALUE;
		return input->readString();
	} else if (ch == '{') {
		type = PrimaryType::OBJ_CONSTRUCTOR;
		return Value::of(resolver.getVar("::makeObj"), "::makeObj"_SPTR);
	} else if (ch == '[') {
		type = PrimaryType::ARRAY_CONSTRUCTOR;
		return Value::of(resolver.getVar("::makeArray"), "::makeArray"_SPTR);
	} else if (ch == CharacterIterator::DONE)
		throw SyntaxErrorException(_HERE_, "Unexpected end of stream");
	else if (ch == ')')
		throw SyntaxErrorException(_HERE_, "Extra right paranthesis");
	else if (ch == '+' || ch == '-' || ch == '&' || ch == '|' || ch == '*' || ch == '/' || ch == '%')
		throw SyntaxErrorException(_HERE_, fmt::format("Misplaced operator '{}'", ch).c_str());
	else
		throw SyntaxErrorException(_HERE_, fmt::format("Unexpected character '{}' encountered", ch).c_str());
}

SPtr<Value> ExpressionEvaluator::evaluateSymbol(SPtr<ExpressionInputStream> const& input, Resolver const& resolver, PrimaryType &type) {
	UPtr<String> symbolName = input->readName();
	input->skipBlanks();
	char ch = input->peek();
	if (ch == '=') {
		// tuple key literal
		type = PrimaryType::LITERAL;
		return Value::of(std::move(symbolName));
	} else {
		type = PrimaryType::VALUE;
		SPtr<Object> value = resolver.getVar(*symbolName);
		if (value)
			return std::make_shared<Value>(value, std::move(symbolName));
		return Value::Nil(std::move(symbolName));
	}
}

} // namespace expr
} // namespace slib
