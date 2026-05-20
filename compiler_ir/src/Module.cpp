/*!
 *@file Module.h
 *@brief 模块接口定义文件
 *@version 1.0.0
 *@date 2022-10-04
 */
#include "Module.h"

#include <utility>

Module::Module(std::string name) : module_name_(std::move(name)) {
  /// @brief 创建类型指针对象
  /// @param name
  void_ty_ = new Type(Type::VoidTyID, this);
  label_ty_ = new Type(Type::LabelTyID, this);
  int1_ty_ = new IntegerType(1, this);
  int32_ty_ = new IntegerType(32, this);
  float32_ty_ = new FloatType(this);

  /// @brief id 涓?瀛楃涓茬殑鏄犲皠娣诲姞
  instr_id2string_.insert({Instruction::ret, "ret"});
  instr_id2string_.insert({Instruction::br, "br"});

  instr_id2string_.insert({Instruction::add, "add"});
  instr_id2string_.insert({Instruction::sub, "sub"});
  instr_id2string_.insert({Instruction::mul, "mul"});
  instr_id2string_.insert({Instruction::sdiv, "sdiv"});
  instr_id2string_.insert({Instruction::mod, "srem"});

  instr_id2string_.insert({Instruction::cmp, "icmp"});

  instr_id2string_.insert({Instruction::alloca, "alloca"});
  instr_id2string_.insert({Instruction::load, "load"});
  instr_id2string_.insert({Instruction::store, "store"});
  instr_id2string_.insert({Instruction::phi, "phi"});
  instr_id2string_.insert({Instruction::call, "call"});
  instr_id2string_.insert({Instruction::getelementptr, "getelementptr"});
  instr_id2string_.insert({Instruction::zext, "zext"});
}

/**
 * @brief Destroy the Module:: Module object 析构函数
 *
 */
Module::~Module() {
  delete void_ty_;
  delete label_ty_;
  delete int1_ty_;
  delete int32_ty_;
  delete float32_ty_;
}
/**
 * @brief Get the void type object，获取一个构建好的void类型指针
 *
 * @return Type*
 */
Type *Module::get_void_type() { return void_ty_; }
/**
 * @brief Get the label type object，获取一个构建好的label类型指针
 *
 * @return Type*
 */
Type *Module::get_label_type() { return label_ty_; }
/**
 * @brief Get the int1 type object，获取一个构建好的integer1类型指针
 *
 * @return IntegerType*
 */
IntegerType *Module::get_int1_type() { return int1_ty_; }
/**
 * @brief Get the int32 type object，获取一个构建好的integer32类型指针
 *
 * @return IntegerType*
 */
IntegerType *Module::get_int32_type() { return int32_ty_; }
/**
 * @brief Get the pointer type object锛岃幏鍙栦竴涓瀯寤哄ソ鐨勬寚閽堢被鍨嬫寚閽?
 *
 * @param contained 鎸囬拡鎸囧悜鏁版嵁鐨勭被鍨?
 * @return PointerType*
 */
PointerType *Module::get_pointer_type(Type *contained) {
  if (pointer_map_.find(contained) == pointer_map_.end()) {
    pointer_map_[contained] = new PointerType(contained);
  }
  return pointer_map_[contained];
}
/**
 * @brief Get the array type object，获取一个构建好的array类型指针
 *
 * @param contained 数组元素类型
 * @param num_elements 数组元素个数
 * @return ArrayType*
 */
ArrayType *Module::get_array_type(Type *contained, unsigned num_elements) {
  if (array_map_.find({contained, num_elements}) == array_map_.end()) {
    array_map_[{contained, num_elements}] =
        new ArrayType(contained, num_elements);
  }
  return array_map_[{contained, num_elements}];
}
/**
 * @brief Get the int32 ptr type object，获取一个构建好的integer32指针类型指针
 *
 * @return PointerType*
 */
PointerType *Module::get_int32_ptr_type() {
  return get_pointer_type(int32_ty_);
}
/**
 * @brief Get the float type object，获取一个构建好的float类型指针
 *
 * @return FloatType*
 */
FloatType *Module::get_float_type() { return float32_ty_; }
/**
 * @brief Get the float ptr type object，获取一个构建好的float指针类型指针
 *
 * @return PointerType*
 */
PointerType *Module::get_float_ptr_type() {
  return get_pointer_type(float32_ty_);
}
/**
 * @brief 添加函数
 *
 * @param f 函数指针
 */
void Module::add_function(Function *f) { function_list_.push_back(f); }
/**
 * @brief Get the functions object锛岃幏鍙栧嚱鏁板垪琛?
 *
 * @return std::list<Function *> 函数列表
 */
std::list<Function *> Module::get_functions() { return function_list_; }
/**
 * @brief 娣诲姞鍏ㄥ眬閲?
 *
 * @param g 鍏ㄥ眬閲忔寚閽?
 */
void Module::add_global_variable(GlobalVariable *g) {
  global_list_.push_back(g);
}
/**
 * @brief Get the global variable object锛岃幏鍙栧叏灞€閲忔寚閽堟暟缁?
 *
 * @return std::list<GlobalVariable *> 鍏ㄥ眬閲忔寚閽堟暟缁?
 */
std::list<GlobalVariable *> Module::get_global_variable() {
  return global_list_;
}
/**
 * @brief Set the print name object，修正模块管理的函数下的名称
 *
 */
void Module::set_print_name() {
  for (auto func : this->get_functions()) {
    func->set_instr_name();
  }
  return;
}
/**
 * @brief 打印中间代码
 *
 * @return std::string
 */
std::string Module::print() {
  std::string module_ir;
  module_ir += "; ModuleID = '" + module_name_ + "'\n";
  module_ir += "source_filename = \"" + source_file_name_ + "\"\n";
  for (auto global_val : this->global_list_) {
    module_ir += global_val->print();
    module_ir += "\n";
  }
  bool has_decl = false;
  bool has_def = false;
  for (auto func : this->function_list_) {
    if (func->is_declaration()) {
      has_decl = true;
      module_ir += func->print();
    } else {
      has_def = true;
    }
  }
  if (has_decl && has_def) {
    module_ir += "\n";
  }
  for (auto func : this->function_list_) {
    if (!func->is_declaration()) {
      module_ir += func->print();
      module_ir += "\n";
    }
  }
  return module_ir;
}

