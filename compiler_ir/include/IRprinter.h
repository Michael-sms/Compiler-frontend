/*!
 *@file IRprinter.h
 *@brief 涓棿璇█杈撳嚭鎺ュ彛澶存枃浠?
 *@version 1.0.0
 *@date 2022-10-04
 */

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "Instruction.h"
#include "Module.h"
#include "Type.h"
#include "User.h"
#include "Value.h"

/*!
 *@brief 鎵撳嵃operands鐨勫悕绉?
 *@return 瀛楃涓?
 *@note
 *---------
 *初始化字符串
 *&emsp; **if** 濡傛灉鎵撳嵃绫诲瀷锛屾墦鍗扮被鍨嬫暟鍊?
 *&emsp; **if** 如果属于全局变量，打印@+对应名称
 *&emsp;&emsp; **else if** 如果属于函数量，打印@+对应名称
 *&emsp;&emsp; **else if** 濡傛灉灞炰簬甯搁噺锛屾墦鍗板搴斿悕绉?
 *&emsp;&emsp; **else** 如果属于普通变量，打印%+对应名称
 *&emsp; 杩斿洖瀛楃涓?
 */
std::string print_as_op(Value *v, bool print_ty);

/*!
 *@brief 鎵撳嵃姣旇緝operands鐨勫悕绉?
 *@return 瀛楃涓?
 *@note
 *---------
 *
 */
std::string print_cmp_type(CmpInst::CmpOp op);
