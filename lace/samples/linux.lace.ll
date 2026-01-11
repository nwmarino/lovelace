; ModuleID = '/home/statim/lace/samples/linux.lace'
source_filename = "/home/statim/lace/samples/linux.lace"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "amd64-AMD-Linux"

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

attributes #0 = { nounwind uwtable "frame-pointer"="all" "target-cpu"="x86-64" }
