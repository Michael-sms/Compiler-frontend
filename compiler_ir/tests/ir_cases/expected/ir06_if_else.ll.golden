; ModuleID = 'sysy2022_compiler'
source_filename = "tests/ir_cases/ir06_if_else.sy"
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
  %op0 = icmp slt i32 1, 2
  br i1 %op0, label %label_then, label %label_else
label_then:                                                ; preds = %main_ENTRY
  ret i32 10
label_ifcont:
  ret i32 0
label_else:                                                ; preds = %main_ENTRY
  ret i32 20
}
