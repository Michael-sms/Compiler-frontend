/*!
 *@file Value.cpp
 *@brief Value绫绘帴鍙ｅ畾涔夋枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */

#include <cassert>

#include "BasicBlock.h"
#include "Type.h"
#include "User.h"
#include "Value.h"
/*!
 *@brief Value鐨勬瀯閫犲嚱鏁?
 *@param ty 类型
 *@param name value名称
 *@return 当前对象本身
 */
Value::Value(Type *ty, const std::string &name) : type_(ty), name_(name) {}

/*!
 *@brief 添加use
 *@param val 使用该value的value
 *@param arg_no 鍦ㄦ寚浠や腑鐨勯『搴?
 *@note
 *---------
 *在use链中添加
 */
void Value::add_use(Value *val, unsigned arg_no) {
  use_list_.push_back(Use(val, arg_no));
}
/*!
 *@brief 鑾峰彇value鐨勫悕绉?
 *@return value瀛楃涓插父閲?
 */
std::string Value::get_name() const { return name_; }
/*!
 *@brief 替换所有对于旧value的引用，改为新的
 *@param new_val value鍨嬫寚閽?
 *@note
 *--------
 *鏀寔瀵逛簬鎵€鏈夌殑value鐨勪慨鏀癸紝鍖呮嫭鍩烘湰鍧?
 *&emsp; 首先遍历所属的use_list，修改其他value中对于当前value的引用为新value
 *&emsp; 转换value类型为basicblock，修改成功即为对基本块间的类型调用修改，
 *&emsp; 渚濇淇敼鍓嶇疆鍚庣疆鐨勯摼琛ㄤ腑瀵逛簬璇ュ熀鏈潡鐨勫紩鐢?
 */
void Value::replace_all_use_with(Value *new_val) {
  for (auto use : use_list_) {
    auto val = dynamic_cast<User *>(use.val_);
    assert(val && "new_val is not a user");
    val->set_operand(use.arg_no_, new_val);
  }
  auto val = dynamic_cast<BasicBlock *>(this);
  if (val) {
    auto new_bb = dynamic_cast<BasicBlock *>(new_val);
    for (BasicBlock *pre_bb : val->get_pre_basic_blocks()) {
      pre_bb->remove_succ_basic_block(val);
      pre_bb->add_succ_basic_block(new_bb);
      new_bb->add_pre_basic_block(pre_bb);
    }
    for (BasicBlock *suc_bb : val->get_succ_basic_blocks()) {
      suc_bb->remove_pre_basic_block(val);
      suc_bb->add_pre_basic_block(new_bb);
      new_bb->add_succ_basic_block(suc_bb);
    }
  }
}

/*!
 *@brief 替换所有对于旧value的引用，改为新的
 *@param val value鍨嬫寚閽?
 *@note
 *----------
 *璁剧疆lambda鍑芥暟锛屽垽瀹歷al鐨勫尮閰?
 *鍖归厤鎴愬姛鍒欒繘琛寁alue鐨勭浉鍏冲垹闄?
 */
void Value::remove_use(Value *val) {
  auto is_val = [val](const Use &use) { return use.val_ == val; };
  use_list_.remove_if(is_val);
}
