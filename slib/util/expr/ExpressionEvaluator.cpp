/* This Source Cod Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

Object Value::_void;

SPtr<Object> ExpressionEvaluator::InternalResolver::getVar(const String &key, ValueDomain domain) const {
	SPtr<Object> value = _externalResolver->getVar(key, domain);

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

/** States used by the string interpolator */
enum class InterState { APPEND, DOLLAR, READEXPR, STRING };

/** @throws EvaluationException */
UPtr<String> ExpressionEvaluator::interpolate(String const& pattern, SPtr<Resolver> const& resolver, bool ignoreMissing) {
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
						ExpressionInputStream iStream(expr);
						UPtr<String> exprValue = strExpressionValue(iStream, resolver);
						if ((!exprValue) && ignoreMissing)
							return nullptr;
						result.add(*exprValue);
					} catch (NilValueException const& e) {
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
		_strResult = newU<StringBuilder>();
		if (_result) {
			_strResult->add(*_result->asString());
			_result = nullptr;
		}
	}
public:
	/** @throws EvaluationException */
	bool appendExpr(SPtr<String> expr, SPtr<Resolver> const& resolver) {
		if (_result)
			convertToString();

		if (_strResult) {
			ExpressionInputStream iStream(expr);
			UPtr<String> exprVal = ExpressionEvaluator::strExpressionValue(iStream, resolver);
			if (!exprVal)
				return false;
			_strResult->addStr(exprVal);
		} else {
			ExpressionInputStream iStream(expr);
			SPtr<Value> exprVal = ExpressionEvaluator::expressionValue(iStream, newS<ExpressionEvaluator::InternalResolver>(resolver));
			if (!exprVal)
				return false;
			_result = std::move(exprVal);
		}

		return true;
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
				return ""_SPTR;
			return _result->_value;
		} else
			return _strResult->toString();
	}
};

/** @throws EvaluationException */
SPtr<Object> ExpressionEvaluator::smartInterpolate(String const& pattern, SPtr<Resolver> const& resolver, bool ignoreMissing) {
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
					} catch (NilValueException const& e) {
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


UPtr<String> ExpressionEvaluator::strExpressionValue(SPtr<IString> const& input, SPtr<Resolver> const& resolver) {
	ExpressionInputStream iStream(input);
	return strExpressionValue(iStream, newS<InternalResolver>(resolver));
}

SPtr<Object> ExpressionEvaluator::expressionValue(const SPtr<IString> &input, SPtr<Resolver> const& resolver) {
	ExpressionInputStream iStream(input);
	UPtr<Value> val = expressionValue(iStream, newS<InternalResolver>(resolver));
	return val->_value;
}

// surrogate values for multi-char operators
static const int OP_LTE = 500;
static const int OP_GTE = 501;
static const int OP_EQ = 502;
static const int OP_NEQ = 503;

UPtr<String> ExpressionEvaluator::strExpressionValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	SPtr<Value> val = expressionValue(input, resolver);
	if (val->isNil())
			return nullptr;
	if (val->isVoid())
			return ""_UPTR;

	if (instanceof<Number>(val->_value)) {
		double d = Class::cast<Number>(val->_value)->doubleValue();
		if (Number::isMathematicalInteger(d))
			return Long::toString((long)d);
		else
			return Double::toString(d);
	} else
		return val->_value->toString();
}

static UPtr<Value> assignmentValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> logicalTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> eqTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> relTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> addTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> termValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> unaryValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> factorValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> primaryValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver);
static UPtr<Value> evaluateSymbol(ExpressionInputStream& input, SPtr<Resolver> const& resolver, ValueDomain domain);

UPtr<Value> ExpressionEvaluator::expressionValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	UPtr<Value> val = assignmentValue(input, resolver);
	input.skipBlanks();
	while (input.peek() == ',') {
		input.readChar();

		input.skipBlanks();
		val = assignmentValue(input, resolver);
	}

	return Value::normalize(std::move(val));
}

UPtr<Value> ExpressionEvaluator::singleExpressionValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	UPtr<Value> val = assignmentValue(input, resolver);

	return Value::normalize(std::move(val));
}

static UPtr<Value> assignmentValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();

	UPtr<Value> val = logicalTermValue(input, resolver);
	input.skipBlanks();

	char c = input.peek();
	if (c != '=')
		return val;
	input.readChar();

	input.skipBlanks();
	UPtr<Value> assignVal = Value::normalize(assignmentValue(input, resolver));

	val->assign(assignVal->getValue());

	return assignVal;
}

static bool isLogicalOp(char c) {
	return (c == '&') || (c == '|') || (c == '?');
}

