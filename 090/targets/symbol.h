#ifndef __MML_TARGETS_SYMBOL_H__
#define __MML_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace mml {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    long _value; // hack!
    int _offset = 0; // 0 (zero) means global variable/function
    bool _constant; // is it a constant?
    int _qualifier; // qualifiers: public, forward, "private" (i.e., none)
    bool _initialized;

  public:
    symbol(bool constant,int qualifier,std::shared_ptr<cdk::basic_type> type, const std::string &name , bool initialized, long value) :
         _type(type), _name(name) , _value(value), _constant(constant), _qualifier(qualifier), _initialized(initialized) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    long value() const {
      return _value;
    }
    long value(long v) {
      return _value = v;
    }
    int qualifier() const {
      return _qualifier;
    }
    const std::string& identifier() const {
      return name();
    }
    bool initialized() const {
      return _initialized;
    }
    int offset() const {
      return _offset;
    }
    void offset(int off) {
      _offset = off;
    }
    bool global() const {
      return _offset == 0;
    }
  };

} // mml

#endif
