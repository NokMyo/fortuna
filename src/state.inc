.section .rdata,"dr"
class_name:         .asciz "FebiusFortunaWindow"
app_title:          .asciz "Febius Fortuna"
ui_title:           .asciz "Febius Fortuna"
ui_subtitle:        .asciz "ORACLE analytical random generator — Lotto 6/45"
ui_result_empty:    .asciz "--  --  --  --  --  --"
ui_status_ready:    .asciz "CSV data required. Load historical draws to begin."
ui_status_loading:  .asciz "Loading and validating draw data..."
ui_status_loaded:   .asciz "Loaded %u valid draws. ORACLE is ready to analyze."
ui_status_analyzing:.asciz "ORACLE: exhaustive scan of 8,145,060 combinations is running..."
ui_status_done:     .asciz "Analysis complete. Recalculate samples the same ORACLE probability field."
ui_status_error:    .asciz "Could not load the CSV or no valid draw rows were found."
ui_status_needdata: .asciz "Load at least 30 valid historical draws first."
ui_status_busy:     .asciz "Analysis is already running."
btn_load:           .asciz "Load CSV"
btn_analyze:        .asciz "Analyze"
btn_recalc:         .asciz "Recalculate"
metrics_empty:      .asciz "ORACLE metrics will appear here after analysis."
disclaimer:         .asciz "Internal model scores are not winning probabilities. In a fair 6/45 draw, every combination has the same jackpot probability."
static_class:       .asciz "STATIC"
button_class:       .asciz "BUTTON"
edit_class:         .asciz "EDIT"
font_face:          .asciz "Segoe UI"
open_title:         .asciz "Load Lotto 6/45 history CSV"
filter_csv:         .ascii "CSV files (*.csv)\0*.csv\0All files (*.*)\0*.*\0\0"
fmt_result:         .asciz "%02u   %02u   %02u   %02u   %02u   %02u"
fmt_metrics:        .asciz "ORACLE Index      %u / 1000\r\nDraws             %u\r\nSearch             8,145,060 exhaustive + 4,096 deep\r\nSkepticism        %u%%\r\nSelected score    %d\r\nTop persistence   %u(%u)  %u(%u)  %u(%u)  %u(%u)  %u(%u)\r\nEngine             ORACLE ASM 0.1"

.section .data
.p2align 3
h_instance:      .quad 0
h_main:          .quad 0
h_result:        .quad 0
h_status:        .quad 0
h_metrics:       .quad 0
h_btn_load:      .quad 0
h_btn_analyze:   .quad 0
h_btn_recalc:    .quad 0
h_font_title:    .quad 0
h_font_result:   .quad 0
h_font_ui:       .quad 0
analysis_busy:   .long 0
analysis_ready:  .long 0
draw_count:      .long 0
skepticism_pct:  .long 25
selected_mask:   .quad 0
selected_score:  .quad 0
final_count:     .long 0
fast_count:      .long 0
recent_pos:      .long 0

.section .bss
.p2align 4
msg_buf:         .skip 48
wc_buf:          .skip 80
ofn_buf:         .skip 152
file_path:       .skip 520
status_buf:      .skip 1024
result_buf:      .skip 256
metrics_buf:     .skip 2048
file_size_q:     .skip 8
bytes_read:      .skip 4

# Historical statistics
freq_total:      .skip 45*4
freq_short:      .skip 45*4
freq_mid:        .skip 45*4
freq_long:       .skip 45*4
last_seen:       .skip 45*4
pair_counts:     .skip 45*45*4
hist_odd:        .skip 7*4
hist_low:        .skip 7*4
hist_adj:        .skip 6*4
sum_acc:         .skip 8
sum_sq_acc:      .skip 8
range_acc:       .skip 8
range_sq_acc:    .skip 8
recent_masks:    .skip 64*8
columns:         .skip 12*4
work_nums:       .skip 6*4
selected_nums:   .skip 6*4
persistence:     .skip 45*4

# Stage A heap and final heap: parallel mask/score arrays
fast_masks:      .skip TOP_FAST*8
fast_scores:     .skip TOP_FAST*8
final_masks:     .skip TOP_FINAL*8
final_scores:    .skip TOP_FINAL*8
