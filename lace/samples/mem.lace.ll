; ModuleID = '/home/statim/lace/samples/mem.lace'
source_filename = "/home/statim/lace/samples/mem.lace"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "amd64-AMD-Linux"

%Metadata = type { i64, ptr, i8 }

@border = internal global ptr null
@head = internal global ptr null

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
define void @mem_copy(ptr %dst, ptr %src, i64 %n) #0 {
entry:
  %s = alloca ptr, align 8
  %d = alloca ptr, align 8
  %0 = alloca ptr, align 8
  store ptr %dst, ptr %0, align 8
  %1 = alloca ptr, align 8
  store ptr %src, ptr %1, align 8
  %2 = alloca i64, align 8
  store i64 %n, ptr %2, align 4
  br label %until.cnd

until.cnd:                                        ; preds = %until.bdy, %entry
  %3 = load i64, ptr %2, align 4
  %4 = icmp eq i64 %3, 0
  br i1 %4, label %until.mrg, label %until.bdy

until.bdy:                                        ; preds = %until.cnd
  %5 = load ptr, ptr %d, align 8
  %6 = load i64, ptr %2, align 4
  %7 = getelementptr inbounds i8, ptr %5, i64 %6
  %8 = load ptr, ptr %s, align 8
  %9 = load i64, ptr %2, align 4
  %10 = getelementptr inbounds i8, ptr %8, i64 %9
  %11 = load i8, ptr %10, align 1
  store i8 %11, ptr %7, align 1
  %12 = load i64, ptr %2, align 4
  store i64 %12, ptr %2, align 4
  %13 = sub i64 %12, 1
  br label %until.cnd

until.mrg:                                        ; preds = %until.cnd
  ret void
}

; Function Attrs: nounwind uwtable
define ptr @mem_alloc(i64 %size) #0 {
entry:
  %last = alloca ptr, align 8
  %align = alloca i64, align 8
  %block = alloca ptr, align 8
  %0 = alloca i64, align 8
  store i64 %size, ptr %0, align 4
  %1 = load i64, ptr %0, align 4
  %2 = urem i64 %1, 16
  %3 = icmp ne i64 %2, 0
  br i1 %3, label %if.thn, label %if.mrg

if.thn:                                           ; preds = %entry
  %4 = load i64, ptr %align, align 4
  store i64 %4, ptr %align, align 4
  %5 = add i64 %4, 16
  %6 = load i64, ptr %0, align 4
  %7 = urem i64 %6, 16
  %8 = sub i64 %5, %7
  br label %if.mrg

if.mrg:                                           ; preds = %if.thn, %entry
  %9 = load ptr, ptr @head, align 8
  %10 = icmp ne ptr %9, null
  br i1 %10, label %if.thn1, label %if.els

if.thn1:                                          ; preds = %if.mrg
  %11 = load i64, ptr %align, align 4
  %12 = call ptr @request_space(ptr null, i64 %11)
  store ptr %12, ptr %block, align 8
  %13 = load ptr, ptr %block, align 8
  %14 = icmp ne ptr %13, null
  br i1 %14, label %if.thn2, label %if.mrg3

if.thn2:                                          ; preds = %if.thn1
  ret ptr null

if.mrg3:                                          ; preds = %if.thn1
  %15 = load ptr, ptr %block, align 8
  store ptr %15, ptr @head, align 8
  br label %if.mrg9

if.els:                                           ; preds = %if.mrg
  %16 = load i64, ptr %align, align 4
  %17 = call ptr @find_free_space(ptr %last, i64 %16)
  store ptr %17, ptr %block, align 8
  %18 = load ptr, ptr %block, align 8
  %19 = icmp ne ptr %18, null
  br i1 %19, label %if.thn4, label %if.els5

if.thn4:                                          ; preds = %if.els
  %20 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 2
  store i8 0, ptr %20, align 1
  br label %if.mrg8

if.els5:                                          ; preds = %if.els
  %21 = load ptr, ptr %last, align 8
  %22 = load i64, ptr %align, align 4
  %23 = call ptr @request_space(ptr %21, i64 %22)
  store ptr %23, ptr %block, align 8
  %24 = load ptr, ptr %block, align 8
  %25 = icmp ne ptr %24, null
  br i1 %25, label %if.thn6, label %if.mrg7

if.thn6:                                          ; preds = %if.els5
  ret ptr null

if.mrg7:                                          ; preds = %if.els5
  br label %if.mrg8

if.mrg8:                                          ; preds = %if.mrg7, %if.thn4
  br label %if.mrg9

if.mrg9:                                          ; preds = %if.mrg8, %if.mrg3
  %26 = load ptr, ptr %block, align 8
  %27 = getelementptr inbounds ptr, ptr %26, i64 1
  ret ptr %27
}

