/*!
 *@file Value.h
 *@brief Value类接口头文件
 *@version 1.0.0
 *@date 2022-10-04
 */

#ifndef SYSYC_VALUE_H
#define SYSYC_VALUE_H

#include <iostream>
#include <list>
#include <string>

class Type;
class Value;

/*! use结构体，作为中间IR的基础*/
struct Use {
  Value *val_;      // 使用value的value
  unsigned arg_no_; // the no. of operand, e.g., func(a, b), a is 0, b is 1
  Use(Value *val, unsigned no) : val_(val), arg_no_(no) {} // 鏋勯€犲嚱鏁?

  /*!
   *@brief 判定两个use是否相等
   *@param 待比较的use-1
   *@param 待比较的use-2
   *@return 比对结果
   */
  friend bool operator==(const Use &lhs, const Use &rhs) { //
    return lhs.val_ == rhs.val_ && lhs.arg_no_ == rhs.arg_no_;
  }
};

/*! value类，作为中间IR的基础*/
class Value {
private:
protected:
  Type *type_;
  std::list<Use> use_list_; // 使用value的value list
  std::string name_;        // value名称

public:
  /*!
   *@brief Value鐨勬瀯閫犲嚱鏁?
   *@param ty 类型
   *@param name value名称
   *@return 当前对象本身
   */
  explicit Value(Type *ty, const std::string &name = "");
  /*!
   *@brief Value鐨勬瀽鏋勫嚱鏁?
   */
  ~Value() = default;

  /*!
   *@brief 鑾峰彇value鐨勭被鍨?
   *@return value 类型常量指针
   */
  Type *get_type() const { return type_; }

  /*!
   *@brief 获取使用该value的use list
   *@return 杩斿洖use-list鐨勫紩鐢?
   */
  std::list<Use> &get_use_list() { return use_list_; }

  /*!
   *@brief 添加use
   *@param val 使用该value的value
   *@param arg_no 鍦ㄦ寚浠や腑鐨勯『搴?
   */
  void add_use(Value *val, unsigned arg_no = 0);

  /*!
   *@brief 对于value设置名称
   *@param name value名称
   *@return 鍚嶇О璁剧疆鐨勫竷灏旂粨鏋?
   *@note
   *---------
   *名字为空即设置新名字，设置后不再进行修改
   */
  bool set_name(std::string name) {
    if (name_ == "") {
      name_ = name;
      return true;
    }
    return false;
  }

  /*!
   *@brief 鑾峰彇value鐨勫悕绉?
   *@return value瀛楃涓插父閲?
   */
  std::string get_name() const;

  /*!
   *@brief 替换所有对于旧value的引用，改为新的
   *@param new_val value鍨嬫寚閽?
   *@note
   *--------
   *鏀寔瀵逛簬鎵€鏈夌殑value鐨勪慨鏀癸紝鍖呮嫭鍩烘湰鍧?
   */
  void replace_all_use_with(Value *new_val);

  /*!
   *@brief 替换所有对于旧value的引用，改为新的
   *@param val value鍨嬫寚閽?
   *@note
   *----------
   *鏀寔瀵逛簬鎸囦护绾у埆value鐨勬浛鎹?
   */
  void remove_use(Value *val);

  /*!
   *@brief value鐨勬墦鍗?
   *@return 默认为空
   *@note
   *--------
   *后期根据不同类型的value进行修改，作为一个虚函数出现
   */
  virtual std::string print() { return ""; }
};

#endif // SYSYC_VALUE_H


