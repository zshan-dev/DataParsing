## Property Prices: High-Performance CSV Ingestion and Hash Indexing in C

This project demonstrates how to efficiently ingest large CSV datasets and build an in-memory hash index for fast lookups, implemented in portable C. It loads UK Property Price data from CSV (e.g., `pp-2024.csv`, `pp-2023.csv`), constructs a hash index on the `street` field, and compares linear search with indexed lookups, reporting timing and load factor statistics.

### Features
- **Fast CSV ingestion**: Streams and parses rows without loading the entire file into memory first.
- **Append-only table**: Multiple CSV files can be read sequentially; rows are appended to a single global table.
- **Hash index on street**: Builds a chained hash table for O(1) average-time street lookups.
- **Benchmarking**: Compares linear search vs. hash-index search and prints wall-clock timings.
- **Load factor insights**: Reports unused buckets and load factor to reason about index quality.
- **Memory safety**: Carefully allocates and frees dynamic memory for variable-length fields.

### How it works
- **Data model**: Records are stored in a contiguous dynamic array of `Record` structs (`table`) with a global `table_size` for row count. Variable-length `district` is heap-allocated per row.
- **CSV parsing**: Each line is read via `fgets` and parsed with a single `sscanf` format string that handles quoted fields. The parser extracts fixed-size fields directly into struct buffers and copies `district` to a right-sized allocation.
- **Incremental growth**: The table grows as lines are read via `realloc`, enabling processing of very large inputs without precomputing row counts.
- **Hash index**: The function `createIndexOnStreet` builds an `IndexEntry**` array of size `INDEX_SIZE` (separate chaining). Collisions are handled by prepending `IndexEntry` nodes in each bucket. The hash function is a classic DJB2 variant modulo `INDEX_SIZE`.
- **Searching**:
  - Linear: `searchStreetLinear` scans the table and compares `street` names.
  - Indexed: `searchStreet` hashes the target street and traverses only that bucket’s chain.
- **Benchmarking and stats**: `main.c` times both search modes and prints `unused slots`, `load factor`, and elapsed seconds for each strategy.

### Why it handles large CSVs with ease
- **Streaming I/O**: Reads one line at a time; memory usage is proportional to rows actually stored, not file size at once.
- **Single-pass parsing**: `sscanf` unpacks fields directly into the struct layout, minimizing copies.
- **Amortized growth**: While `realloc` per row is used here for clarity, modern allocators make this surprisingly efficient; growth can be tuned (e.g., geometric) if you need even faster bulk loads.
- **O(1) average indexing**: After build, street lookups avoid table scans, keeping query latency flat as datasets grow.

### Quantifiable aspects
- **Time complexity**:
  - Ingestion: O(N) to read and parse N rows.
  - Index build: O(N) inserts with separate chaining.
  - Search: O(N) linear vs. O(1) average with hashing (O(K) where K is bucket length).
- **Space usage**:
  - Table: O(N) records.
  - Index: O(INDEX_SIZE + N) `IndexEntry` nodes and keys.
- **Load factor (α = used_buckets / INDEX_SIZE)**:
  - Reported at runtime to guide `INDEX_SIZE` tuning for your dataset.

### Project layout
- `main.c`: Orchestrates file loads, index creation, timings, and stats.
- `myDSlib.h`: Public types, constants, and function declarations (`Record`, `IndexEntry`, `INDEX_SIZE`, etc.).
- `myDSlib.c`: CSV ingestion, hashing, index build/search, memory management.
- `pp-2023.csv`, `pp-2024.csv`: Sample input data files.
- `Makefile`: Build automation.

### Build
```bash
make
```

This produces an executable (commonly `main` or target defined in the `Makefile`).

### Run
```bash
./main
```

Expected console output includes total records appended after each file, creation of the hash index, timings for linear vs. hash search, and hash table stats such as unused slots and load factor. Example excerpt:

```text
Total records appneded: 850000
Total records appneded: 1700000

Hash index on street created.

Time (Linear Search): 0.842311 seconds
Time (Hash Index)   : 0.000317 seconds

Unused hash slots: 12 out of 100 (12.00% unused)
Hash table load factor: 0.880
```

Note: Output magnitudes depend on dataset size and `INDEX_SIZE`.

### Tuning and customization
- **`INDEX_SIZE`**: Adjust in `myDSlib.h` to balance memory and collision rate. Larger values reduce chain lengths and speed up lookups at the cost of more memory.
- **Printing matches**: Toggle `printFlagLinearSearch` and `printFlagHashIndexSearch` in `main.c` to inspect found records.
- **Target field**: The index is built on `street`, but you can build additional indices (e.g., on `postcode`) by following the same pattern.

### Memory management
- Every `district` string is individually allocated and freed in `free_table`.
- Index nodes and copied keys are freed in `free_index`.
- All resources are released at program exit.

### Limitations and extensions
- Current ingestion uses `realloc` per row for simplicity; switching to capacity doubling will improve bulk load throughput.
- The CSV parser assumes a consistent quoted format; if your CSV varies, adjust the `sscanf` format or use a dedicated CSV parser.
- Consider normalizing `street` case and trimming whitespace to improve match rates.


