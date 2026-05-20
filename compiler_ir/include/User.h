/*!
 *@file User.h
 *@brief 用户类接口头文件
 *@version 1.0.0
 *@date 2022-10-04
 */

#ifndef SYSYC_USER_H
#define SYSYC_USER_H

#include "Value.h"
#include <vector>

/*! user类，中间IR的基础*/
class User : public Value {
private:
protected:
  std::vector<Value *> operands_; // operands of this value
  unsigned num_ops_;              // value值的个数

public:
  /*!
   *@brief User鐨勬瀯閫犲嚱鏁?
   *@param ty 类型
   *@param name User名称
   *@param num_ops value鐨勪綅缃?
   *@return 当前对象本身
   */
  User(Type *ty, const std::string &name = "", unsigned num_ops = 0);

  /*!
   *@brief Value鐨勬瀽鏋勫嚱鏁?
   */
  ~User() = default;

  /*!
   *@brief 鑾峰緱鍖呭惈value鎸囬拡鐨勬暟缁?
   *@return 返回User维护的Value数组
   */
  std::vector<Value *> &get_operands();

  /*!
   *@brief 鑾峰緱鏁扮粍涓殑绗琲涓獀alue鏁板€兼寚閽?
   *@return 鑾峰緱鏁扮粍涓殑绗琲涓獀alue鏁板€煎父閲忔寚閽?
   */
  Value *get_operand(unsigned i) const;

  /*!
   *@brief 璁剧疆鏁扮粍涓殑绗琲涓獀alue鏁板€兼寚閽?
   *@note
   *--------
   *璁剧疆鏁扮粍涓殑绗琲涓獀alue鏁板€煎父閲忔寚閽?
   */
  void set_operand(unsigned i, Value *v);

  /*!
   *@brief 娣诲姞鏂扮殑value鏁板€兼寚閽?
   *@param v value鏁板€兼寚閽?
   *@note
   *--------
   *&emsp; value数组尾插入一个value
   *&emsp; 为value添加一个use关系
   *&emsp; 计数加一
   */
  void add_operand(Value *v);

  /*!
   *@brief 获取User维护的operand数量
   *@return operand甯搁噺鏁板€?
   *@note
   *-------
   *判断
   */
  unsigned get_num_operand() const;

  /*!
   *@brief 娣诲姞鏂扮殑value鏁板€兼寚閽?
   *@param v value鏁板€兼寚閽?
   *@note
   *--------
   *
   */
  void remove_use_of_ops();

  /*!
   *@brief 娣诲姞鏂扮殑value鏁板€兼寚閽?
   *@param index1 索引1
   *@param index2 索引2
   *@note
   */
  void remove_operands(int index1, int index2);
};

#endif // SYSYC_USER_H


