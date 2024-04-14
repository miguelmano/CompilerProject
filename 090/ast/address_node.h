#ifndef __MML_AST_ADDRESS_NODE_H__
#define __MML_AST_ADDRESS_NODE_H__

#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/expression_node.h>

namespace mml {

  class address_node: public cdk::expression_node {
    cdk::lvalue_node *_lvalue;

  public:
    address_node(int lineno, cdk::lvalue_node *lvalue) :
        cdk::expression_node(lineno), _lvalue(lvalue) {
    }

  public:
    cdk::lvalue_node* lvalue() {
      return _lvalue;
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_address_node(this, level);
    }

  };

} // mml

#endif
