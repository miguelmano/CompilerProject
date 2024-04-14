#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

#include "mml_parser.tab.h"

//---------------------------------------------------------------------------

void mml::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
//---------------------------------------------------------------------------

void mml::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
//---------------------------------------------------------------------------

void mml::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
 if (_inBlock) {
    _pf.DOUBLE(node->value()); // load number to the stack
  } else {
    _pf.SDOUBLE(node->value());    // double is on the DATA segment
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------
void mml::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (_inBlock) {
    _pf.INT(node->value()); // integer literal is on the stack: push an integer
  } else {
    _pf.SINT(node->value()); // integer literal is on the DATA segment
  }
}

void mml::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  /* leave the address on the stack */
   if (_functionLabels.size()) {
    // local variable initializer
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  } else {
    // global variable initializer
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.NEG(); // 2-complement
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);

  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);

   if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE)
    _pf.DADD();
  else
    _pf.ADD();
}
void mml::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE)
    _pf.DSUB();
  else
    _pf.SUB();
}
void mml::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  
  if (node->type()->name() == cdk::TYPE_DOUBLE)
    _pf.DMUL();
  else
    _pf.MUL();
}
void mml::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();

  _pf.DIV();
}
void mml::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void mml::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.LT();
}
void mml::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.LE();
}
void mml::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();
  
  node->right()->accept(this, lvl);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();
  
  _pf.GE();
}
void mml::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.GT();
}
void mml::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  _pf.NE();
}
void mml::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->left()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  _pf.EQ();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  const std::string &id = node->name();

  auto symbol = _symtab.find(id);
  if (symbol->global()) {
    if (symbol->type()->name() == cdk::TYPE_FUNCTIONAL) {
      if (symbol->qualifier() == tFOREIGN) {
        _pf.ADDR(symbol->name());
      }
      _pf.ADDRV(symbol->name());
    }
    else {
      _pf.ADDR(symbol->name());
    }
  } else {
    if (symbol->type()->name() == cdk::TYPE_FUNCTIONAL) {
      _pf.LOCV(symbol->offset());
    }
    else {
      _pf.LOCAL(symbol->offset());
    }
    std::cout << "LOCAL " << symbol->name() << ":" << symbol->type()->size() << ":" << symbol->offset() << std::endl;
    //std::cerr << "LVAL " << symbol->name() << ":" << symbol->type()->size() << ":" << symbol->offset() << std::endl;
  }

}

void mml::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  _pf.LDINT(); // depends on type size
}

void mml::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); // determine the new value
  if (node->type()->name() == cdk::TYPE_DOUBLE) {
    if (node->rvalue()->type()->name() == cdk::TYPE_INT) {
      _pf.I2D();
    }
    _pf.DUP64();
  }
  else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl); // where to store the value
  if (node->type()->name() != cdk::TYPE_DOUBLE) {
    _pf.STINT();
  }
  else {
    _pf.STDOUBLE();
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_program_node(mml::program_node * const node, int lvl) {
  // Note that MML doesn't have functions. Thus, it doesn't need
  // a function node. However, it must start in the main function.
  // The ProgramNode (representing the whole program) doubles as a
  // main function node.

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");
  _functionLabels.push_back(0);
  _functionReturnTypes.push_back(cdk::primitive_type::create(4, cdk::TYPE_INT));
  _functionTypes.push_back(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)));
  int size = countVariablesInBlock(node->block());
  std::cout << "size: " << size << std::endl;
  _pf.ENTER(size);
  node->block()->accept(this, lvl);
  _functionLabels.pop_back();
  _functionReturnTypes.pop_back();
  _functionTypes.pop_back();
  // end the main function
  _pf.INT(0);
  _pf.STFVAL32();
  _pf.LEAVE();
  _pf.RET();

  // these are just a few library function imports
  _pf.EXTERN("readi");
  _pf.EXTERN("printi");
  _pf.EXTERN("prints");
  _pf.EXTERN("println");
  _pf.EXTERN("printd");
  for (size_t i = 0; i < _external.size(); i++) {
    _pf.EXTERN(_external[i]);
  }
  
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_evaluation_node(mml::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.TRASH(8); // delete the evaluated value
  }
  else if (node->argument()->is_typed(cdk::TYPE_VOID)) {
    std::cout << "void" << std::endl;
  }
  else {
    _pf.TRASH(4); // delete the evaluated value
  }
}

