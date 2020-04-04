#include "duckdb/main/relation/projection_relation.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/parser/query_node/select_node.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"

namespace duckdb {

ProjectionRelation::ProjectionRelation(shared_ptr<Relation> child_p, vector<unique_ptr<ParsedExpression>> parsed_expressions, vector<string> aliases) :
	Relation(child_p->context, RelationType::PROJECTION), expressions(move(parsed_expressions)), child(move(child_p)) {
	if (aliases.size() > 0) {
		if (aliases.size() != expressions.size()) {
			throw ParserException("Aliases list length must match expression list length!");
		}
		for(idx_t i = 0; i < aliases.size(); i++) {
			expressions[i]->alias = aliases[i];
		}
	}
	// bind the expressions
	context.TryBindRelation(*this, this->columns);
}

unique_ptr<QueryNode> ProjectionRelation::GetQueryNode() {
	auto child_node = child->GetQueryNode();

	auto result = make_unique<SelectNode>();
	for(auto &expr : expressions) {
		result->select_list.push_back(expr->Copy());
	}
	result->from_table = make_unique<SubqueryRef>(move(child_node), child->GetAlias());
	return move(result);
}

const vector<ColumnDefinition> &ProjectionRelation::Columns() {
	return columns;
}

string ProjectionRelation::ToString(idx_t depth) {
	string str = RenderWhitespace(depth) + "Projection [";
	for(idx_t i = 0; i < expressions.size(); i++) {
		if (i != 0) {
			str += ", ";
		}
		str += expressions[i]->ToString();
	}
	str += "]\n";
	return str + child->ToString(depth + 1);;
}

}