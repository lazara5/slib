/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Expression.h"
#include "slib/util/expr/ExpressionEvaluator.h"

namespace slib {
namespace expr {

Expression::~Expression() {}

UPtr<Value> Expression::evaluate(SPtr<Resolver> const& resolver) {
	_text->reset();
	return ExpressionEvaluator::expressionValue(*_text, resolver);
}

UPtr<Value> Expression::readLiteral() {
	using ReservedWord = ExpressionInputStream::ReservedWord;

	_text->reset();
	_text->skipBlanks();
	ValueDomain domain = _text->readDomain();
	char ch = _text->peek();

	if (ExpressionInputStream::isIdentifierStart(ch)) {
		ReservedWord reservedWord;
		UPtr<String> symbolName = _text->readName(reservedWord);
		if (reservedWord != ReservedWord::NONE)
			throw SyntaxErrorException(_HERE_, fmt::format("Symbol name expected, reserved word '{}' found instead", *symbolName).c_str());
		_text->skipBlanks();
		if (_text->peek() == CharacterIterator::DONE)
			return Value::of(std::move(symbolName), domain);
	}

	THROW(SyntaxErrorException, "Literal expected");
}

} // namespace expr
} // namespace slib
