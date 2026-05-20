#ifndef SYSYC_IRBUILDER_H
#define SYSYC_IRBUILDER_H

#include "BasicBlock.h"
#include "Instruction.h"
#include "Value.h"

class IRBuilder {
private:
  BasicBlock *BB_;
  Module *m_;
  Function *curfunc;

public:
  /*!
   *@brief irbuilder鐨勬瀯閫犲嚱鏁?
   *@param bb 鍩烘湰鍧?
   *@param m 模块
   *@return 当前对象本身
   */
  IRBuilder(BasicBlock *bb, Module *m) : BB_(bb), m_(m){};
  /*!
   *@brief irbuilder鐨勬瀽鏋勫嚱鏁?
   */
  ~IRBuilder() = default;
  /*!
   *@brief 设置函数指针
   *@return 函数指针
   */
  void set_curFunc(Function *f) { this->curfunc = f; };
  /*!
   *@brief 获取函数指针
   *@return 函数指针
   */
  Function *get_curFunc() { return this->curfunc; }
  /*!
   *@brief 获取模块指针
   *@return 模块指针
   */
  Module *get_module() { return m_; }
  /*!
   *@brief 鑾峰彇瑕佹彃鍏ョ殑鍩烘湰鍧?
   *@return 鍩烘湰鍧楁寚閽?
   */
  BasicBlock *get_insert_block() { return this->BB_; }
  /*!
   *@brief 鏇存柊瑕佽繘琛屼慨鏀圭殑鍩烘湰鍧?
   *@param bb 鍩烘湰鍧楁寚閽?
   *@note 鍦ㄦ煇涓熀鏈潡涓彃鍏ユ寚浠?
   */
  void set_insert_point(BasicBlock *bb) {
    this->BB_ = bb;
  } //鍦ㄦ煇涓熀鏈潡涓彃鍏ユ寚浠?
  /*!
   *@brief 创建加法指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹀姞娉曟寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_iadd(Value *lhs, Value *rhs) {
    return BinaryInst::create_add(lhs, rhs, this->BB_, m_);
  } //创建加法指令（以及其他算术指令）
  /*!
   *@brief 创建减法指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹀噺娉曟寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_isub(Value *lhs, Value *rhs) {
    return BinaryInst::create_sub(lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建乘法指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗︿箻娉曟寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_imul(Value *lhs, Value *rhs) {
    return BinaryInst::create_mul(lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建除法指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹂櫎娉曟寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_isdiv(Value *lhs, Value *rhs) {
    return BinaryInst::create_sdiv(lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 鍒涘缓妯¤繍绠楁寚浠?
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 二元操作符模运算指令指针
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_irem(Value *lhs, Value *rhs) {
    return BinaryInst::create_mod(lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 鍒涘缓涓庤繍绠楁寚浠?
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹂櫎娉曟寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_iand(Value *lhs, Value *rhs) {
    return BinaryInst::create_sdiv(lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 鍒涘缓鎴栬繍绠楁寚浠?
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹂櫎娉曟寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BinaryInst *create_ior(Value *lhs, Value *rhs) {
    return BinaryInst::create_sdiv(lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建比较相等指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗︾浉绛夋寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */

