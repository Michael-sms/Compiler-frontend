; ModuleID = 'sysy2022_compiler'
source_filename = "tests/ir_cases/ir03_main_return.sy"
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
  ret i32 0
}
