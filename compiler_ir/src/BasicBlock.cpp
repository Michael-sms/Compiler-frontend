/*!
 *@file BasicBlock.cpp
 *@brief 鍩烘湰鍧楁帴鍙ｅ畾涔夋枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */
#include "BasicBlock.h"
#include "Function.h"
#include "IRprinter.h"
#include "Module.h"
#include <cassert>

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
 *鏋勫缓榛樿涓虹湡鍩烘湰鍧楋紝鍚嶇О涓虹┖锛岀被鍨嬩负鍩烘湰鍧?
 *&emsp; 濡傛灉parent涓虹┖锛屽垱寤哄け璐ワ紝寮傚父閫€鍑?
 *&emsp; 向所属的函数中添加基本块指针
 */
BasicBlock::BasicBlock(Module *m, const std::string &name = "",
                       Function *parent = nullptr, bool fake = false)
    : Value(Type::get_label_type(m), name), parent_(parent), _fake(fake) {
  // assert(parent && "currently parent should not be nullptr");
  parent_->add_basic_block(this);
}

/*!
 *@brief 鍚戝熀鏈潡涓坊鍔犳寚浠?
 *@param 待添加的指令指针
 *@note
 *----------
 *鍦ㄥ熀鏈潡鐨勫熬閮ㄦ坊鍔犳寚浠?
 *&emsp; 设置待添加的指令后缀为空
 *&emsp; **if** 如果指令链表为空
 *&emsp;&emsp; 璁剧疆鎸囦护涓哄悗缁т负绌?
 *&emsp;&emsp; 涓嶄负绌猴紝鍒欒幏鍙栨寚浠ら摼琛ㄧ殑鏈€鍚庝竴涓寚浠?
 *&emsp;&emsp; 璁剧疆鎻掑叆鎸囦护鐨勫墠缁т负鏈€鍚庝竴涓寚浠?
 *&emsp;&emsp; 鏈€鍚庝竴涓寚浠ゅ悗缁т负寰呮彃鍏ユ寚浠?
 *&emsp; 尾部插入指令
 */
void BasicBlock::add_instruction(Instruction *instr) {
  instr->setSuccInst(nullptr);
  if (instr_list_.empty()) {
    instr->setSuccInst(nullptr);
  } else {
    Instruction *last_inst = instr_list_.back();
    instr->setPrevInst(last_inst);
    last_inst->setSuccInst(instr);
  }
  instr_list_.push_back(instr);
}

  /*!
   *@brief 杩斿洖鍩烘湰鍧楃殑鎵€灞炴ā鍧?
   *@return 浠庡睘鐨勬ā鍧楀璞℃寚閽?
   *@note
   *----------
   *return parent, or null if none.
   */
  Module *BasicBlock::get_module() { return get_parent()->get_parent(); }

  /*!
   *@brief 灏嗗熀鏈潡浠庝粠灞炵殑鍑芥暟涓垹闄?
   *@note
   *----------
   *获取基本块的所属函数并删除函数所保存的关于该基本块的指针
   */
  void BasicBlock::erase_from_parent() { this->get_parent()->remove(this); }

/*!
 *@brief 鍚戝熀鏈潡涓坊鍔犳寚浠?
 *@param 待添加的指令指针
 *@note
 *----------
 *鍦ㄥ熀鏈潡鐨勫ご閮ㄦ坊鍔犳寚浠?
 *&emsp; 设置待添加的指令前继为空
 *&emsp; **if** 如果指令链表为空
 *&emsp;&emsp; 璁剧疆鎸囦护涓哄悗缁т负绌?
 *&emsp;&emsp; 涓嶄负绌猴紝鍒欒幏鍙栨寚浠ら摼琛ㄧ殑绗竴涓寚浠?
 *&emsp;&emsp; 璁剧疆鎻掑叆鎸囦护鐨勫悗缁т负绗竴涓寚浠?
 *&emsp;&emsp; 绗竴涓寚浠ゅ墠缁т负寰呮彃鍏ユ寚浠?
 *&emsp; 头部插入指令
 */
void BasicBlock::add_instr_begin(Instruction *instr) {
  instr->setPrevInst(nullptr);
  if (instr_list_.empty()) {
    instr->setSuccInst(nullptr);
  } else {
    Instruction *first_inst = instr_list_.front();
    instr->setSuccInst(first_inst);
    first_inst->setPrevInst(instr);
  }
  instr_list_.push_front(instr);
}

/*!
 *@brief 在phi指令之后添加指令
 *@param 待添加的指令指针
 *@note
 *----------
 *鍚憄hi鎸囦护鍚庢坊鍔犳寚浠?
 *&emsp; 设置指令的从属基本块
 *&emsp; 鑾峰彇鎸囦护閾捐〃鐨勫紑濮?
 *&emsp;&emsp; **for** 寰幆锛岄亶鍘嗚幏寰梡hi鎸囦护鐐?
 *&emsp; **if** 濡傛灉涓嶆槸閾捐〃澶达紝phi鑺傜偣鍓嶇户涓鸿妭鐐瑰墠缁?
 *&emsp; **if** 濡傛灉涓嶆槸閾捐〃灏撅紝phi鑺傜偣鍚庣户璁剧疆涓鸿妭鐐规湰韬?
 *&emsp; 修正插入节点的连接关系，前继和后继的修改
 */