; Function Attrs: nounwind uwtable
define void @mem_free(ptr %p) #0 {
entry:
  %block = alloca ptr, align 8
  %0 = alloca ptr, align 8
  store ptr %p, ptr %0, align 8
  %1 = load ptr, ptr %0, align 8
  %2 = icmp ne ptr %1, null
  br i1 %2, label %if.thn, label %if.mrg

if.thn:                                           ; preds = %entry
  ret void

if.mrg:                                           ; preds = %entry
  %3 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 2
  store i8 1, ptr %3, align 1
  %4 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 1
  %5 = load ptr, ptr %4, align 8
  %6 = icmp ne ptr %5, null
  br i1 %6, label %if.thn1, label %if.mrg4

if.thn1:                                          ; preds = %if.mrg
  %7 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 1
  %8 = getelementptr inbounds %Metadata, ptr %7, i32 0, i32 2
  %9 = load i8, ptr %8, align 1
  %10 = icmp ne i8 %9, 0
  br i1 %10, label %if.thn2, label %if.mrg3

if.thn2:                                          ; preds = %if.thn1
  %11 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 0
  %12 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 0
  %13 = load i64, ptr %12, align 4
  store i64 %13, ptr %11, align 4
  %14 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 1
  %15 = getelementptr inbounds %Metadata, ptr %14, i32 0, i32 0
  %16 = load i64, ptr %15, align 4
  %17 = add i64 %13, %16
  %18 = add i64 %17, 24
  %19 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 1
  %20 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 1
  %21 = getelementptr inbounds %Metadata, ptr %20, i32 0, i32 1
  %22 = load ptr, ptr %21, align 8
  store ptr %22, ptr %19, align 8
  br label %if.mrg3

if.mrg3:                                          ; preds = %if.thn2, %if.thn1
  br label %if.mrg4

if.mrg4:                                          ; preds = %if.mrg3, %if.mrg
  ret void
}

; Function Attrs: nounwind uwtable
define ptr @mem_realloc(ptr %p, i64 %size) #0 {
entry:
  %new_p = alloca ptr, align 8
  %block = alloca ptr, align 8
  %0 = alloca ptr, align 8
  store ptr %p, ptr %0, align 8
  %1 = alloca i64, align 8
  store i64 %size, ptr %1, align 4
  %2 = load ptr, ptr %0, align 8
  %3 = icmp ne ptr %2, null
  br i1 %3, label %if.thn, label %if.mrg

if.thn:                                           ; preds = %entry
  %4 = load i64, ptr %1, align 4
  %5 = call ptr @mem_alloc(i64 %4)
  ret ptr %5

if.mrg:                                           ; preds = %entry
  %6 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 0
  %7 = load i64, ptr %6, align 4
  %8 = load i64, ptr %1, align 4
  %9 = icmp uge i64 %7, %8
  br i1 %9, label %if.thn1, label %if.mrg2

if.thn1:                                          ; preds = %if.mrg
  %10 = load ptr, ptr %0, align 8
  ret ptr %10

if.mrg2:                                          ; preds = %if.mrg
  %11 = load ptr, ptr %new_p, align 8
  %12 = icmp ne ptr %11, null
  br i1 %12, label %if.thn3, label %if.mrg4

if.thn3:                                          ; preds = %if.mrg2
  ret ptr null

if.mrg4:                                          ; preds = %if.mrg2
  %13 = load ptr, ptr %new_p, align 8
  %14 = load ptr, ptr %0, align 8
  %15 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 0
  %16 = load i64, ptr %15, align 4
  call void @mem_copy(ptr %13, ptr %14, i64 %16)
  %17 = load ptr, ptr %0, align 8
  call void @mem_free(ptr %17)
  %18 = load ptr, ptr %new_p, align 8
  ret ptr %18
}

; Function Attrs: nounwind uwtable
define internal ptr @fetch_border() #0 {
entry:
  %0 = call ptr @brk(i64 0)
  store ptr %0, ptr @border, align 8
  %1 = load ptr, ptr @border, align 8
  ret ptr %1
}

; Function Attrs: nounwind uwtable
define internal ptr @get_block_pointer(ptr %p) #0 {
entry:
  %0 = alloca ptr, align 8
  store ptr %p, ptr %0, align 8
  %1 = load ptr, ptr %0, align 8
  %2 = getelementptr inbounds ptr, ptr %1, i64 -1
  ret ptr %2
}

; Function Attrs: nounwind uwtable
define internal ptr @extend_heap(i64 %inc) #0 {
entry:
  %new_border = alloca ptr, align 8
  %old_border = alloca ptr, align 8
  %0 = alloca i64, align 8
  store i64 %inc, ptr %0, align 4
  %1 = load ptr, ptr @border, align 8
  %2 = icmp ne ptr %1, null
  br i1 %2, label %if.thn, label %if.mrg2

if.thn:                                           ; preds = %entry
  %3 = call ptr @fetch_border()
  %4 = load ptr, ptr @border, align 8
  %5 = icmp ne ptr %4, null
  br i1 %5, label %if.thn1, label %if.mrg

