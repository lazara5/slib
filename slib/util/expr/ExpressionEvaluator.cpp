/* This Source Cod Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

SPtr<Object> ExpressionEvaluator::InternalResolver::getVar(const String &key) const {
	SPtr<Object> value = _externalResolver->getVar(key);

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

UPtr<String> ExpressionEvaluator::strExpressionValue(SPtr<BasicString> const& input, SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
	return strExpressionValue(newS<ExpressionInputStream>(input), newS<InternalResolver>(resolver), evalFlags);
}

SPtr<Object> ExpressionEvaluator::expressionValue(const SPtr<BasicString> &input, SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
	UPtr<Value> val = expressionValue(newS<ExpressionInputStream>(input), newS<InternalResolver>(resolver), evalFlags);
	return val->_value;
}

// surrogate values for multi-char operators
static const int OP_LTE = 500;
static const int OP_GTE = 501;
static const int OP_EQ = 502;
static const int OP_NEQ = 503;

UPtr<String> ExpressionEvaluator::strExpressionValue(const SPtr<ExpressionInputStream> &input, SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
	SPtr<Value> val = expressionValue(input, resolver, evalFlags);
	if (val->isNil()) {
		if (evalFlags & EXPR_IGNORE_UNDEFINED)
			return nullptr;
		else
			Value::checkNil(*val);
	}
	if (instanceof<Number>(val->_value)) {
		double d = Class::cast<Number>(val->_value)->doubleValue();
		if (Number::isMathematicalInteger(d))
			return Long::toString((long)d);
		else
			return Double::toString(d);
	} else
		return val->_value->toString();
}

UPtr<Value> ExpressionEvaluator::expressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
	input->skipBlanks();

	UPtr<Value> val = prefixTermValue(input, resolver, evalFlags);
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
		UPtr<Value> nextVal = prefixTermValue(input, resolver, evalFlags);
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
			case OP_EQ:
				val = val->eq(nextVal);
				break;
			case OP_NEQ:
				val = val->neq(nextVal);
				break;
			default:
				throw SyntaxErrorException(_HERE_, fmt::format("Unknown operator '{}'", (char)op).c_str());
		}

		input->skipBlanks();
	}

	return Value::normalize(std::move(val));
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
						UPtr<String> exprValue = strExpressionValue(newS<ExpressionInputStream>(expr), resolver,
																	ignoreMissing ? EXPR_IGNORE_UNDEFINED : 0);
						if ((!exprValue) && ignoreMissing)
							return nullptr;
						result.add(*exprValue);
					} catch (MissingSymbolException const& e) {
						if (ignoreMissing) {
							result.add("${").add(*expr).add('}');
						} else
							throw;
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
	void appendExpr(SPtr<String> expr, SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
		if (_result)
			convertToString();

		if (_strResult)
			_strResult->add(ExpressionEvaluator::strExpressionValue(newS<ExpressionInputStream>(expr), resolver, evalFlags));
		else
			_result = ExpressionEvaluator::expressionValue(newS<ExpressionInputStream>(expr), newS<ExpressionEvaluator::InternalResolver>(resolver), evalFlags);
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
							result.appendExpr(std::move(expr), resolver,
											  ignoreMissing ? EXPR_IGNORE_UNDEFINED : 0);
						} catch (MissingSymbolException const& e) {
							if (ignoreMissing)
								result.add("${").add(*expr).add('}');
							else
								throw;
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

UPtr<Value> ExpressionEvaluator::prefixTermValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
												 EvalFlags evalFlags) {
	bool negative = false;
	bool negate = false;

	if (input->peek() == '-') {
		input->readChar();
		negative = true;
	} else if (input->peek() == '!') {
		input->readChar();
		negate = true;
	}

	UPtr<Value> val = termValue(input, resolver, evalFlags);
	if (negative)
		val = val->inverse();
	else if (negate)
		val = val->logicalNegate();

	return val;
}

UPtr<Value> ExpressionEvaluator::termValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
										   EvalFlags evalFlags) {
	input->skipBlanks();
	UPtr<Value> val = factorValue(input, resolver, evalFlags);
	input->skipBlanks();
	while (input->peek() == '*' || input->peek() == '/' || input->peek() == '%') {
		char op = input->readChar();
		UPtr<Value> nextVal = factorValue(input, resolver, evalFlags);
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

static void swallowSeparators(SPtr<ExpressionInputStream> const& input, bool newlineIsSep) {
	do {
		input->skipBlanks();
		switch (input->peek()) {
			case '\n':
				if (!newlineIsSep)
					return;
				/* fall through */
			case ',':
				input->readChar();
				input->skipBlanks();
				continue;
			default:
				return;
		}
	} while (true);
}

typedef struct {
	char functionClose;
	bool newlineIsSep;
} FunctionParseOpts;

static void resetFPO(FunctionParseOpts &fpo) {
	fpo = {
		.functionClose = ')',
		.newlineIsSep = false
	};
}

