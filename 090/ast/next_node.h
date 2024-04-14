#ifndef __MML_AST_NEXT_NODE_H__
#define __MML_AST_NEXT_NODE_H__

#include <cdk/ast/integer_node.h>

namespace mml {

  /**
   * Class for describing next nodes.
   */
  class next_node: public cdk::basic_node {
    size_t _cycle;

  public:
    inline next_node(int lineno, size_t cycle = 1) :
        cdk::basic_node(lineno), _cycle(cycle) {
    }

  public:
    inline size_t cycle() {
      return _cycle;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_next_node(this, level);
    }

  };

} // mml

#endif