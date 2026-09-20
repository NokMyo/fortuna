.section .data
one: .double 1.0
rho: .double 1.0
.section .bss
.p2align 4
history_count: .skip 4
last_round: .skip 4
available_first_round: .skip 4
available_last_round: .skip 4
parse_error: .skip 4
parse_line: .skip 4
model_ready: .skip 4
field_ready: .skip 4
field_count: .skip 4
session_count: .skip 4
selected_index: .skip 4
validation_count: .skip 4
progress: .skip 4
analysis_stage: .skip 4
cancel_flag: .skip 4
selected_mask: .skip 8
selected_nums: .skip 24
session_masks: .skip 80
elapsed_ms: .skip 8
api_snapshot: .skip 272
api_progress: .skip 16
report_buffer: .skip 2097152
# UI-only performance calibration and dynamic time estimates.
benchmark_ms: .skip 4
benchmark_ready: .skip 4
benchmark_sink: .skip 8
estimate_ready: .skip 4
estimate_last_start: .skip 4
estimate_last_end: .skip 4
estimate_normal_minutes: .skip 4
estimate_deep_minutes: .skip 4
ui_int_ok: .skip 4
ui_est_normal: .skip 128
ui_est_deep: .skip 128

.section .bss
app_research_mode: .skip 4
forecast_path: .skip 2048

app_draw_kind: .skip 4
app_request_kind: .skip 4
app_model_kind: .skip 4
.section .data
app_ticket_count: .long 1
