/*!
 *@file Function.h
 *@brief 鍑芥暟鎺ュ彛澶存枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */

#ifndef SYSYC_FUNCTION_H
#define SYSYC_FUNCTION_H

#include <cassert>
#include <cstddef>
#include <iterator>
#include <list>
#include <map>

#include "BasicBlock.h"
#include "Module.h"
#include "Type.h"
#include "User.h"

class Module;
class Argument;
class BasicBlock;
class Type;
class FunctionType;

/**
 * @brief 函数
 * @note 绠＄悊鍩烘湰鍧楋紝浠庡睘浜庢ā鍧?
 */
class Function : public Value {
public:
  /**
   * @brief Construct a new Function object
   *
   * @param ty 函数类型指针
   * @param name 函数名称
   * @param parent 鎵€灞炴ā鍧?
   */
  Function(FunctionType *ty, const std::string &name, Module *parent);
  /**
   * @brief Destroy the Function object
   *
   */
  ~Function();
  /**
   * @brief 创建函数对象
   *
   * @param ty 函数类型
   * @param name 函数名称
   * @param parent 鎵€灞炴ā鍧?
   * @return Function* 函数对象指针
   */
  static Function *create(FunctionType *ty, const std::string &name,
                          Module *parent);
  /**
   * @brief Get the function type object，获取函数的返回类型
   *
   * @return FunctionType* 函数类型
   */
  FunctionType *get_function_type() const;
  /**
   * @brief Get the return type object锛岃幏鍙栧嚱鏁拌繑鍥炵被鍨?
   *
   * @return Type* 杩斿洖杩斿洖鍊肩被鍨?
   */
  Type *get_return_type() const;
  /**
   * @brief 娣诲姞鍩烘湰鍧?
   *
   * @param bb 鍩烘湰鍧楁寚閽?
   */
  void add_basic_block(BasicBlock *bb);
  /**
   * @brief Get the num of args object，获取函数的参数个数
   *
   * @return unsigned 鍑芥暟鍙傛暟鐨勪釜鏁?
   */
  unsigned get_num_of_args() const;
  /**
   * @brief 获取函数参数数组的迭代器
   *
   * @return std::list<Argument *>::iterator
   * 榛樿杩斿洖鍑芥暟鍙傛暟鏁扮粍鐨勮凯浠ｅ櫒锛屾寚鍚戠涓€涓厓绱?
   */
  std::list<Argument *>::iterator arg_begin() { return arguments_.begin(); }
  /**
   * @brief 获取函数参数数组的迭代器
   *
   * @return std::list<Argument *>::iterator
   * 杩斿洖鍑芥暟鍙傛暟鏁扮粍鐨勮凯浠ｅ櫒锛屾寚鍚戞渶鍚庝竴涓厓绱?
   */
  std::list<Argument *>::iterator arg_end() { return arguments_.end(); }
  /**
   * @brief Get the num basic blocks object锛岃幏鍙栧嚱鏁扮鐞嗗熀鏈潡鐨勬暟閲?
   *
   * @return unsigned 锛屽嚱鏁扮鐞嗙殑鍩烘湰蹇暟閲?
   */
  unsigned get_num_basic_blocks() const;
  /**
   * @brief Get the parent object锛岃幏鍙栧嚱鏁版墍灞炴ā鍧?
   *
   * @return Module* ，模块指针，默认为本文件
   */
  Module *get_parent() const;
  /**
   * @brief 鍒犻櫎鍑芥暟鍐呯殑鎸囧畾鍩烘湰鍧?
   *
   * @param bb 鍩烘湰鍧楁寚閽?
   */
  void remove(BasicBlock *bb);
  /**
   * @brief Get the entry block object，获取基本块入口
   *
   * @return BasicBlock* 鍩烘湰鍧楁寚閽?
   *
   * @note 获取管理的基本块链第一个基本块
   */
  BasicBlock *get_entry_block() { return *basic_blocks_.begin(); }
  /**
   * @brief Get the basic blocks object锛岃幏鍙栧熀鏈潡閾?
   *
   * @return std::list<BasicBlock *>& 鍩烘湰鍧楅摼鐨勫紩鐢?
   */
  std::list<BasicBlock *> &get_basic_blocks() { return basic_blocks_; }
  /**
   * @brief Get the args object锛岃幏鍙栧弬鏁板垪琛?
   *
   * @return std::list<Argument *>& 参数列表引用
   */
  std::list<Argument *> &get_args() { return arguments_; }
  /**
   * @brief 判断函数是否声明
   *
   * @return true 鍖呭惈鍩烘湰鍧?
   * @return false 不包含基本块
   */
  bool is_declaration() { return basic_blocks_.empty(); }
  /**
   * @brief Set the instr name object，为参数和基本块设置名称
   *
   */
  void set_instr_name();
  /**
   * @brief 打印函数
   *
   * @return std::string 瀛楃涓?
   */
  std::string print();

private:
  std::list<BasicBlock *> basic_blocks_; // basic blocks
  std::list<Argument *> arguments_;      // arguments
  Module *parent_;
  unsigned seq_cnt_;
  /**
   * @brief 创建函数参数列表
   *
   */
  void build_args();
};

/**
 * @brief Argument of Function, does not contain actual value，函数参数类
 *
 */
class Argument : public Value {
public:
  /**
   * @brief Construct a new Argument object
   *
   * @param ty 参数类型
   * @param name 参数名称
   * @param f 鎵€灞炲嚱鏁?
   * @param arg_no 参数列表中的位置
   */
  explicit Argument(Type *ty, const std::string &name = "",
                    Function *f = nullptr, unsigned arg_no = 0)
      : Value(ty, name), parent_(f), arg_no_(arg_no) {}
  /**
   * @brief Destroy the Argument object锛屾瀽鏋勫嚱鏁?
   *
   */
  ~Argument() {}
  /**
   * @brief Get the parent object锛岃幏鍙栧弬鏁版墍灞炲嚱鏁?
   *
   * @return const Function* 常量函数指针
   */
  inline const Function *get_parent() const { return parent_; }
  /**
   * @brief Get the parent object锛岃幏鍙栧弬鏁版墍灞炲嚱鏁?
   *
   * @return Function* 函数指针
   */
  inline Function *get_parent() { return parent_; }
  /**
   * @brief 娣辨嫹璐?
   *
   * @return Argument* 锛岃幏鍙栨柊鐨勫弬鏁板璞℃寚閽?
   */
  Argument *deepcopy() { return new Argument(type_, name_, parent_, arg_no_); }
  /**
   * @brief Get the arg no object锛岃幏鍙栧弬鏁板垪琛ㄥ弬鏁颁釜鏁?
   *
   * @return unsigned 参数个数
   * @note "void foo(int a, float b)" a is 0 and b is 1.
   */
  unsigned get_arg_no() const {
    assert(parent_ && "can't get number of unparented arg");
    return arg_no_;
  }
  /**
   * @brief 打印参数列表
   *
   * @return std::string 瀛楃涓?
   */
  virtual std::string print() override;

private:
  Function *parent_;
  unsigned arg_no_; // argument No.
};

#endif // SYSYC_FUNCTION_H


