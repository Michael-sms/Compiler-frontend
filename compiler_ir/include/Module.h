/*!
 *@file Module.h
 *@brief 妯″潡鎺ュ彛澶存枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */

#ifndef SYSYC_MODULE_H
#define SYSYC_MODULE_H

#include <list>
#include <map>
#include <string>

#include "Function.h"
#include "GlobalVariable.h"
#include "Instruction.h"
#include "Type.h"
#include "Value.h"

class GlobalVariable;

/**
 * @brief 妯″潡绫伙紝涓棿缁撴瀯鐨勫ぇ绫?
 *
 */
class Module {
private:
  /// @brief 各基础类型指针
  IntegerType *int1_ty_;
  IntegerType *int32_ty_;
  Type *label_ty_;
  Type *void_ty_;
  FloatType *float32_ty_;

  /// @brief 鎸囬拡鏄犲皠鍥惧拰鏁扮粍鏄犲皠鍥?
  std::map<Type *, PointerType *> pointer_map_;
  std::map<std::pair<Type *, int>, ArrayType *> array_map_;

  /// @brief 全局变量列表
  /// The Global Variables in the module
  std::list<GlobalVariable *> global_list_;
  /// The Functions in the module
  std::list<Function *> function_list_;
  /// Symbol table for values
  std::map<std::string, Value *> value_sym_;
  /// Instruction from opid to string
  std::map<Instruction::OpID, std::string> instr_id2string_;
  /// Human readable identifier for the module
  std::string module_name_;
  /// Original source file name for module, for test and debug
  std::string source_file_name_;

public:
  /**
   * @brief Construct a new Module object
   *
   * @param name 模块名称
   */
  explicit Module(std::string name);
  /**
   * @brief Destroy the Module object
   *
   */
  ~Module();

  /**
   * @brief Get the void type object，获取一个构建好的void类型指针
   *
   * @return Type*
   */
  Type *get_void_type();
  /**
   * @brief Get the label type object，获取一个构建好的label类型指针
   *
   * @return Type*
   */
  Type *get_label_type();
  /**
   * @brief Get the int1 type object，获取一个构建好的integer1类型指针
   *
   * @return IntegerType*
   */
  IntegerType *get_int1_type();
  /**
   * @brief Get the int32 type object，获取一个构建好的integer32类型指针
   *
   * @return IntegerType*
   */
  IntegerType *get_int32_type();
  /**
   * @brief Get the int32 ptr type object，获取一个构建好的integer32指针类型指针
   *
   * @return PointerType*
   */
  PointerType *get_int32_ptr_type();
  /**
   * @brief Get the float type object，获取一个构建好的float类型指针
   *
   * @return FloatType*
   */
  FloatType *get_float_type();
  /**
   * @brief Get the float ptr type object，获取一个构建好的float指针类型指针
   *
   * @return PointerType*
   */
  PointerType *get_float_ptr_type();
  /**
   * @brief Get the pointer type object锛岃幏鍙栦竴涓瀯寤哄ソ鐨勬寚閽堢被鍨嬫寚閽?
   *
   * @param contained 鎸囬拡鎸囧悜鏁版嵁鐨勭被鍨?
   * @return PointerType*
   */
  PointerType *get_pointer_type(Type *contained);
  /**
   * @brief Get the array type object，获取一个构建好的array类型指针
   *
   * @param contained 数组元素类型
   * @param num_elements 数组元素个数
   * @return ArrayType*
   */
  ArrayType *get_array_type(Type *contained, unsigned num_elements);
  /**
   * @brief 添加函数
   *
   * @param f 函数指针
   */
  void add_function(Function *f);
  /**
   * @brief Get the functions object锛岃幏鍙栧嚱鏁板垪琛?
   *
   * @return std::list<Function *> 函数列表
   */
  std::list<Function *> get_functions();
  /**
   * @brief 娣诲姞鍏ㄥ眬閲?
   *
   * @param g 鍏ㄥ眬閲忔寚閽?
   */
  void add_global_variable(GlobalVariable *g);
  /**
   * @brief 鍒犻櫎鍏ㄥ眬閲?
   *
   * @param g 鍏ㄥ眬閲忔寚閽?
   */
  void delete_global_variable(GlobalVariable *g) { global_list_.remove(g); }
  /**
   * @brief Get the global variable object锛岃幏鍙栧叏灞€閲忔寚閽堟暟缁?
   *
   * @return std::list<GlobalVariable *> 鍏ㄥ眬閲忔寚閽堟暟缁?
   */
  std::list<GlobalVariable *> get_global_variable();
  /**
   * @brief Get the instr op name object锛岃幏鍙栨寚浠?
   *
   * @param instr 指令id
   * @return std::string ID的字符串表达
   */
  std::string get_instr_op_name(Instruction::OpID instr) {
    return instr_id2string_[instr];
  }
  /**
   * @brief Set the print name object，修正模块管理的函数下的名称
   *
   */
  void set_print_name();
  /**
   * @brief Set the source file name for IR header
   *
   * @param name 源文件名
   */
  void set_source_file_name(const std::string &name) {
    source_file_name_ = name;
  }
  /**
   * @brief 获取模块名称
   */
  const std::string &get_module_name() const { return module_name_; }
  /**
   * @brief 获取源文件名
   */
  const std::string &get_source_file_name() const { return source_file_name_; }
  /**
   * @brief 打印中间代码
   *
   * @return std::string
   */
  virtual std::string print();
};

#endif // SYSYC_MODULE_H

