/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONFORMATTER_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONFORMATTER_H

#include "slib/StringBuilder.h"
#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

/**
 * Originally part of the Apache Harmony project. Adapted and modified to integrate with the
 * ExpressionEvaluator.
 */
class ExpressionFormatter {
private:
	SPtr<StringBuilder> _out;
public:
	ExpressionFormatter(SPtr<StringBuilder> const& out)
	:_out(out) {}

	/**
	 * @throws EvaluationException
	 */
	UPtr<ExpressionFormatter> format(ArgList const& args, SPtr<Resolver> const& resolver);
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONFORMATTER_H
