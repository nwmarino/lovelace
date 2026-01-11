; ModuleID = '/home/statim/lace/samples/A.lace'
source_filename = "/home/statim/lace/samples/A.lace"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "amd64-AMD-Linux"

@0 = private unnamed_addr constant [5 x i8] c"hey!\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"hey!\0A\00", align 1

; Function Attrs: nounwind uwtable
declare void @exit(i64) #0

; Function Attrs: nounwind uwtable
declare i64 @open(ptr, i64, i64) #0

; Function Attrs: nounwind uwtable
declare i64 @close(i64) #0

; Function Attrs: nounwind uwtable
declare i64 @write(i64, ptr, i64) #0

; Function Attrs: nounwind uwtable
declare ptr @brk(i64) #0

; Function Attrs: nounwind uwtable
declare void @mem_copy(ptr, ptr, i64) #0

; Function Attrs: nounwind uwtable
declare ptr @mem_alloc(i64) #0

; Function Attrs: nounwind uwtable
declare void @mem_free(ptr) #0

; Function Attrs: nounwind uwtable
declare ptr @mem_realloc(ptr, i64) #0

; Function Attrs: nounwind uwtable
define internal i64 @strlen(ptr %str) #0 {
entry:
  %len = alloca i64, align 8
  %0 = alloca ptr, align 8
  store ptr %str, ptr %0, align 8
  br label %until.cnd

until.cnd:                                        ; preds = %until.bdy, %entry
  %1 = load ptr, ptr %0, align 8
  %2 = load i64, ptr %len, align 4
  %3 = getelementptr inbounds i8, ptr %1, i64 %2
  %4 = load i8, ptr %3, align 1
  %5 = icmp eq i8 %4, 0
  br i1 %5, label %until.mrg, label %until.bdy

until.bdy:                                        ; preds = %until.cnd
  %6 = load i64, ptr %len, align 4
  store i64 %6, ptr %len, align 4
  %7 = add i64 %6, 1
  br label %until.cnd

until.mrg:                                        ; preds = %until.cnd
  %8 = load i64, ptr %len, align 4
  ret i64 %8
}

; Function Attrs: nounwind uwtable
define internal void @print(ptr %str) #0 {
entry:
  %0 = alloca ptr, align 8
  store ptr %str, ptr %0, align 8
  %1 = load ptr, ptr %0, align 8
  %2 = load ptr, ptr %0, align 8
  %3 = call i64 @strlen(ptr %2)
  %4 = call i64 @write(i64 1, ptr %1, i64 %3)
  ret void
}

; Function Attrs: nounwind uwtable
define i64 @main() #0 {
entry:
  %x = alloca i64, align 8
  %p = alloca ptr, align 8
  %fd = alloca i64, align 8
  %0 = load i64, ptr %fd, align 4
  %1 = call i64 @write(i64 %0, ptr @0, i64 4)
  %2 = load i64, ptr %fd, align 4
  %3 = call i64 @close(i64 %2)
  %4 = load ptr, ptr %p, align 8
  store i64 5, ptr %4, align 4
  %5 = load ptr, ptr %p, align 8
  call void @mem_free(ptr %5)
  call void @print(ptr @1)
  %6 = load i64, ptr %x, align 4
  ret i64 %6
}

attributes #0 = { nounwind uwtable "frame-pointer"="all" "target-cpu"="x86-64" }
