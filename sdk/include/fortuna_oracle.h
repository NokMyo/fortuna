#ifndef FORTUNA_ORACLE_H
#define FORTUNA_ORACLE_H
/* ABI declarations only. Engine and example executable are pure x64 assembly. */
#include <stdint.h>
#include <wchar.h>
#ifdef __cplusplus
extern "C" {
#endif
#define FORTUNA_ORACLE_ABI_V1 0x00010000u
#define FO_OK 0
#define FO_REUSED 1
#define FO_INVALID_ARGUMENT (-1)
#define FO_BUSY (-2)
#define FO_OPERATION_FAILED (-3)
#define FO_NOT_READY (-4)
#define FO_RANDOM_FAILED (-5)
#define FO_CANCELLED (-6)
#define FO_API __declspec(dllimport)
#pragma pack(push, 8)
typedef struct FortunaOracleSnapshot {
 uint32_t size, abi_version, engine_version, flags;
 uint32_t history_count, last_round, parse_error, parse_line;
 uint32_t field_count, validation_count, selected_index, session_count;
 uint64_t selected_mask;
 uint32_t selected_numbers[6];
 uint64_t session_masks[10];
 double uniform_mixture, evidence;
 uint8_t dataset_sha256[32], field_sha256[32], config_sha256[32];
} FortunaOracleSnapshot;
typedef struct FortunaOracleProgress {
 uint32_t stage, percent, validation_count, history_count;
} FortunaOracleProgress;
#pragma pack(pop)
FO_API uint32_t FortunaOracleGetAbiVersion(void);
FO_API uint32_t FortunaOracleGetEngineVersion(void);
FO_API int32_t FortunaOracleInitialize(void);
FO_API int32_t FortunaOracleLoadCsv(const void *utf8, uint64_t bytes);
FO_API int32_t FortunaOracleLoadCsvFileW(const wchar_t *path);
FO_API int32_t FortunaOracleAnalyze(void);
FO_API int32_t FortunaOracleCancel(void);
FO_API int32_t FortunaOracleGenerate(uint32_t count, uint64_t *masks, uint32_t capacity);
FO_API int32_t FortunaOracleGetSnapshot(FortunaOracleSnapshot *out, uint32_t bytes);
FO_API int32_t FortunaOracleGetProgress(FortunaOracleProgress *out, uint32_t bytes);
/* Positive return = required UTF-8 bytes INCLUDING NUL; negative = error.
   Query with NULL,0. A too-small buffer is never written. */
FO_API int32_t FortunaOracleGetReport(void *out, uint32_t bytes);
/* 0 appended, 1 reused, negative error. See snapshot for sealed selection. */
FO_API int32_t FortunaOracleSealW(const wchar_t *path);
FO_API int32_t FortunaOracleUseUniform(void);
/* Destructive diagnostic; replaces this process's in-memory analysis state.
   Returns 0 success, negative API errors or 10+ self-test failure stage. */
FO_API int32_t FortunaOracleSelfTest(void);
#ifdef __cplusplus
}
static_assert(sizeof(FortunaOracleSnapshot)==272, "ABI layout mismatch");
static_assert(sizeof(FortunaOracleProgress)==16, "ABI layout mismatch");
#endif
#endif
