#ifndef __MML_TARGETS_POSTFIX_WRITER_H__
#define __MML_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace mml {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<mml::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    std::vector<int> _whileCondLabels;
    std::vector<int> _whileEndLabels;
    std::vector<int> _functionLabels; // for nested functions
    std::vector<std::shared_ptr<cdk::basic_type>> _functionReturnTypes;
    std::vector<std::shared_ptr<cdk::basic_type>> _functionTypes;
    std::vector<int> _offset; // stack for offsets and for the booleans (at least _inBlock)
    std::vector<std::string> _external;
    int _lbl;
    int _inBlock = 0;
    bool _inFunctionArgs = false;
    int _readType; // 0 = int, 1 = double
    cdk::typename_type _lvalueType;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<mml::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0), _inBlock(0), _inFunctionArgs(false){
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

  public:
  int countVariablesInBlock(mml::block_node* const node);
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // mml

#endif