static UPtr<Value> logicalTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();

	UPtr<Value> val = eqTermValue(input, resolver);
	input.skipBlanks();
	while (isLogicalOp(input.peek())) {
		int op = input.readChar();

		input.skipBlanks();
		UPtr<Value> nextVal = eqTermValue(input, resolver);
		switch (op) {
			case '&':
				val = Value::logicalAnd(std::move(val), std::move(nextVal));
				break;
			case '|':
				val = Value::logicalOr(std::move(val), std::move(nextVal));
				break;
			case '?':
				val = Value::selectOr(std::move(val), std::move(nextVal));
				break;
			default:
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '{}'", (char)op).c_str());
		}

		input.skipBlanks();
	}

	return val;
}

static UPtr<Value> eqTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();

	UPtr<Value> val = relTermValue(input, resolver);
	input.skipBlanks();
	while (input.peek() == '~' || input.peek() == '=' ) {
		ssize_t beforeEquals = input.getIndex();
		int op = input.readChar();
		if (op == '=') {
			if (input.peek() == '=') {
				input.readChar();
				op = OP_EQ;
			} else {
				// probably an assignment, pass up callstack
				input.setIndex(beforeEquals);
				break;
			}
		} else if (op == '~') {
			if (input.peek() == '=') {
				input.readChar();
				op = OP_NEQ;
			} else
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '~{}'", input.peek()).c_str());
		}

		input.skipBlanks();
		UPtr<Value> nextVal = relTermValue(input, resolver);
		switch (op) {
			case OP_EQ:
				val = val->eq(nextVal);
				break;
			case OP_NEQ:
				val = val->neq(nextVal);
				break;
			default:
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '{}'", (char)op).c_str());
		}

		input.skipBlanks();
	}

	return val;
}

static UPtr<Value> relTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();

	UPtr<Value> val = addTermValue(input, resolver);
	input.skipBlanks();
	while (input.peek() == '<' || input.peek() == '>') {
		int op = input.readChar();
		if (op == '<') {
			if (input.peek() == '=') {
				input.readChar();
				op = OP_LTE;
			}
		} else if (op == '>') {
			if (input.peek() == '=') {
				input.readChar();
				op = OP_GTE;
			}
		}

		input.skipBlanks();
		UPtr<Value> nextVal = addTermValue(input, resolver);
		switch (op) {
			case '<':
				val = val->lt(nextVal);
				break;
			case OP_LTE:
				val = val->lte(nextVal);
				break;
			case '>':
				val = val->gt(nextVal);
				break;
			case OP_GTE:
				val = val->gte(nextVal);
				break;
			default:
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '{}'", (char)op).c_str());
		}

		input.skipBlanks();
	}

	return val;
}

static UPtr<Value> addTermValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();

	UPtr<Value> val = termValue(input, resolver);
	while (input.peek() == '+' || input.peek() == '-' ) {
		int op = input.readChar();

		input.skipBlanks();
		UPtr<Value> nextVal = termValue(input, resolver);

		switch (op) {
			case '+':
				val = Value::add(std::move(val), std::move(nextVal));
				break;
			case '-':
				val = val->subtract(nextVal);
				break;
			default:
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '{}'", (char)op).c_str());
		}

		input.skipBlanks();
	}

	return val;
}

static UPtr<Value> termValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();
	UPtr<Value> val = unaryValue(input, resolver);
	input.skipBlanks();
	while (input.peek() == '*' || input.peek() == '/' || input.peek() == '%') {
		char op = input.readChar();
		UPtr<Value> nextVal = unaryValue(input, resolver);
		switch (op) {
			case '*':
				val = val->multiply(nextVal);
				break;
			case '/':
				val = val->divide(nextVal);
				break;
			default:
				val = val->remainder(nextVal);
				break;
		}
		input.skipBlanks();
	}
	return val;
}

static void swallowSeparators(ExpressionInputStream& input, SPtr<Function> const& func) {
	char argSep = func->_argSeparator;
	do {
		input.skipBlanks();
		switch (input.peek()) {
			case ';':
				if (argSep == ';') {
					input.readChar();
					input.skipBlanks();
					continue;
				}
				return;
			case ',':
				if (argSep == ',') {
					input.readChar();
					input.skipBlanks();
					continue;
				}
				return;
			default:
				return;
		}
	} while (true);
}

static UPtr<Value> unaryValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	bool negative = false;
	bool negate = false;

	if (input.peek() == '-') {
		input.readChar();
		negative = true;
	} else if (input.peek() == '!') {
		input.readChar();
		negate = true;
	}

	UPtr<Value> val = factorValue(input, resolver);
	if (negative)
		val = val->inverse();
	else if (negate)
		val = val->logicalNegate();

	return val;
}