void mml::postfix_writer::do_print_node(mml::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  
  auto arguments = node->arguments();
  arguments->accept(this, lvl); // determine the value to print
  for (size_t i = 0; i < arguments->size(); i++) {
    auto arg = dynamic_cast<cdk::expression_node*>(arguments->node(i));
    if (arg->is_typed(cdk::TYPE_INT)) {
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (arg->is_typed(cdk::TYPE_STRING)) {
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else if (arg->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.CALL("printd");
      _pf.TRASH(8); // delete the printed value
    }
    else {
      throw "invalid type for print argument at index" + std::to_string(i) ;
    }
  }
  if (node->newline()) {
    _pf.CALL("println"); // print a newline
  }
}


//---------------------------------------------------------------------------

void mml::postfix_writer::do_variable_declaration_node(mml::variable_declaration_node *const node, int lvl) {
  auto id = node->identifier();
  if (node->qualifier() == tFOREIGN) {
    _external.push_back(id);
    ASSERT_SAFE_EXPRESSIONS;
    return;
  }
  else if (node->qualifier() == tFORWARD) {
    _external.push_back(id);
    return;
  }
  ASSERT_SAFE_EXPRESSIONS;
  
  // type size?
  int offset = 0, typesize = node->type()->size(); // in bytes
  std::cout << "ARG: " << id << ", " << typesize << std::endl;
  int offsetIndex = _offset.size()-1;
  std::cout << "Block: " << std::to_string(_inBlock) << std::endl;
  if (_inBlock > 0) {
    std::cout << "IN BODY" << std::endl;
    _offset[offsetIndex] -= typesize;
    offset = _offset[offsetIndex];
  } else if (_inFunctionArgs) {
    std::cout << "IN ARGS " << std::to_string(offsetIndex) << std::endl;
    offset = _offset[offsetIndex];
    _offset[offsetIndex] += typesize;
  } else {
    std::cout << "GLOBAL!" << std::endl;
    offset = 0; // global variable
  }
  std::cout << "OFFSET: " << id << ", " << offset << std::endl;

  auto symbol = new_symbol();
  if (symbol) {
    symbol->offset(offset);
    reset_new_symbol();
  }

  if (_inBlock > 0) {
    // if we are dealing with local variables, then no action is needed
    // unless an initializer exists
    if (node->initializer()) {
      if (node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL)){
        _pf.SADDR(mklbl(_lbl+1));
        node->initializer()->accept(this, lvl);
        _pf.TRASH(4);
      }
      else {
        node->initializer()->accept(this, lvl);
      }
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_FUNCTIONAL)) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        std::cerr << "cannot initialize" << std::endl;
      }
    }
  } else {
    if (_functionLabels.size() == 0) {
      if (node->initializer() == nullptr) {
        _pf.BSS();
        _pf.ALIGN();
        _pf.GLOBAL(id, _pf.OBJ());
        _pf.LABEL(id);
        _pf.SALLOC(typesize);
      } else {

        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_FUNCTIONAL)) {
          _pf.DATA();
          _pf.ALIGN();
          _pf.GLOBAL(id, _pf.OBJ());
          _pf.LABEL(id);

          if (node->is_typed(cdk::TYPE_DOUBLE)) {
            if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
              node->initializer()->accept(this, lvl);
            } else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
              cdk::integer_node *dclini = dynamic_cast<cdk::integer_node*>(node->initializer());
              cdk::double_node ddi(dclini->lineno(), dclini->value());
              ddi.accept(this, lvl);
            } else {
              std::cerr << node->lineno() << ": '" << id << "' has bad initializer for real value" << std::endl;
            }
          }
          else {
            if (node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL)){
              _pf.SADDR(mklbl(_lbl+1));
              node->initializer()->accept(this, lvl);
              _pf.TRASH(4);
            }
            else {
              node->initializer()->accept(this, lvl);
            }
          }
        } else if (node->is_typed(cdk::TYPE_STRING)) {
          _pf.DATA();
          _pf.ALIGN();
          _pf.GLOBAL(id, _pf.OBJ());
          _pf.LABEL(id);
          node->initializer()->accept(this, lvl);
        } else {
          std::cerr << node->lineno() << ": '" << id << "' has unexpected initializer" << std::endl;
        }

      }

    }
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_return_node(mml::return_node * const node, int lvl) { 
  ASSERT_SAFE_EXPRESSIONS;
  if (_functionReturnTypes.size() == 0) {
    throw "return statement called ouside of a function body";
  }
  auto expected = _functionReturnTypes.back();
  if (expected->name() != cdk::TYPE_VOID) {
    node->expression2return()->accept(this, lvl + 2);
    if (expected->name() == cdk::TYPE_INT || expected->name() == cdk::TYPE_STRING
        || expected->name() == cdk::TYPE_POINTER || expected->name() == cdk::TYPE_FUNCTIONAL) {
      // IS IT REALLY SO SIMPLE TO RETURN A FUNCTION
      _pf.STFVAL32();
    } else if (expected->name() == cdk::TYPE_DOUBLE) {
      if (node->expression2return()->type()->name() == cdk::TYPE_INT) _pf.I2D();
      _pf.STFVAL64();
    } else {
      std::cerr << node->lineno() << ": should not happen: unknown return type" << std::endl;
    }
  }
  _pf.LEAVE();
  _pf.RET();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_nullptr_node(mml::nullptr_node *const node, int lvl) {
  // a pointer is a 32-bit integer
  if (_inBlock) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
 }

//---------------------------------------------------------------------------

void mml::postfix_writer::do_stack_alloc_node(mml::stack_alloc_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(node->type()->size());
  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_next_node(mml::next_node * const node, int lvl) {
  int target;
  if (_whileCondLabels.size() == 0) {
    throw "next statement outside a while block";
  }
  if (_whileCondLabels.size() < node->cycle()) {
    throw "target for next statement is outside a while block";
  }
  target = _whileCondLabels[_whileCondLabels.size() - node->cycle()];
  _pf.JMP(mklbl(target));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_stop_node(mml::stop_node * const node, int lvl) {
  int target;
  if (_whileEndLabels.size() == 0) {
    throw "stop statement outside a while block";
  }
  if (_whileEndLabels.size() < node->cycle()) {
    throw "target for stop statement is outside a while block";
  }
  target = _whileEndLabels[_whileEndLabels.size() - node->cycle()];
  _pf.JMP(mklbl(target));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_sizeof_node(mml::sizeof_node *const node, int lvl) { 
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->expression()->type()->size());
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_block_node(mml::block_node *const node, int lvl) {
  _offset.push_back(0);
  _inBlock++;
  _symtab.push();
  if (node->declarations()) node->declarations()->accept(this, lvl);
  if (node->instructions()) node->instructions()->accept(this, lvl);
  _symtab.pop();
  _inBlock--;
  _offset.pop_back();
 }

//---------------------------------------------------------------------------

void mml::postfix_writer::do_index_node(mml::index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
  _pf.INT(3);
  _pf.SHTL();
  _pf.ADD();
 } 

//---------------------------------------------------------------------------

void mml::postfix_writer::do_identity_node(mml::identity_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_address_node(mml::address_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
} 

//---------------------------------------------------------------------------

void mml::postfix_writer::do_function_def_node(mml::function_def_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int functionLabel = ++_lbl;
  std::cout << "FUNCTION: " << node->type()->to_string() << " " << functionLabel << std::endl;
  _functionReturnTypes.push_back(cdk::functional_type::cast(node->type())->output(0));
  _functionTypes.push_back(node->type());
  _functionLabels.push_back(functionLabel);
  // DO ARGS
  _inFunctionArgs = true;
  _offset.push_back(8); // for args
  _symtab.push();
  node->arguments()->accept(this, lvl); // lvl might be incorrect
  _offset.pop_back();
  _inFunctionArgs = false;
  _pf.TEXT(mklbl(functionLabel));
  _pf.ALIGN();

  _pf.LABEL(mklbl(functionLabel));
  int size = countVariablesInBlock(node->block());
  _pf.ENTER(size);

  node->block()->accept(this, lvl);
  // DO BLOCK
  if (_functionReturnTypes.back()->name() == cdk::TYPE_VOID) {
    _pf.LEAVE();
    _pf.RET();
  }
  _symtab.pop();
  _functionReturnTypes.pop_back();
  _functionTypes.pop_back();
  _functionLabels.pop_back();
  _pf.SADDR(mklbl(functionLabel));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_function_call_node(mml::function_call_node *const node, int lvl) { 
  ASSERT_SAFE_EXPRESSIONS;

  size_t argsSize = 0;
  if (node->arguments()->size() > 0) {
    for (int i = node->arguments()->size() - 1; i >= 0; i--) {
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
      arg->accept(this, lvl + 2);
      /*if (node->function->arguments()->node(i)->is_typed(cdk::TYPE_DOUBLE) && arg->is_typed(cdk::TYPE_INT)) {
        _pf.I2D();
      }*/
      argsSize += arg->type()->size();
    }
  }
  if (!node->recursive()) {
    node->function()->accept(this, lvl);
  }
  if (node->recursive()) {
    _pf.CALL(mklbl(_functionLabels[_functionLabels.size() - 1]));
  } else {
    // getLabel
    _pf.BRANCH();
  }
  if (argsSize != 0) {
    std::cout << "bytes of arguments: " << argsSize << std::endl;
    _pf.TRASH(argsSize);
  }

  std::shared_ptr<cdk::basic_type> retType = node->type();
  if (retType->name() == cdk::TYPE_DOUBLE) {
    _pf.LDFVAL64();
  } else if (retType->name() == cdk::TYPE_VOID) {
    std::cout << "void function call" << std::endl;
  }
  else {
    _pf.LDFVAL32();
  }
}

//---------------------------------------------------------------------------
void mml::postfix_writer::do_read_node(mml::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // DO WE ASSERT AND DO WE ADAPT THE READ TYPE TO THE VARIABLE TYPE?
  if (_lvalueType == cdk::TYPE_DOUBLE) {
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else if (_lvalueType == cdk::TYPE_INT) {
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else {
    std::cerr << "FATAL: " << node->lineno() << ": cannot read type" << std::endl;
    return;
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_while_node(mml::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // WHILE COND
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl2 = ++_lbl));

  _whileCondLabels.push_back(lbl1);
  _whileEndLabels.push_back(lbl2);

  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2)); // WHILE END

  _whileCondLabels.pop_back();
  _whileEndLabels.pop_back();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_node(mml::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_else_node(mml::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}

int mml::postfix_writer::countVariablesInBlock(mml::block_node* const node) {
  int count = 0;
  if (node->declarations()) {
    _symtab.push();
    for (size_t i = 0; i < node->declarations()->size(); i++) {
      mml::variable_declaration_node* declaration = dynamic_cast<mml::variable_declaration_node*>(node->declarations()->node(i));
      if (!declaration->type()) {
        try {
          if (_functionReturnTypes.size() > 0) {
            std::cout << "the good one" << std::endl;
            mml::type_checker checker(_compiler, _symtab, _functionReturnTypes.back(), _functionTypes.back(), this);
            declaration->accept(&checker, 0);
          }
          else {
            mml::type_checker checker(_compiler, _symtab, nullptr, nullptr, this);
            declaration->accept(&checker, 0);
          }
        }
        catch (const std::string &problem) {
          std::cerr << (node)->lineno() << ": " << problem << std::endl;
          return 0;
        }
      }
      std::cout << "passing through variable declaration" << std::endl;
      std::cout << "variable declaration size: " << declaration->type()->size() << std::endl;
      if (declaration->type()) {
        std::cout << "but not variable declaration type: " << std::endl;
        count += declaration->type()->size();
      }  
    }
    _symtab.pop();
  }
  return count;
}