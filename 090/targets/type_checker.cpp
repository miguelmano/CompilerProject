#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#include "mml_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

bool typesAreEqual(cdk::basic_type *type1, cdk::basic_type *type2) {
  if (type1->name() != type2->name()) {
    return false;
  }
  /*
  if (type1->name() == cdk::TYPE_FUNCTIONAL) {
    type1 = cdk::functional_type::cast(type1);
    type2 = cdk::functional_type::cast(type2);
    if (type1->output()->name() != type2->output()->name()) {
      return false;
    }
    for (size_t i = 0; i < type1->input()->size(); i++) {
      if (type1->parameters()->at(i)->name() != type2->parameters()->at(i)->name()) {
        return false;
      }
    }
    return true;
  }*/
  return true;
}

//---------------------------------------------------------------------------

void mml::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void mml::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void mml::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}
void mml::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}
void mml::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void mml::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) { // Fixme: Add Doubles
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in MML, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE)) throw std::string("wrong type in left argument of binary expression");
  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE)) throw std::string("wrong type in right argument of binary expression");

  // in MML, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<mml::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void mml::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void mml::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl);
  
  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->type()) { // stackalloc (is this enough?)
    node->rvalue()->type(node->lvalue()->type());
  }
  
  if (node->lvalue()->type() != node->rvalue()->type()) {
    if (node->lvalue()->type()->name() != cdk::TYPE_DOUBLE && node->rvalue()->type()->name() != cdk::TYPE_INT) {
      throw std::string("wrong type in right argument of assignment expression");
    }
    else {
      // DO CAST
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    }
  }
  else { // THIS MIGHT BE INCOMPLETE (Stas)
    node->type(node->rvalue()->type());
  }
  // in MML, expressions are always int
}

//---------------------------------------------------------------------------

