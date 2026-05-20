/*!
 *@file BasicBlock.h
 *@brief 基本块接口头文件
 *@version 1.0.0
 *@date 2022-10-04
 */

#ifndef SYSYC_BASICBLOCK_H
#define SYSYC_BASICBLOCK_H

#include "Function.h"
#include "Instruction.h"
#include "Module.h"
#include "Value.h"

#include <list>
#include <set>
#include <string>
#include <vector>

class Function;
class Instruction;
class Module;

/*!
  @brief 鍩烘湰鍧楄妭鐐?
*/
class BasicBlock : public Value {
private:
  std::list<BasicBlock *> pre_bbs_;     //!<  pre basic blocks
  std::list<BasicBlock *> succ_bbs_;    //!<  subsequence basic blocks
  std::list<Instruction *> instr_list_; //!<  instruction in basic block
  Function *parent_;                    //!<  belong to which function
  bool _fake;                           //!<  is fake basicblock

public:
  /*!
   *@brief 鍩烘湰鍧楃殑鏋勯€犲嚱鏁?
   *@param m 所从属模块
   *@param name 鍩烘湰鍧楀悕绉?
   *@param parent 鎵€浠庡睘鐨勫嚱鏁?
   *@param fake 是否是假基本块，即基本块是否为空
   *@return 当前对象本身
   */
  explicit BasicBlock(Module *m, const std::string &name, Function *parent,
                      bool fake);

  /*!
   *@brief 基本块的创建函数
   *@param m 所从属模块
   *@param name 鍩烘湰鍧楀悕绉?
   *@param parent 鎵€浠庡睘鐨勫嚱鏁?
   *@param fake 是否是假基本块，即基本块是否为空，默认为
   *@return 创建的基本块对象指针
   *@note
   *----------
   *默认基本块名称为label_()number
   */
  static BasicBlock *create(Module *m, const std::string &name,
                            Function *parent, bool fake = false) {
    auto prefix = name.empty() ? "" : "label_";
    return new BasicBlock(m, prefix + name, parent, fake);
  }

  /*!
   *@brief 杩斿洖鍩烘湰鍧楃殑鎵€灞炲嚱鏁?
   *@return 鎵€浠庡睘鐨勫嚱鏁板璞℃寚閽?
   *@note
   *----------
   *return parent, or null if none.
   */
  Function *get_parent() { return parent_; }

  /*!
   *@brief 杩斿洖鍩烘湰鍧楃殑鎵€灞炴ā鍧?
   *@return 浠庡睘鐨勬ā鍧楀璞℃寚閽?
   *@note
   *----------
   *return parent, or null if none.
   */
  Module *get_module();

  /*!
   *@brief 返回基本块的前置基本块链
   *@return 前置基本块链
   *@note
   *----------
   *return pre_bbs_, or null if none.
   */
  std::list<BasicBlock *> &get_pre_basic_blocks() { return pre_bbs_; }

  /*!
   *@brief 返回基本块的前置基本块链
   *@return 后置基本块链
   *@note
   *----------
   *return succ_bbs_, or null if none.
   */
  std::list<BasicBlock *> &get_succ_basic_blocks() { return succ_bbs_; }

  /*!
   *@brief 鍚戝墠缃熀鏈潡閾句腑鎻掑叆鏂扮殑鍩烘湰鍧?
   *@param bb 寰呮彃鍏ョ殑鍩烘湰鍧楁寚閽?
   *@note
   *----------
   */
  void add_pre_basic_block(BasicBlock *bb) { pre_bbs_.push_back(bb); }

  /*!
   *@brief 鍚戝悗缃熀鏈潡閾句腑鎻掑叆鏂扮殑鍩烘湰鍧?
   *@param bb 寰呮彃鍏ョ殑鍩烘湰鍧楁寚閽?
   *@note
   *----------
   *return parent, or null if none.
   */
  void add_succ_basic_block(BasicBlock *bb) { succ_bbs_.push_back(bb); }

  /*!
   *@brief 更新基本块的前置基本块链
   *@param bb_list 鍩烘湰鍧楅摼鐨勯泦鍚?
   *@note
   *----------
   *&emsp; 娓呯┖鍘熷厛鐨勫墠缃熀鏈潡閾?
   *&emsp; 鐢ㄦ柊鐨勫熀鏈潡閾捐繘琛屽～鍏?
   */
  void set_pre_bb(const std::set<BasicBlock *> &bb_list) {
    pre_bbs_.clear();
    pre_bbs_.insert(pre_bbs_.begin(), bb_list.begin(), bb_list.end());
  }

  /*!
   *@brief 更新基本块的后置基本块链
   *@param bb_list 鍩烘湰鍧楅摼鐨勯泦鍚?
   *@note
   *----------
   *&emsp; 娓呯┖鍘熷厛鐨勫悗缃熀鏈潡閾?
   *&emsp; 鐢ㄦ柊鐨勫熀鏈潡閾捐繘琛屽～鍏?
   */
  void set_succ_bb(const std::set<BasicBlock *> &bb_list) {
    succ_bbs_.clear();
    succ_bbs_.insert(succ_bbs_.begin(), bb_list.begin(), bb_list.end());
  }

