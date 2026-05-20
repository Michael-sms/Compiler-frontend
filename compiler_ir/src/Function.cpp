/*!
 *@file Function.h
 *@brief 函数接口定义文件
 *@version 1.0.0
 *@date 2022-10-04
 */

#include "Function.h"
#include "IRprinter.h"
#include "Module.h"

/**
 * @brief Construct a new Function object
 *
 * @param ty 函数类型指针
 * @param name 函数名称
 * @param parent 鎵€灞炴ā鍧?
 * @note 鎵€灞炴ā鍧楁坊鍔犲嚱鏁版寚閽?
 * @note 函数创建参数列表
 */
Function::Function(FunctionType *ty, const std::string &name, Module *parent)
    : Value(ty, name), parent_(parent), seq_cnt_(0) {
  parent->add_function(this);
  build_args();
}

/**
 * @brief 创建函数对象
 *
 * @param ty 函数类型
 * @param name 函数名称
 * @param parent 鎵€灞炴ā鍧?
 * @return Function* 函数对象指针
 */
Function *Function::create(FunctionType *ty, const std::string &name,
                           Module *parent) {
  return new Function(ty, name, parent);
}

/**
 * @brief Get the function type object，获取函数的返回类型
 *
 * @return FunctionType* 函数类型
 */
FunctionType *Function::get_function_type() const {
  return static_cast<FunctionType *>(get_type());
}

/**
 * @brief Get the return type object锛岃幏鍙栧嚱鏁拌繑鍥炵被鍨?
 *
 * @return Type* 杩斿洖杩斿洖鍊肩被鍨?
 */
Type *Function::get_return_type() const {
  return get_function_type()->get_return_type();
}

/**
 * @brief Get the num of args object，获取函数的参数个数
 *
 * @return unsigned 鍑芥暟鍙傛暟鐨勪釜鏁?
 */
unsigned Function::get_num_of_args() const {
  return get_function_type()->get_num_of_args();
}

/**
 * @brief Get the num basic blocks object锛岃幏鍙栧嚱鏁扮鐞嗗熀鏈潡鐨勬暟閲?
 *
 * @return unsigned 锛屽嚱鏁扮鐞嗙殑鍩烘湰蹇暟閲?
 */
unsigned Function::get_num_basic_blocks() const { return basic_blocks_.size(); }

/**
 * @brief Get the parent object锛岃幏鍙栧嚱鏁版墍灞炴ā鍧?
 *
 * @return Module* ，模块指针，默认为本文件
 */
Module *Function::get_parent() const { return parent_; }

/**
 * @brief 鍒犻櫎鍑芥暟鍐呯殑鎸囧畾鍩烘湰鍧?
 *
 * @param bb 鍩烘湰鍧楁寚閽?
 * @note 删除phi节点对于基本块的使用
 * @note 鍒犻櫎鍓嶇疆鍩烘湰鍧椾腑瀵逛簬璇ュ熀鏈潡鐨勫悗缁?
 * @note 鍒犻櫎鍚庣户鍩烘湰鍧椾腑瀵逛簬璇ュ熀鏈潡鐨勫墠缁?
 */
void Function::remove(BasicBlock *bb) {
  basic_blocks_.remove(bb);
  std::vector<PhiInst *> phis;
  for (auto user : bb->get_use_list()) {
    auto phi = dynamic_cast<PhiInst *>(user.val_);
    if (phi != nullptr) {
      phis.push_back(phi);
    }
  }
  /// 删除phi节点对于基本块的使用
  for (auto phi : phis) {
    phi->remove_source(bb);
  }
  /// 鍒犻櫎鍓嶇疆鍩烘湰鍧椾腑瀵逛簬璇ュ熀鏈潡鐨勫悗缁?
  for (auto pre : bb->get_pre_basic_blocks()) {
    pre->remove_succ_basic_block(bb);
  }
  /// 鍒犻櫎鍚庣户鍩烘湰鍧椾腑瀵逛簬璇ュ熀鏈潡鐨勫墠缁?
  for (auto succ : bb->get_succ_basic_blocks()) {
    succ->remove_pre_basic_block(bb);
  }
}

