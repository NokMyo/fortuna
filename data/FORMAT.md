# CSV input contract — 1.0.0

UTF-8 (optional BOM), comma separated, CRLF or LF. Maximum 16 MiB / 8,192 draws. Blank lines are allowed. An optional ASCII header starting with `round` must be the first physical line.

```csv
round,n1,n2,n3,n4,n5,n6,bonus
1,10,23,29,33,37,40,16
```

```csv
round,date,n1,n2,n3,n4,n5,n6,bonus
1,2002-12-07,10,23,29,33,37,40,16
```

Bonus is optional in either form. Date accepts `YYYY-MM-DD` or `YYYYMMDD`. Use the same column form throughout a file. Dates, when supplied, must be valid Gregorian dates in 2002–9999 and strictly increase with the round. The parser does not authenticate the source or cross-check each date against the official weekly calendar.

Main numbers must be six distinct integers 1–45. Bonus, when present, must be 1–45 and different from all six. Integers may be double-quoted and surrounded by spaces. Embedded quotes, escaped CSV payloads, comments, fractional/signed values, empty fields and extra columns are rejected. Round numbers must be integers 1–1,000,000. Rows and main numbers can be unsorted on input; canonicalization sorts both. Duplicate or missing rounds are rejected. A dataset may begin at a round other than 1 but must then be contiguous.

Import is transactional: all rows are parsed and verified in a staging buffer. One error rejects the entire import, leaving the prior dataset and hash intact. Successful replacement invalidates the prior analysis and generated display.

The SHA-256 dataset identifier hashes canonical 48-byte little-endian records: round u32, date YYYYMMDD u32 (0 if absent), six sorted u32 main numbers, bonus u32 (0 if absent), zero u32 reserved, 45-bit mask u64. It identifies content; it does not certify that the content is official.

`SYNTHETIC-example.csv` is a deterministic **fictional** 60-draw fixture for testing. Do not treat it as lottery history or use it to assess predictive performance. Obtain and verify your actual history independently. The application does not make background network requests.