  /*!
   *@brief 替换指定的基本块
   *@param oldBB 被替换的旧基本块指针
   *@param newBB 鐢ㄤ簬鏇挎崲鐨勬柊鍩烘湰鍧楁寚閽?
   *@note
   *----------
   *####1 替换前置基本块链中的旧基本块
   *&emsp; **for** 遍历前置链表
   *&emsp;&emsp; 如果匹配到指定的基本块指针，进行替换
   *####2 替换后置基本块链中的就基本块
   *&emsp; **for** 遍历后置链表
   *&emsp;&emsp; 如果匹配到指定的基本块指针，进行替换
   */
  void replace_basic_block(BasicBlock *oldBB, BasicBlock *newBB) {
    for (auto it = pre_bbs_.begin(); it != pre_bbs_.end(); ++it) {
      if (*it == oldBB) {
        *it = newBB;
      }
    }
    for (auto it = succ_bbs_.begin(); it != succ_bbs_.end(); ++it) {
      if (*it == oldBB) {
        *it = newBB;
      }
    }
  }

  /*!
   *@brief 删除前置基本块链的指定基本块
   *@param 寰呭垹闄ょ殑鍩烘湰鍧楁寚閽?
   *@note
   *----------
   *pre list remove bb
   */
  void remove_pre_basic_block(BasicBlock *bb) { pre_bbs_.remove(bb); }

  /*!
   *@brief 删除后置基本块链的指定基本块
   *@param 寰呭垹闄ょ殑鍩烘湰鍧楁寚閽?
   *@note
   *----------
   *succ list remove bb
   */
  void remove_succ_basic_block(BasicBlock *bb) { succ_bbs_.remove(bb); }

  /*!
   *@brief 鑾峰彇鍩烘湰鍧楀唴鐨勭粓缁撴寚浠?
   *@return 终结指令常量指针
   *@note
   *----------
   *Returns the terminator instruction if the block is well formed
   *if the block is not well formed then null
   *杩斿洖鍩烘湰鍧楃殑鐨勪腑鐨勭粓缁撴寚浠ゅ父閲忔寚閽?
   */
  const Instruction *get_terminator() const;

  /*!
   *@brief 鑾峰彇鍩烘湰鍧楀唴鐨勭粓缁撴寚浠?
   *@return 终结指令常量指针
   *@note
   *----------
   *Returns the terminator instruction if the block is well formed
   *if the block is not well formed then null
   *杩斿洖鍩烘湰鍧楃殑鐨勪腑鐨勭粓缁撴寚浠ゅ父閲忔寚閽?
   */
  Instruction *get_terminator() {
    return const_cast<Instruction *>(
        static_cast<const BasicBlock *>(this)->get_terminator());
  }

  /*!
   *@brief 鍚戝熀鏈潡涓坊鍔犳寚浠?
   *@param instr 待添加的指令指针
   *@note
   *----------
   *鍦ㄥ熀鏈潡鐨勫熬閮ㄦ坊鍔犳寚浠?
   */
  void add_instruction(Instruction *instr);

  /*!
   *@brief 鍚戝熀鏈潡涓坊鍔犳寚浠?
   *@param instr 待添加的指令指针
   *@note
   *----------
   *鍦ㄥ熀鏈潡鐨勫ご閮ㄦ坊鍔犳寚浠?
   */
  void add_instr_begin(Instruction *instr);

  /*!
   *@brief 在phi指令之后添加指令
   *@param 待添加的指令指针
   *@note
   *----------
   *鍚憄hi鎸囦护鍚庢坊鍔犳寚浠?
   */
  void add_instr_after_phi(Instruction *instr);

  /*!
   *@brief 鍒犻櫎鍩烘湰鍧椾腑鐨勬煇涓寚浠?
   *@param 待删除的指令指针
   *@note
   *----------
   *&emsp; 鍒犻櫎缁存姢鐨勬寚浠ら摼琛ㄤ腑鐨勬寚浠?
   *&emsp; 获取删除的指令的前后指令
   *&emsp; 淇鎸囦护鐨勯摼鎺ュ叧绯伙細锛?
   *&emsp; **if** 濡傛灉鍓嶇疆鎸囦护涓嶄负绌?
   *&emsp;&emsp; 涓哄墠缃寚浠よ缃柊鐨勫悗缃寚浠?
   *&emsp; **if** 濡傛灉鍚庣疆鎸囦护涓嶄负绌?
   *&emsp;&emsp; 涓哄悗缃寚浠よ缃柊鐨勫墠缃寚浠?
   *&emsp; 琚垹闄ょ殑鎸囦护杩涜鐩稿叧use鐨勫垹闄?
   */
  void delete_instr(Instruction *instr);

  /*!
   *@brief 判断基本块维护的指令链表是否为空
   *@return 指令链表是否为空结果
   *@note
   *----------
   *为空1，不为空0
   */
  bool empty() { return instr_list_.empty(); }

  /*!
   *@brief 鍒ゆ柇鏄惁鏄亣鍩烘湰鍧?
   *@return 鍩烘湰鍧楀垽瀹氱粨鏋?
   *@note
   *----------
   *为假1，不为假0
   */
  bool is_fake_block() { return _fake; }

  /*!
   *@brief 指针数量
   *@return 返回基本块维护指令链表的指针数量
   *@note
   *----------
   */
  int get_num_of_instr() { return instr_list_.size(); }

  /*!
   *@brief 获取基本块的指针链表
   *@return 指令指针链表引用
   *@note
   *----------
   */
  std::list<Instruction *> &get_instructions() { return instr_list_; }

  /*!
   *@brief 灏嗗熀鏈潡浠庝粠灞炵殑鍑芥暟涓垹闄?
   *@note
   *----------
   *获取基本块的所属函数并删除函数所保存的关于该基本块的指针
   */
  void erase_from_parent();

  /*!
   *@brief 鎵撳嵃鍩烘湰鍧?
   *@note
   *----------
   *return parent, or null if none.
   */
  virtual std::string print() override;
};

#endif


