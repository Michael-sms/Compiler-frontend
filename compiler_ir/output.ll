; ModuleID = 'sysy2022_compiler'
source_filename = "tests/parser_b_cases/case1.sy"
@a = global i32 0
declare i32 @getint()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putint(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()
