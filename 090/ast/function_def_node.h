#ifndef __MML_AST_FUNCTION_DEf_NODE_H__
#define __MML_AST_FUNCTION_DEF_NODE_H__

#include <string>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/typed_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/functional_type.h>


namespace mml {

  /**
   * Class for describing function definition nodes.
   */
  class function_def_node: public cdk::expression_node {
		cdk::sequence_node *_arguments;
		mml::block_node *_block;

  public:
    inline function_def_node(int lineno,std::shared_ptr<cdk::basic_type> funcType , cdk::sequence_node *arguments, mml::block_node *block) :
        cdk::expression_node(lineno), _arguments(arguments), _block(block) {
          if(_arguments->size()!=0){
            std::vector<std::shared_ptr<cdk::basic_type>> input;
            for(int i = 0 ; i<(int)_arguments->size(); i++){
              input.push_back((dynamic_cast<cdk::typed_node*>(_arguments->node(i)))->type());
            }
            _type = cdk::functional_type::create(input,funcType);
          }  
          else{
            _type = cdk::functional_type::create(funcType);
          }
        }

  public:
		inline cdk::sequence_node* arguments(){
			return _arguments;
		}
    
    cdk::typed_node* argument(size_t ax) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(ax));
    }
	
		inline mml::block_node* block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_def_node(this, level);
    }

  };

} // mml

#endif
