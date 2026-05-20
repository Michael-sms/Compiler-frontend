; ModuleID = 'sysy2022_compiler'
source_filename = "tests/ir_cases/ir07_const.sy"
@N = constant i32 5
declare i32 @getint()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putint(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()

define i32 @main() {
main_ENTRY:
  %op0 = load i32, i32* @N
  ret i32 %op0
}