  CmpInst *create_icmp_eq(Value *lhs, Value *rhs) {
    return CmpInst::create_cmp(CmpInst::EQ, lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建比较不等指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗︿笉绛夋寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  CmpInst *create_icmp_ne(Value *lhs, Value *rhs) {
    return CmpInst::create_cmp(CmpInst::NE, lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建比较大于等于指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹀ぇ浜庣瓑浜庢寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  CmpInst *create_icmp_gt(Value *lhs, Value *rhs) {
    return CmpInst::create_cmp(CmpInst::GT, lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建比较大于指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗﹀ぇ浜庢寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */

  CmpInst *create_icmp_ge(Value *lhs, Value *rhs) {
    return CmpInst::create_cmp(CmpInst::GE, lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建比较小于指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗︽瘮杈冨皬浜庢寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  CmpInst *create_icmp_lt(Value *lhs, Value *rhs) {
    return CmpInst::create_cmp(CmpInst::LT, lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建比较小于等于指令
   *@param lhs 宸﹀€兼寚閽?
   *@param rhs 鍙冲€兼寚閽?
   *@return 浜屽厓鎿嶄綔绗︽瘮杈冨皬浜庢寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  CmpInst *create_icmp_le(Value *lhs, Value *rhs) {
    return CmpInst::create_cmp(CmpInst::LE, lhs, rhs, this->BB_, m_);
  }
  /*!
   *@brief 创建调用指令
   *@param func 函数指针
   *@param args 函数参数value指针数组
   *@return 调用指令指针
   *@note 调用指针类的创建函数
   */
  CallInst *create_call(Value *func, std::vector<Value *> args) {
    assert(dynamic_cast<Function *>(func) && "func must be Function * type");
    return CallInst::create(static_cast<Function *>(func), args, this->BB_);
  }
  /*!
   *@brief 鍒涘缓鏃犳潯浠惰烦杞寚浠?
   *@param if_true 鍩烘湰鍧楁寚閽?
   *@return 鏃犳潯浠惰烦杞寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BranchInst *create_br(BasicBlock *if_true) {
    return BranchInst::create_br(if_true, this->BB_);
  }

  /*!
   *@brief 鍒涘缓鏈夋潯浠惰烦杞寚浠?
   *@param cond 条件
   *@param if_ture 条件为真的基本块指针
   *@param if_false 条件为假的基本块指针
   *@return 鏈夋潯浠惰烦杞寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  BranchInst *create_cond_br(Value *cond, BasicBlock *if_true,
                             BasicBlock *if_false) {
    return BranchInst::create_cond_br(cond, if_true, if_false, this->BB_);
  }
  /*!
   *@brief 创建返回指令
   *@param val 杩斿洖鍊?
   *@return 返回指令指针
   *@note 调用指针类的创建函数
   */
  ReturnInst *create_ret(Value *val) {
    return ReturnInst::create_ret(val, this->BB_);
  }
  /*!
   *@brief 鍒涘缓绌哄€艰繑鍥炴寚浠?
   *@return 鏃犲€艰繑鍥炴寚浠ゆ寚閽?
   *@note 调用指针类的创建函数
   */
  ReturnInst *create_void_ret() {
    return ReturnInst::create_void_ret(this->BB_);
  }
  /*!
   *@brief 创建指针指令
   *@param ptr 指令指针
   *@param rhs value指针数组
   *@return 元素指针指令指针
   *@note 调用指针类的创建函数
   */
  GetElementPtrInst *create_gep(Value *ptr, std::vector<Value *> idxs) {
    return GetElementPtrInst::create_gep(ptr, idxs, this->BB_);
  }
  /*!
   *@brief 创建存储指令
   *@param val value鍊兼寚閽?
   *@param ptr 指针，指向存值的地址
   *@return 存储指令指针
   *@note 调用指针类的创建函数
   */
  StoreInst *create_store(Value *val, Value *ptr) {
    return StoreInst::create_store(val, ptr, this->BB_);
  }

  /*!
   *@brief 创建加载指令
   *@param ty 类型
   *@param ptr 指针，指向存值的地址
   *@return 加载指令指针
   *@note 调用指针类的创建函数
   */
  LoadInst *create_load(Type *ty, Value *ptr) {
    return LoadInst::create_load(ty, ptr, this->BB_);
  }

  /*!
   *@brief 创建加载指令
   *@param ptr value类型指针
   *@return 加载指令指针
   *@note 调用指针类的创建函数
   */
  LoadInst *create_load(Value *ptr) {
    assert(ptr->get_type()->is_pointer_type() && "ptr must be pointer type");
    return LoadInst::create_load(ptr->get_type()->get_pointer_element_type(),
                                 ptr, this->BB_);
  }
  /*!
   *@brief 创建申请内容指令
   *@param ty 类型
   *@return 申请内容指令指针
   *@note 调用指针类的创建函数
   */
  AllocaInst *create_alloca(Type *ty) {
    return AllocaInst::create_alloca(ty, this->BB_);
  }

  /*!
   *@brief 创建扩展指令
   *@param value value鍊兼寚閽?
   *@param ty 鏂扮被鍨?
   *@return 扩展指令指针
   *@note 调用指针类的创建函数
   */
  ZextInst *create_zext(Value *val, Type *ty) {
    return ZextInst::create_zext(val, ty, this->BB_);
  }
};

#endif // SYSYC_IRBUILDER_H