if.thn1:                                          ; preds = %if.thn
  ret ptr inttoptr (i64 -1 to ptr)

if.mrg:                                           ; preds = %if.thn
  br label %if.mrg2

if.mrg2:                                          ; preds = %if.mrg, %entry
  %6 = load i64, ptr %0, align 4
  %7 = icmp eq i64 %6, 0
  br i1 %7, label %if.thn3, label %if.mrg4

if.thn3:                                          ; preds = %if.mrg2
  %8 = load ptr, ptr @border, align 8
  ret ptr %8

if.mrg4:                                          ; preds = %if.mrg2
  %9 = load ptr, ptr %new_border, align 8
  %10 = ptrtoint ptr %9 to i64
  %11 = call ptr @brk(i64 %10)
  %12 = load ptr, ptr %old_border, align 8
  %13 = icmp eq ptr %11, %12
  br i1 %13, label %if.thn5, label %if.mrg6

if.thn5:                                          ; preds = %if.mrg4
  ret ptr inttoptr (i64 -1 to ptr)

if.mrg6:                                          ; preds = %if.mrg4
  %14 = load ptr, ptr %new_border, align 8
  store ptr %14, ptr @border, align 8
  %15 = load ptr, ptr @border, align 8
  ret ptr %15
}

; Function Attrs: nounwind uwtable
define internal ptr @find_free_space(ptr %last, i64 %size) #0 {
entry:
  %curr = alloca ptr, align 8
  %0 = alloca ptr, align 8
  store ptr %last, ptr %0, align 8
  %1 = alloca i64, align 8
  store i64 %size, ptr %1, align 4
  br label %until.cnd

until.cnd:                                        ; preds = %if.mrg, %entry
  %2 = load ptr, ptr %curr, align 8
  %3 = icmp ne ptr %2, null
  br i1 %3, label %until.mrg, label %until.bdy

until.bdy:                                        ; preds = %until.cnd
  %4 = getelementptr inbounds %Metadata, ptr %curr, i32 0, i32 2
  %5 = load i8, ptr %4, align 1
  %6 = icmp ne i8 %5, 0
  br i1 %6, label %land.rgt, label %land.mrg

land.rgt:                                         ; preds = %until.bdy
  %7 = getelementptr inbounds %Metadata, ptr %curr, i32 0, i32 0
  %8 = load i64, ptr %7, align 4
  %9 = load i64, ptr %1, align 4
  %10 = icmp uge i64 %8, %9
  br label %land.mrg

land.mrg:                                         ; preds = %land.rgt, %until.bdy
  %11 = phi i1 [ false, %until.bdy ], [ %10, %land.rgt ]
  br i1 %11, label %if.thn, label %if.mrg

if.thn:                                           ; preds = %land.mrg
  br label %until.mrg

if.mrg:                                           ; preds = %land.mrg
  %12 = load ptr, ptr %0, align 8
  %13 = load ptr, ptr %curr, align 8
  store ptr %13, ptr %12, align 8
  %14 = getelementptr inbounds %Metadata, ptr %curr, i32 0, i32 1
  %15 = load ptr, ptr %14, align 8
  store ptr %15, ptr %curr, align 8
  br label %until.cnd

until.mrg:                                        ; preds = %if.thn, %until.cnd
  %16 = load ptr, ptr %curr, align 8
  ret ptr %16
}

; Function Attrs: nounwind uwtable
define internal ptr @request_space(ptr %last, i64 %size) #0 {
entry:
  %border_meta = alloca ptr, align 8
  %mem = alloca ptr, align 8
  %block = alloca ptr, align 8
  %total_size = alloca i64, align 8
  %0 = alloca ptr, align 8
  store ptr %last, ptr %0, align 8
  %1 = alloca i64, align 8
  store i64 %size, ptr %1, align 4
  %2 = load ptr, ptr %border_meta, align 8
  store ptr %2, ptr %block, align 8
  %3 = load i64, ptr %total_size, align 4
  %4 = call ptr @extend_heap(i64 %3)
  store ptr %4, ptr %mem, align 8
  %5 = load ptr, ptr %0, align 8
  %6 = icmp ne ptr %5, null
  br i1 %6, label %if.thn, label %if.mrg

if.thn:                                           ; preds = %entry
  %7 = getelementptr inbounds %Metadata, ptr %0, i32 0, i32 1
  %8 = load ptr, ptr %block, align 8
  store ptr %8, ptr %7, align 8
  br label %if.mrg

if.mrg:                                           ; preds = %if.thn, %entry
  %9 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 0
  %10 = load i64, ptr %1, align 4
  store i64 %10, ptr %9, align 4
  %11 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 1
  store ptr null, ptr %11, align 8
  %12 = getelementptr inbounds %Metadata, ptr %block, i32 0, i32 2
  store i8 0, ptr %12, align 1
  %13 = load ptr, ptr %block, align 8
  ret ptr %13
}

attributes #0 = { nounwind uwtable "frame-pointer"="all" "target-cpu"="x86-64" }