/**
 * @brief 创建函数参数列表
 *
 * @note 获取函数参数类型
 * @note 鑾峰彇瑕佸垵濮嬪寲鐨勫弬鏁颁釜鏁?
 * @note 依次往空列表中压入参数
 */
void Function::build_args() {
  auto *func_ty = get_function_type();
  unsigned num_args = get_num_of_args();
  for (int i = 0; i < (int)num_args; i++) {
    arguments_.push_back(new Argument(func_ty->get_param_type(i), "", this, i));
  }
}

/**
 * @brief 娣诲姞鍩烘湰鍧?
 *
 * @param bb 鍩烘湰鍧楁寚閽?
 */
void Function::add_basic_block(BasicBlock *bb) { basic_blocks_.push_back(bb); }

/**
 * @brief Set the instr name object，为参数和基本块设置名称
 *
 * @note 閽堝鍑芥暟鐨勫弬鏁拌缃悕绉?
 * @note 针对函数内部的基本块设置名称，并针对每个基本块的指令设置名称
 */
void Function::set_instr_name() {
  /// 针对函数的参数设置名称，
  std::map<Value *, int> seq;
  for (auto arg : this->get_args()) {
    if (seq.find(arg) == seq.end()) {
      auto seq_num = seq.size() + seq_cnt_;
      if (arg->set_name("arg" + std::to_string(seq_num))) {
        seq.insert({arg, seq_num});
      }
    }
  }
  /// 针对函数内部的基本块设置名称，并针对每个基本块的指令设置名称
  for (auto bb : basic_blocks_) {
    if (seq.find(bb) == seq.end()) {
      auto seq_num = seq.size() + seq_cnt_;
      if (bb->set_name("label" + std::to_string(seq_num))) {
        seq.insert({bb, seq_num});
      }
    }
    for (auto instr : bb->get_instructions()) {
      if (!instr->is_void() && seq.find(instr) == seq.end()) {
        auto seq_num = seq.size() + seq_cnt_;
        if (instr->set_name("op" + std::to_string(seq_num))) {
          seq.insert({instr, seq_num});
        }
      }
    }
  }
  seq_cnt_ += seq.size();
}

/**
 * @brief 打印函数
 *
 * @return std::string 瀛楃涓?
 * @note 鍒ゆ柇鍑芥暟鏄０鏄庤繕鏄畾涔?
 * @note 渚濇娣诲姞鍑芥暟绫诲瀷鍜屽悕绉?
 * @note 鍑芥暟涓哄０鏄?瀹氫箟锛屼笉鍚屾坊鍔犺鍒?
 * @note 鍑芥暟涓哄０鏄庯紝鎹㈣缁撴潫/涓哄畾涔夛紝渚濇鎵撳嵃鍩烘湰鍧?
 */
std::string Function::print() {
  set_instr_name();
  std::string func_ir;
  if (this->is_declaration()) {
    func_ir += "declare ";
  } else {
    func_ir += "define ";
  }

  func_ir += this->get_return_type()->print();
  func_ir += " ";
  func_ir += print_as_op(this, false);
  func_ir += "(";

  // print arg
  if (this->is_declaration()) {
    for (int i = 0; i < static_cast<int>(this->get_num_of_args()); i++) {
      if (i)
        func_ir += ", ";
      func_ir += static_cast<FunctionType *>(this->get_type())
                     ->get_param_type(i)
                     ->print();
    }
  } else {
    for (auto arg = this->arg_begin(); arg != arg_end(); arg++) {
      if (arg != this->arg_begin()) {
        func_ir += ", ";
      }
      func_ir += static_cast<Argument *>(*arg)->print();
    }
  }
  func_ir += ")";

  // print bb
  if (this->is_declaration()) {
    func_ir += "\n";
  } else {
    func_ir += " {";
    func_ir += "\n";
    for (auto bb : this->get_basic_blocks()) {
      func_ir += bb->print();
    }
    func_ir += "}";
  }

  return func_ir;
}

/**
 * @brief 打印参数列表
 *
 * @return std::string 瀛楃涓?
 * @note 依次打印参数的类型和名称
 */
std::string Argument::print() {
  std::string arg_ir;
  arg_ir += this->get_type()->print();
  arg_ir += " %";
  arg_ir += this->get_name();
  return arg_ir;
}
