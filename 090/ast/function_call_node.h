#ifndef __MML_AST_FUNCTION_CALL_NODE_H__
#define __MML_AST_FUNCTION_CALL_NODE_H__

#include <string>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>


namespace mml {

  /**
   * Class for describing function definition nodes.
   */
  class function_call_node: public cdk::expression_node {
		cdk::expression_node *_function;
		cdk::sequence_node *_arguments;
    int _recursive;

  public:

    /**
     * Constructor for a function call with arguments.
     */
    function_call_node(int lineno, cdk::expression_node *function, cdk::sequence_node *arguments , int flag) :
       cdk::expression_node(lineno), _function(function), _arguments(arguments), _recursive(flag) {}

  public:
   	inline cdk::expression_node *function() {
      return _function;
    }

    inline int& recursive() {
      return _recursive;
    }

 		inline cdk::sequence_node* arguments(){
			return _arguments;
		}

    cdk::expression_node *argument(size_t ix) {
      return dynamic_cast<cdk::expression_node*>(_arguments->node(ix));
    }
	
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_call_node(this, level);
    }

  };

} // mml

#endif