void BasicBlock::add_instr_after_phi(Instruction *instr) {
  instr->set_parent(this);
  auto it = instr_list_.begin();
  //閬嶅巻鑾峰緱phi鎸囦护鐐?
  for (; it != instr_list_.end(); ++it) {
    if (!(*it)->is_phi()) {
      break;
    }
  }
  //鑾峰彇鎻掑叆鐐?
  Instruction *front = nullptr, *back = nullptr;
  if (it != instr_list_.begin()) {
    front = *(--it);
    ++it;
  }
  if (it != instr_list_.end()) {
    back = *it;
  }
  if (front != nullptr) {
    front->setSuccInst(instr);
  }
  if (back != nullptr) {
    back->setPrevInst(instr);
  }
  // 璁剧疆寰呮彃鍏ヨ妭鐐圭殑鍓嶅悗缁?
  instr->setPrevInst(front);
  instr->setSuccInst(back);
  instr_list_.insert(it, instr);
}

/*!
 *@brief 鍒犻櫎鍩烘湰鍧椾腑鐨勬煇涓寚浠?
 *@param 待删除的指令指针
 *@note
 *----------
 *&emsp; 鍒犻櫎缁存姢鐨勬寚浠ら摼琛ㄤ腑鐨勬寚浠?
 *&emsp; 获取删除的指令的前后指令
 *&emsp; 修正指令的链接关系：
 *&emsp; **if** 濡傛灉鍓嶇疆鎸囦护涓嶄负绌?
 *&emsp;&emsp; 涓哄墠缃寚浠よ缃柊鐨勫悗缃寚浠?
 *&emsp; **if** 濡傛灉鍚庣疆鎸囦护涓嶄负绌?
 *&emsp;&emsp; 涓哄悗缃寚浠よ缃柊鐨勫墠缃寚浠?
 *&emsp; 琚垹闄ょ殑鎸囦护杩涜鐩稿叧use鐨勫垹闄?
 */
void BasicBlock::delete_instr(Instruction *instr) {
  instr_list_.remove(instr);
  Instruction *prev = instr->getPrevInst();
  Instruction *succ = instr->getSuccInst();
  //淇鎸囦护鐨勯摼鎺ュ叧绯?
  if (prev != nullptr) {
    prev->setSuccInst(succ);
  }
  if (succ != nullptr) {
    succ->setPrevInst(prev);
  }
  //琚垹闄ょ殑鎸囦护杩涜鐩稿叧use鐨勫垹闄?
  instr->remove_use_of_ops();
}

/*!
 *@brief 鑾峰彇鍩烘湰鍧楀唴鐨勭粓缁撴寚浠?
 *@return 终结指令常量指针
 *@note
 *----------
 *Returns the terminator instruction if the block is well formed
 *if the block is not well formed then null
 *杩斿洖鍩烘湰鍧楃殑鐨勪腑鐨勭粓缁撴寚浠ゅ父閲忔寚閽?
 *---------
 *&emsp; 如果为空，返回null
 *&emsp; 获取终结指令的类型，并进行对应的判定
 *&emsp;&emsp; 杩斿洖鎸囦护锛岃繑鍥炴寚浠よ〃鐨勬渶鍚庝竴涓寚浠?
 *&emsp;&emsp; 鏉′欢鎸囦护锛岃繑鍥炴寚浠よ〃鐨勬渶鍚庝竴涓寚浠?
 *&emsp;&emsp; 其他指令，判定为空，返回
 */
const Instruction *BasicBlock::get_terminator() const {
  if (instr_list_.empty()) {
    return nullptr;
  }
  switch (instr_list_.back()->get_instr_type()) {
  case Instruction::ret:
    return instr_list_.back();
    break;

  case Instruction::br:
    return instr_list_.back();
    break;

  default:
    return nullptr;
    break;
  }
}

/*!
 *@brief 鎵撳嵃鍩烘湰鍧?
 *@return 寰呮墦鍗扮殑瀛楃涓?
 *@note
 *----------
 *return parent, or null if none.
 *&emsp; 濡傛灉鍩烘湰鍧椾负鍋囷紝閭ｄ箞杈撳嚭绌?
 *&emsp; 定义输出字符串string
 *&emsp; 添加基本块名称，
 *&emsp; 添加前置基本块的说明，依次打印前置基本块
 *&emsp; 隶属于函数则进行空行添加
 *&emsp; 渚濇鎵撳嵃缁存姢鐨勬寚浠ら摼琛ㄥ唴瀹?
 *&emsp; 濡傛灉鍩烘湰鍧楁棤缁堢粨鎸囦护锛岄粯璁ゆ坊鍔?
 */
std::string BasicBlock::print() {
  if (_fake) {
    return "";
  }
  std::string bb_ir;
  bb_ir += this->get_name();
  bb_ir += ":";
  // print prebb
  if (!this->get_pre_basic_blocks().empty()) {
    bb_ir += "                                                ; preds = ";
  }
  for (auto bb : this->get_pre_basic_blocks()) {
    if (bb != *this->get_pre_basic_blocks().begin())
      bb_ir += ", ";
    bb_ir += print_as_op(bb, false);
  }

  // print func
  if (!this->get_parent()) {
    bb_ir += "\n";
    bb_ir += "; Error: Block without parent!";
  }
  bb_ir += "\n";
  for (auto instr : this->get_instructions()) {
    bb_ir += "  ";
    bb_ir += instr->print();
    bb_ir += "\n";
  }

  // 空BasicBlock，自动加上return语句
  if (get_terminator() == nullptr) {
    bb_ir += "  ";
    if (get_parent()->get_return_type()->is_void_type()) {
      bb_ir += "ret void\n";
    } else {
      bb_ir += "ret i32 0\n";
    }
  }

  return bb_ir;
}
