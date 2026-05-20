/*!
 *@file GlobalVariable.h
 *@brief 全局变量接口定义文件
 *@version 1.0.0
 *@date 2022-10-04
 */
#include "GlobalVariable.h"
#include "IRprinter.h"

/*!
 *@brief 鍏ㄥ眬鍙橀噺鐨勬瀯閫犲嚱鏁?
 *@param name 全局变量名称
 *@param m 所从属模块
 *@param ty 鍙橀噺鐨勭被鍨?
 *@param is_const 鏄惁涓哄父閲?
 *@param init 常量指针
 *@return 当前对象本身
 *@note
 *---------
 *在所属模块m中添加全局变量
 *全局变量如果有初始值，进行初始赋值，操作符第一个位置为init
 */
GlobalVariable::GlobalVariable(std::string name, Module *m, Type *ty,
                               bool is_const, Constant *init)
    : User(ty, name, init != nullptr), is_const_(is_const), init_val_(init) {
  m->add_global_variable(this);
  if (init) {
    this->set_operand(0, init);
  }
} // global操作数为initval

/*!
 *@brief 鍏ㄥ眬鍙橀噺鐨勫垱寤哄嚱鏁?
 *@param name 全局变量名称
 *@param m 所从属模块
 *@param ty 鍙橀噺鐨勭被鍨?
 *@param is_const 鏄惁涓哄父閲?
 *@param init 常量指针
 *@return 当前对象本身
 *@note
 *-------
 *鍏ㄥ眬鍙橀噺鐨勫垱寤?
 *榛樿鍙橀噺绫诲瀷涓烘寚閽堢被鍨嬶紝鏃犲垵鍊?
 */
GlobalVariable *GlobalVariable::create(std::string name, Module *m, Type *ty,
                                       bool is_const,
                                       Constant *init = nullptr) {
  return new GlobalVariable(name, m, PointerType::get(ty), is_const, init);
}

/*!
 *@brief 打印全局变量
 *@return 瀛楃涓?
 *@note
 *--------
 *初始化字符串
 *添加名称
 *添加常量类型
 *娣诲姞鏁版嵁鎸囬拡鎵€鎸囧悜鏁版嵁鐨勭被鍨?
 *娣诲姞鍙橀噺鍒濆€?
 */
std::string GlobalVariable::print() {
  std::string global_val_ir;
  global_val_ir += print_as_op(this, false);
  global_val_ir += " = ";
  global_val_ir += (this->is_const() ? "constant " : "global ");
  global_val_ir += this->get_type()->get_pointer_element_type()->print();
  global_val_ir += " ";
  global_val_ir += this->get_init()->print();
  return global_val_ir;
}
