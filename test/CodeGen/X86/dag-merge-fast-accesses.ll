; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=-slow-unaligned-mem-16 | FileCheck %s --check-prefix=FAST
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+slow-unaligned-mem-16 | FileCheck %s --check-prefix=SLOW

; Verify that the DAGCombiner is creating unaligned 16-byte loads and stores
; if and only if those are fast.

define void @merge_const_vec_store(i64* %ptr) {
; FAST-LABEL: merge_const_vec_store:
; FAST:       # BB#0:
; FAST-NEXT:    xorps %xmm0, %xmm0
; FAST-NEXT:    movups %xmm0, (%rdi)
; FAST-NEXT:    retq
;
; SLOW-LABEL: merge_const_vec_store:
; SLOW:       # BB#0:
; SLOW-NEXT:    movq $0, (%rdi)
; SLOW-NEXT:    movq $0, 8(%rdi)
; SLOW-NEXT:    retq

  %idx0 = getelementptr i64, i64* %ptr, i64 0
  %idx1 = getelementptr i64, i64* %ptr, i64 1

  store i64 0, i64* %idx0, align 8
  store i64 0, i64* %idx1, align 8
  ret void
}


define void @merge_vec_element_store(<4 x double> %v, double* %ptr) {
; FAST-LABEL: merge_vec_element_store:
; FAST:       # BB#0:
; FAST-NEXT:    movups %xmm0, (%rdi)
; FAST-NEXT:    retq
;
; SLOW-LABEL: merge_vec_element_store:
; SLOW:       # BB#0:
; SLOW-NEXT:    movlpd %xmm0, (%rdi)
; SLOW-NEXT:    movhpd %xmm0, 8(%rdi)
; SLOW-NEXT:    retq

  %vecext0 = extractelement <4 x double> %v, i32 0
  %vecext1 = extractelement <4 x double> %v, i32 1

  %idx0 = getelementptr double, double* %ptr, i64 0
  %idx1 = getelementptr double, double* %ptr, i64 1

  store double %vecext0, double* %idx0, align 8
  store double %vecext1, double* %idx1, align 8
  ret void
}


;; TODO: FAST *should* be:
;;    movups (%rdi), %xmm0
;;    movups %xmm0, 40(%rdi)
;; ..but is not currently. See the UseAA FIXME in DAGCombiner.cpp
;; visitSTORE.

define void @merge_vec_load_and_stores(i64 *%ptr) {
; FAST-LABEL: merge_vec_load_and_stores:
; FAST:       # BB#0:
; FAST-NEXT:    movq (%rdi), %rax
; FAST-NEXT:    movq 8(%rdi), %rcx
; FAST-NEXT:    movq %rax, 40(%rdi)
; FAST-NEXT:    movq %rcx, 48(%rdi)
; FAST-NEXT:    retq
;
; SLOW-LABEL: merge_vec_load_and_stores:
; SLOW:       # BB#0:
; SLOW-NEXT:    movq (%rdi), %rax
; SLOW-NEXT:    movq 8(%rdi), %rcx
; SLOW-NEXT:    movq %rax, 40(%rdi)
; SLOW-NEXT:    movq %rcx, 48(%rdi)
; SLOW-NEXT:    retq

  %idx0 = getelementptr i64, i64* %ptr, i64 0
  %idx1 = getelementptr i64, i64* %ptr, i64 1

  %ld0 = load i64, i64* %idx0, align 4
  %ld1 = load i64, i64* %idx1, align 4

  %idx4 = getelementptr i64, i64* %ptr, i64 5
  %idx5 = getelementptr i64, i64* %ptr, i64 6

  store i64 %ld0, i64* %idx4, align 4
  store i64 %ld1, i64* %idx5, align 4
  ret void
}