void mml::type_checker::do_program_node(mml::program_node *const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_evaluation_node(mml::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void mml::type_checker::do_print_node(mml::print_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);
  
}

//---------------------------------------------------------------------------

void mml::type_checker::do_read_node(mml::read_node *const node, int lvl) {

}

//---------------------------------------------------------------------------

void mml::type_checker::do_variable_declaration_node(mml::variable_declaration_node *const node, int lvl) {
  const std::string &id = node->identifier();
  std::shared_ptr<mml::symbol> symbol = _symtab.find(id); //
  bool initialized = false;
  if (symbol != nullptr) {
    throw id + " redeclared";
  }
  if (node->initializer()) {
    node->initializer()->accept(this, lvl + 2);
    if (!node->initializer()->type()) { //stackalloc
      if (!node->type()) {
        throw "attempt at allocating a stack variable without a type";
      }
      node->initializer()->type(node->type());
    }
    if (!node->type()) {
      node->type(node->initializer()->type());
      std::cout << "type: AUTO" << node->type()->to_string() << std::endl;
    } 
    if (node->initializer()->type() != node->type()) {
      // The above comparison is not sufficient for functions or pointers
      if (node->type()->name() == cdk::TYPE_POINTER) {
        if (node->initializer()->type()->name() != cdk::TYPE_POINTER) {
          throw "wrong type for initializer (got " + node->initializer()->type()->to_string() + " expected " + node->type()->to_string() + ")";
        }
        // might have to check if the pointer type is of the correct type
      }
      throw "wrong type for initializer (got " + node->initializer()->type()->to_string() + " expected " + node->type()->to_string() + ")";
    }
    initialized = true;
  }
  // use the type from the declaration to create a new symbol
  symbol = std::make_shared<mml::symbol>(false, node->qualifier(), node->type(), id, initialized, 0);
  _symtab.insert(id, symbol);
  _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
}

//---------------------------------------------------------------------------

void mml::type_checker::do_nullptr_node(mml::nullptr_node *const node, int lvl) {
  node->type(cdk::primitive_type::create(4, cdk::TYPE_POINTER));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_block_node(mml::block_node *const node, int lvl) { }

//---------------------------------------------------------------------------

void mml::type_checker::do_stack_alloc_node(mml::stack_alloc_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->type()->name() != cdk::TYPE_INT) {
    throw "wrong type for argument of allocation expression (integer expected)";
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_index_node(mml::index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  if (node->base()->type()->name() != cdk::TYPE_POINTER) {
    throw "wrong type in left argument of index expression (pointer expected)";
  }
  node->index()->accept(this, lvl + 2);
  if (node->index()->type()->name() != cdk::TYPE_INT) {
    throw "wrong type in right argument of index expression (integer expected)";
  }
  node->type( cdk::reference_type::cast(node->base()->type())->referenced() );
}

//---------------------------------------------------------------------------

void mml::type_checker::do_identity_node(mml::identity_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_return_node(mml::return_node *const node, int lvl) {
  node->expression2return()->accept(this, lvl + 2);   //NOT SURE IF +2 OR +4 WHAT DOES THIS VALUE AFFECT?
  if (_functionReturnType->name() == cdk::TYPE_VOID && node->expression2return() != nullptr) {
    throw "returning non-void value from void function";
  }
  else if (_functionReturnType->name() != cdk::TYPE_VOID && node->expression2return() == nullptr) {
    throw "returning void value from non-void function";
  }
  else if (node->expression2return()->type() != _functionReturnType) {
    if (_functionReturnType->name() == cdk::TYPE_POINTER && node->expression2return()->type()->name() == cdk::TYPE_INT) {
      return;
    }
    if (_functionReturnType->name() == cdk::TYPE_DOUBLE && node->expression2return()->type()->name() == cdk::TYPE_INT) {
      return;
    }
    throw "wrong type in return expression. Expected " + _functionReturnType->to_string() + " but got " + node->expression2return()->type()->to_string();
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_stop_node(mml::stop_node *const node, int lvl) { }

//---------------------------------------------------------------------------

void mml::type_checker::do_next_node(mml::next_node *const node, int lvl) { }

//---------------------------------------------------------------------------

void mml::type_checker::do_sizeof_node(mml::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_function_def_node(mml::function_def_node *const node, int lvl) {
  // INTO VARIALBE DECLARATION
}

//---------------------------------------------------------------------------

void mml::type_checker::do_function_call_node(mml::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;
  
  if (node->recursive()) {
    node->type(_functionReturnType);
    _functionType->name();
    if (_functionType->name() != cdk::TYPE_FUNCTIONAL) {
      throw "the expression being called is not a function";
    }
  }
  else {
    node->function()->accept(this, lvl + 2);
    node->type(cdk::functional_type::cast(node->function()->type())->output(0));
    if (node->function()->type()->name() != cdk::TYPE_FUNCTIONAL) {
      throw "the expression being called is not a function";
    }
  }
  std::shared_ptr<cdk::functional_type> function;
  if (node->recursive()) {
    function = cdk::functional_type::cast(_functionType);
  }
  else {
    function = cdk::functional_type::cast(node->function()->type());
  }if (node->arguments()->size() != function->input_length()) {
    throw "wrong number of arguments in call to function. Expected " + std::to_string(function->input_length()) + " but got " + std::to_string(node->arguments()->size());
  }
  node->arguments()->accept(this, lvl + 2);
  for (size_t i = 0; i < node->arguments()->size(); i++) {
    if (node->argument(i)->type()->name() == cdk::TYPE_INT && function->input(i)->name() == cdk::TYPE_DOUBLE) {
      continue;
    }
    if (node->argument(i)->type()->name() != function->input(i)->name()) {
      throw "wrong type for argument " + std::to_string(i + 1) + " in call to function. Expected " + function->input(i)->to_string() + " but got " + node->argument(i)->type()->to_string();
      // what alternative information can we give if we don't know the function name
    }
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_address_node(mml::address_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_while_node(mml::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_if_node(mml::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void mml::type_checker::do_if_else_node(mml::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}