/** @throws EvaluationException */
static UPtr<Value> checkedExpressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
										  EvalFlags evalFlags) {
	try {
		return ExpressionEvaluator::expressionValue(input, resolver, evalFlags);
	} catch (MissingSymbolException const& e) {
		if (evalFlags & EXPR_IGNORE_UNDEFINED) {
			return Value::Nil(e.getSymbolName());
		} else
			throw;
	} catch (NilValueException const& e) {
		if (evalFlags & EXPR_IGNORE_UNDEFINED) {
			return Value::Nil();
		} else
			throw;
	}
}

UPtr<Value> ExpressionEvaluator::factorValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
											 EvalFlags evalFlags) {
	input->skipBlanks();

	PrimaryType primaryType = PrimaryType::NONE;
	UPtr<Value> val = primaryValue(input, resolver, primaryType, evalFlags);

	FunctionParseOpts fpo = {
		.functionClose = ')',
		.newlineIsSep = false
	};

	// override next char to skip to function
	char peekOverride = '\0';

	switch(primaryType) {
		case OBJ_CONSTRUCTOR:
			fpo.functionClose = '}';
			fpo.newlineIsSep = true;
			peekOverride = '(';
			break;
		case ARRAY_CONSTRUCTOR:
			fpo.functionClose = ']';
			fpo.newlineIsSep = true;
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
					UPtr<Value> arg = expressionValue(input, resolver, evalFlags);
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
						symbolName = "<unknown>"_SPTR;
					UPtr<FunctionParseContext> parseCtx = FunctionParseContext::forFunction(func, symbolName, resolver);

					do {
						swallowSeparators(input, fpo.newlineIsSep);

						if (input->peek() == fpo.functionClose) {
							input->readChar();
							resetFPO(fpo);
							break;
						}

						Class const& argClass = parseCtx->peekArg();
						if (argClass == classOf<Expression>::_class())
							parseCtx->addArg(input->readArg());
						else {
							parseCtx->addArg(checkedExpressionValue(input, resolver, evalFlags)->_value);
						}
					} while (true);

					/*// check for 0 parameters
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
					}*/
					try {
						val = parseCtx->evaluate(resolver, evalFlags);
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
					if (val->isNil()) {
						input->skipBlanks();
						if (input->peek() != '.') {
							// reached end of dotted and still nothing
							Value::checkNil(*val);
						}
					}
				}
				break;
			case '=':
				{
					if (primaryType != PrimaryType::LITERAL)
						throw SyntaxErrorException(_HERE_, "Object key must be literal");
					input->readChar();
					SPtr<String> tupleName = Class::cast<String>(val->getValue());
					UPtr<Value> tupleVal = checkedExpressionValue(input, resolver, evalFlags);
					val = Value::of(newS<KeyValue<String>>(tupleName, val->isGlobal(), tupleVal->getValue()));
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

UPtr<Value> ExpressionEvaluator::primaryValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver, PrimaryType &type,
											  EvalFlags evalFlags) {
	type = PrimaryType::NONE;
	bool global = false;

	input->skipBlanks();
	char ch = input->peek();
	if (ch == ':') {
		global = true;
		input->readChar();
		ch = input->peek();
	}
	if (std::isdigit(ch)) {
		type = PrimaryType::VALUE;
		return input->readNumber();
	} else if (ExpressionInputStream::isIdentifierStart(ch)) {
		type = PrimaryType::VALUE;
		return evaluateSymbol(input, resolver, type, global);
	} else if (ch == '(') {
		type = PrimaryType::VALUE;
		input->readChar();
		UPtr<Value> val = expressionValue(input, resolver, evalFlags);
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
		return Value::of(resolver->getVar("::makeObj"));
	} else if (ch == '[') {
		type = PrimaryType::ARRAY_CONSTRUCTOR;
		return Value::of(resolver->getVar("::makeArray"));
	} else if (ch == CharacterIterator::DONE)
		throw SyntaxErrorException(_HERE_, "Unexpected end of stream");
	else if (ch == ')')
		throw SyntaxErrorException(_HERE_, "Extra right paranthesis");
	else if (ch == '+' || ch == '-' || ch == '&' || ch == '|' || ch == '*' || ch == '/' || ch == '%')
		throw SyntaxErrorException(_HERE_, fmt::format("Misplaced operator '{}'", ch).c_str());
	else
		throw SyntaxErrorException(_HERE_, fmt::format("Unexpected character '{}' encountered", ch).c_str());
}

UPtr<Value> ExpressionEvaluator::evaluateSymbol(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
												PrimaryType &type, bool global) {
	UPtr<String> symbolName = input->readName();
	input->skipBlanks();
	char ch = input->peek();
	if (ch == '=') {
		// tuple key literal
		type = PrimaryType::LITERAL;
		return Value::scopedOf(std::move(symbolName), global);
	} else {
		type = PrimaryType::VALUE;
		SPtr<Object> value = resolver->getVar(*symbolName);
		if (value)
			return newU<Value>(value, std::move(symbolName));
		return Value::Nil(std::move(symbolName));
	}
}

} // namespace expr
} // namespace slib
