/*!
 *@file User.h
 *@brief 鐢ㄦ埛绫绘帴鍙ｅ畾涔夋枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */

#include "User.h"
#include <cassert>

/*!
 *@brief User鐨勬瀯閫犲嚱鏁?
 *@param ty 类型
 *@param name User名称
 *@param num_ops value鐨勪綅缃?
 *@return 当前对象本身
 *@note
 *---------
 *初始化operands数组全为nullptr
 */
User::User(Type *ty, const std::string &name, unsigned num_ops)
    : Value(ty, name), num_ops_(num_ops) {
  operands_.resize(num_ops_, nullptr);
}

/*!
 *@brief 鑾峰緱鍖呭惈value鎸囬拡鐨勬暟缁?
 *@return 返回User维护的Value数组
 */
std::vector<Value *> &User::get_operands() { return operands_; }

/*!
 *@brief 鑾峰緱鏁扮粍涓殑绗琲涓獀alue鏁板€兼寚閽?
 *@return 鑾峰緱鏁扮粍涓殑绗琲涓獀alue鏁板€煎父閲忔寚閽?
 */
Value *User::get_operand(unsigned i) const { return operands_[i]; }

/*!
 *@brief 璁剧疆鏁扮粍涓殑绗琲涓獀alue鏁板€兼寚閽?
 *@note
 *--------
 *璁剧疆鏁扮粍涓殑绗琲涓獀alue鏁板€煎父閲忔寚閽?
 *设置界限检查，查看索引i是否超限
 *--------
 *&emsp; value数组尾插入一个value
 *&emsp; 为value添加一个use关系
 *&emsp; 计数加一
 */
void User::set_operand(unsigned i, Value *v) {
  assert(i < num_ops_ && "set_operand out of index");
  // assert(operands_[i] == nullptr && "ith operand is not null");
  operands_[i] = v;
  v->add_use(this, i);
}

/*!
 *@brief 娣诲姞鏂扮殑value鏁板€兼寚閽?
 *@param v value鏁板€兼寚閽?
 *@note
 *--------
 *&emsp; value数组尾插入一个value
 *&emsp; 为value添加一个use关系
 *&emsp; 计数加一
 */
void User::add_operand(Value *v) {
  operands_.push_back(v);
  v->add_use(this, num_ops_);
  num_ops_++;
}

/*!
 *@brief 获取User维护的operand数量
 *@return operand甯搁噺鏁板€?
 *@note
 *-------
 *判断
 */
unsigned User::get_num_operand() const { return num_ops_; }

/*!
 *@brief 娣诲姞鏂扮殑value鏁板€兼寚閽?
 *@param v value鏁板€兼寚閽?
 *@note
 *--------
 *閬嶅巻operands閾捐〃锛屽浜庢瘡涓獀alue閲忕殑use_list杩涜妫€鏌?
 *在operands表中删除对于本对象的使用
 */
void User::remove_use_of_ops() {
  for (auto op : operands_) {
    op->remove_use(this);
  }
}

/*!
 *@brief 删除指定范围的operands
 *@param index1 索引1
 *@param index2 索引2
 *@note
 *--------
 *閬嶅巻operands琛ㄧ储寮曡寖鍥达紝鍒犻櫎瀵逛簬鏈瑄ser鐨勪娇鐢?
 *删除本表的相关operands
 *修改operands_size
 */
void User::remove_operands(int index1, int index2) {
  for (int i = index1; i <= index2; i++) {
    operands_[i]->remove_use(this);
  }
  operands_.erase(operands_.begin() + index1, operands_.begin() + index2 + 1);
  num_ops_ = operands_.size();
}

