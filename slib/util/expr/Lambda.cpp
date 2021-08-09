/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Lambda.h"
#include "slib/util/expr/ExpressionEvaluator.h"

namespace slib {
namespace expr {

Lambda::~Lambda() {}

UPtr<Value> Lambda::evaluate(SPtr<Resolver> const& resolver) {
	return ExpressionEvaluator::expressionValue(newS<ExpressionInputStream>(_text), resolver);
}

UPtr<Value> Lambda::readLiteral() {
	SPtr<ExpressionInputStream> input = newS<ExpressionInputStream>(_text);

	input->skipBlanks();
	ValueDomain domain = input->readDomain();
	char ch = input->peek();

	if (ExpressionInputStream::isIdentifierStart(ch)) {
		UPtr<String> symbolName = input->readName();
		input->skipBlanks();
		if (input->peek() == CharacterIterator::DONE)
			return Value::of(std::move(symbolName), domain);
	}

	throw SyntaxErrorException(_HERE_, "Literal expected");
}

} // namespace expr
} // namespace slib