static UPtr<Value> factorValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	using ReservedWord = ExpressionInputStream::ReservedWord;

	input.skipBlanks();

	UPtr<Value> val = primaryValue(input, resolver);

	// override next char to skip to function
	char peekOverride = '\0';

	if (val->is<Function>()) {
		SPtr<Function> func = Class::cast<Function>(val->getValue());
		peekOverride = func->_peekOverride;
	}

	bool inFactor = true;
	while (inFactor) {
		input.skipBlanks();
		char nextChar = input.peek();
		if (peekOverride != '\0') {
			nextChar = peekOverride;
			peekOverride = '\0';
		}
		switch (nextChar) {
			case '[':
				{
					input.readChar();
					UPtr<Value> arg = ExpressionEvaluator::expressionValue(input, resolver);
					input.skipBlanks();
					if (input.peek() != ']')
						throw SyntaxErrorException(_HERE_, "Missing right bracket after array argument");
					input.readChar();
					val = val->index(arg);
				}
				break;
			case '(':
				{
					if (!val->is<Function>())
						throw EvaluationException(_HERE_, "Not a function");

					input.readChar();

					// evaluate function
					SPtr<Function> func = Class::cast<Function>(val->getValue());
					SPtr<String> symbolName = val->getName();
					if (!symbolName)
						symbolName = "<unknown>"_SPTR;

					SPtr<FunctionInstance> fInstance = func->newInstance(symbolName, resolver);

					do {
						swallowSeparators(input, func);

						if (input.peek() == func->_argClose) {
							input.readChar();
							break;
						}

						fInstance->readArg(input);
					} while (true);

					try {
						val = fInstance->evaluate();
					} catch (ClassCastException const& e) {
						throw CastException(_HERE_, fmt::format("Cast exception in function {}()", *symbolName).c_str(), e);
					}
				}
				break;
			case '.':
				{
					input.readChar();
					ReservedWord reservedWord;
					SPtr<String> name = input.readName(reservedWord);
					if (reservedWord != ReservedWord::NONE)
						throw SyntaxErrorException(_HERE_, fmt::format("Symbol name expected, reserved word '{}' found instead", *name).c_str());
					val = val->member(name, resolver);
					if (val->isNil()) {
						input.skipBlanks();
						if (input.peek() != '.') {
							// reached end of dotted and still nothing
							// continue with nil
						}
					}
				}
				break;
			default:
				inFactor = false;
				break;
		}
	}
	input.skipBlanks();
	return val;
}

static UPtr<Value> primaryValue(ExpressionInputStream& input, SPtr<Resolver> const& resolver) {
	input.skipBlanks();
	ValueDomain domain = input.readDomain();
	char ch = input.peek();

	if (std::isdigit(ch))
		return input.readNumber();
	else if (ExpressionInputStream::isIdentifierStart(ch))
		return evaluateSymbol(input, resolver, domain);
	else if (ch == '(') {
		input.readChar();
		UPtr<Value> val = ExpressionEvaluator::expressionValue(input, resolver);
		input.skipBlanks();
		if (input.peek() != ')')
			throw SyntaxErrorException(_HERE_, "Missing right paranthesis");
		input.readChar();
		return val;
	} else if (ch == '\'' || ch == '\"')
		return input.readString();
	else if (ch == '{')
		return Value::of(ExpressionEvaluator::_builtins->_objectConstructor);
	else if (ch == '[')
		return Value::of(ExpressionEvaluator::_builtins->_arrayConstructor);
	else if (ch == CharacterIterator::DONE)
		throw SyntaxErrorException(_HERE_, "Unexpected end of stream");
	else if (ch == ')')
		throw SyntaxErrorException(_HERE_, "Extra right paranthesis");
	else if (ch == '+' || ch == '-' || ch == '&' || ch == '|' || ch == '*' || ch == '/' || ch == '%')
		throw SyntaxErrorException(_HERE_, fmt::format("Misplaced operator '{}'", ch).c_str());
	else
		throw SyntaxErrorException(_HERE_, fmt::format("Unexpected character '{}' encountered", ch).c_str());
}

static UPtr<Value> evaluateSymbol(ExpressionInputStream& input, SPtr<Resolver> const& resolver,
								  ValueDomain domain) {
	using ReservedWord = ExpressionInputStream::ReservedWord;

	ReservedWord reservedWord;
	SPtr<String> symbolName = input.readName(reservedWord);
	input.skipBlanks();
	if (reservedWord != ReservedWord::NONE) {
		if (domain == ValueDomain::DEFAULT) {
			switch (reservedWord) {
				case ReservedWord::FALSE:
					return Value::of(newS<Boolean>(false));
				case ReservedWord::TRUE:
					return Value::of(newS<Boolean>(true));
				case ReservedWord::NIL:
					return Value::Nil();
				case ReservedWord::NONE:
					break;
			}
		} else {
			throw SyntaxErrorException(_HERE_, fmt::format("Symbol name expected, reserved word '{}' found instead", *symbolName).c_str());
		}
	}
	char ch = input.peek();
	SPtr<Object> value = (ch == '=') ? nullptr : resolver->getVar(*symbolName, domain);
	return Value::assignableOf(newS<ResolverAssignable>(resolver, symbolName, domain),
							   value, symbolName, domain);
}

} // namespace expr
} // namespace slib

