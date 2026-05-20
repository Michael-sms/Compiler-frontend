/*!
 *@file GlobalVariable.h
 *@brief 鍏ㄥ眬鍙橀噺鎺ュ彛澶存枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */

#ifndef SYSYC_GLOBALVARIABLE_H
#define SYSYC_GLOBALVARIABLE_H

#include "Constant.h"
#include "Module.h"
#include "User.h"

/*! 全局变量类，包含常量*/
class GlobalVariable : public User {
private:
  bool is_const_;      //<! 鏄惁涓哄父閲?
  Constant *init_val_; //<! 鍒濆鍊?
  std::vector<int>
      _flatten_init_val; //<! 甯搁噺鏁扮粍鐨勫瓨鍌ㄦ暟缁?鎵佸钩鍖栵紝澶氱淮鏁扮粍闄嶄负涓€缁?
  /*!
   *@brief 鍏ㄥ眬鍙橀噺鐨勬瀯閫犲嚱鏁?
   *@param name 全局变量名称
   *@param m 所从属模块
   *@param ty 鍙橀噺鐨勭被鍨?
   *@param is_const 鏄惁涓哄父閲?
   *@param init 常量指针
   *@return 当前对象本身
   */
  GlobalVariable(std::string name, Module *m, Type *ty, bool is_const,
                 Constant *init = nullptr);

public:
  /*!
   *@brief 鍏ㄥ眬鍙橀噺鐨勫垱寤哄嚱鏁?
   *@param name 全局变量名称
   *@param m 所从属模块
   *@param ty 鍙橀噺鐨勭被鍨?
   *@param is_const 鏄惁涓哄父閲?
   *@param init 常量指针
   *@return 当前对象本身
   */
  static GlobalVariable *create(std::string name, Module *m, Type *ty,
                                bool is_const, Constant *init);

  /*!
   *@brief 鍏ㄥ眬鍙橀噺鐨勫垱寤哄嚱鏁?
   *@param name 全局变量名称
   *@param m 所从属模块
   *@param ty 鍙橀噺鐨勭被鍨?
   *@param is_const 鏄惁涓哄父閲?
   *@param init 常量指针
   *@return 当前对象本身
   */
  Constant *get_init() { return init_val_; }

  /*!
   *@brief 鍒ゆ柇鏄惁鏄父閲?
   *@return 常量判定结果
   *@note 是常量为1，不是为0
   */
  bool is_const() const { return is_const_; }

  /*!
   *@brief 扁平化数组的创建
   *@param i 常量化的扁平数组
   */
  void setFlattenInit(const std::vector<int> &i) { _flatten_init_val = i; }

  /*!
   *@brief 鑾峰彇鎵佸钩鍖栨暟缁?
   *@return 甯搁噺鎵佸钩鍖栨暟缁?
   */
  std::vector<int> getFlattenInit() const { return _flatten_init_val; }

  /*!
   *@brief 打印全局变量
   *@return 瀛楃涓?
   */
  std::string print();
};
#endif // SYSYC_GLOBALVARIABLE_H


