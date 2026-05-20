/*!
 *@file Constant.h
 *@brief 常量接口定义文件
 *@version 1.0.0
 *@date 2022-10-04
 */
#include "Constant.h"
#include "Module.h"
#include <iostream>
#include <sstream>
/*!
 *@brief 甯搁噺鏁存暟绫?2浣嶅垱寤哄嚱鏁?
 *@param val 甯搁噺鍊?
 *@param m 鎵€灞炴ā鍧?
 *@return 甯搁噺绫诲璞℃寚閽?
 */
ConstantInt *ConstantInt::get(int val, Module *m) {
  return new ConstantInt(Type::get_int32_type(m), val);
}
/*!
 *@brief 甯搁噺鏁存暟绫?浣嶅垱寤哄嚱鏁?
 *@param val 甯搁噺鍊?
 *@param m 鎵€灞炴ā鍧?
 *@return 甯搁噺绫诲璞℃寚閽?
 */
ConstantInt *ConstantInt::get(bool val, Module *m) {
  return new ConstantInt(Type::get_int1_type(m), val ? 1 : 0);
}
/*!
 *@brief 鎵撳嵃甯搁噺绫诲彉閲?
 *@return 瀛楃涓?
 *@note
 *---------
 *获取常量类型
 *&emsp; **if** 鍒ゅ畾涓烘暣鏁板父閲忕被鍨嬪苟涓斾负1涓虹殑甯冨皵绫诲瀷锛?
 *&emsp;&emsp; 娣诲姞true/flase瀛楃涓?
 *&emsp; 鍒ゅ畾涓?2浣嶇被鍨?
 *&emsp; 将其数组转换为字符串输出
 *&emsp; 杩斿洖瀛楃涓?
 */
std::string ConstantInt::print() {
  std::string const_ir;
  Type *ty = this->get_type();
  if (ty->is_integer_type() &&
      static_cast<IntegerType *>(ty)->get_num_bits() == 1) {
    // int1
    const_ir += (this->get_value() == 0) ? "false" : "true";
  } else {
    // int32
    const_ir += std::to_string(this->get_value());
  }
  return const_ir;
}

/*!
 *@brief 甯搁噺鏁存暟绫绘瀯閫犲嚱鏁?
 *@param ty 常量类型
 *@param val 甯搁噺鏁板€肩被鍨嬫暟缁?
 *@return 鑷韩绫诲璞?
 *constant int array
 */
ConstantArray::ConstantArray(ArrayType *ty, const std::vector<Constant *> &val)
    : Constant(ty, "", val.size()) {
  for (int i = 0; i < (int)val.size(); i++)
    set_operand(i, val[i]);
  this->const_array.assign(val.begin(), val.end());
}
/*!
 *@brief 鑾峰彇甯搁噺鏁扮粍鎸囧畾绱㈠紩鐨勫父閲忔暟鍊?
 *@param index 索引
 *@return 甯搁噺绫绘寚閽?
 *constant int array
 */
Constant *ConstantArray::get_element_value(int index) {
  return this->const_array[index];
}
/*!
 *@brief 常量数组类的创建函数
 *@param ty 鏁扮粍鍏冪礌鐨勭被鍨?
 *@param val 甯搁噺绫绘暟缁?
 *@return 甯搁噺鏁扮粍绫绘寚閽?
 *constant int array
 */
ConstantArray *ConstantArray::get(ArrayType *ty,
                                  const std::vector<Constant *> &val) {
  return new ConstantArray(ty, val);
}
/*!
 *@brief 甯搁噺鏁扮粍绫绘墦鍗板嚱鏁?
 *@return 瀛楃涓?
 *constant int array
 */
std::string ConstantArray::print() {
  std::string const_ir;
  const_ir += "[";
  for (int i = 0; i < static_cast<int>(this->get_size_of_array()); i++) {
    const_ir += get_element_value(i)->get_type()->print();
    const_ir += " ";
    const_ir += get_element_value(i)->print();
    const_ir += ", ";
  }
  const_ir.pop_back();
  const_ir.pop_back();
  const_ir += "]";
  return const_ir;
}
/*!
 *@brief 甯搁噺鏁存暟绫绘瀯閫犲嚱鏁?
 *@param ty 常量类型
 *@param val 甯搁噺鏁板€肩被鍨嬫暟缁?
 *@return 鑷韩绫诲璞?
 *constant int zero
 */
ConstantZero *ConstantZero::get(Type *ty, Module *m) {
  return new ConstantZero(ty);
}
/*!
 *@brief 鎵撳嵃甯搁噺闆跺€?
 *@return 瀛楃涓?
 *constant int zero
 */
std::string ConstantZero::print() { return "zeroinitializer"; }

