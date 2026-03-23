# MANIFESTO

## Tree and Blocks

(intro)

### Project Seed

### Asset Topology

The `assets/` directory has been designed to serve as the project's artifact layer. It is not intended for arbitrary files and must not be used as some rubbish dump: the contents of `assets/` are grouped by purpose through a fixed first-level directory layout. Within the project’s working environment, this directory is also treated as a source for copying or linking into `build/`, `.manifest/`, and other service environments; `assets/showcase/` is excluded from this rule, as it is oriented primarily toward documentation and project presentation.

* The `assets/brand/` directory is intended for the project’s visual identity. The preferred format here is `svg`, since it preserves the possibility of meaningful manual editing while remaining readable as code. This directory is also the expected home of `favicon.ico`.

* The `assets/showcase/` directory is intended for stable project artifacts: screenshots, screen recordings, demo materials, PDF files, and other media. Such files may be used, for example, in documentation and `readme.md`. The contents of `showcase/` are treated primarily as artifacts in their own right rather than as line-oriented files meant for manual editing.

* The `assets/reports/` directory is intended for reports and run results. Its contents are generated automatically and may remain informative not only on their own, but also when compared across versions. The presence of `assets/reports/` can be fixed conveniently via `assets/reports/.gitkeep`. Files inside `reports/` use a suffix-based naming scheme: the current version is marked with the `--latest` suffix, while versioned snapshots use a suffix of the form `--YYYYMMDD-HHMMSS`.

* The `assets/dataset/` directory is intended for input data sets used in tests, examples, and related validation scenarios. The naming at the first level of nesting is fixed: `test/`, `train/`, `stress/`, and `todo/`. These branches are optional in general, but if `dataset/` is not empty, `test/` must be present. Below that layer, the structure is left unconstrained: both a flat file layout and arbitrary further subdivision into subdirectories are permitted.

* The `assets/dumps/` directory is intended for auto-generated outputs, including results produced from runs over data sets in `dataset/`. The first level of nesting in `dumps/` must mirror the first level of `dataset/`; in other words, `dumps/test/`, `dumps/train/`, `dumps/stress/`, and `dumps/todo/` are only valid as reflections of the corresponding input branches. Below that layer, the structure remains unrestricted. At the same time, an explicit escape hatch is reserved here: despite the strict structural rule, `assets/.gitignore` may still ignore any subset of `dumps/`, including first-level directories.

* The `assets/sets/` directory is intended for artifacts that the application uses and renders at runtime, together with the basic metadata and settings associated with them. These may include themes, portable serialized state representations, configuration sets, and other runtime artifacts that the program treats as ready-made input entities.

* The `assets/templates/` directory is intended for file-based templates used by the program when generating data, text, or other derived artifacts. The general rule here is simple: long template strings should not be kept directly in code when they can be moved into a dedicated file instead. The internal structure of `templates/` is left unconstrained and may branch arbitrarily if that helps organize the templates themselves.

Within any first-level directory inside `assets/`, the meta-pair `index.tsv` and `readme.md` may be used whenever an ordered listing of media or runtime artifacts is needed. The `index.tsv` file defines the subset of files that participates in a carousel, listing, or any other derived representation; the row order in the table also defines the display order. The table is expected to use the columns `id`, `path`, `type`, `description`, and `datetime`. In the `path` column, the file is referenced by its relative path from the directory that contains `index.tsv`; for example, if the table is located at `assets/dataset/index.tsv` and the file is located at `assets/dataset/test/some_dir/example1.ex`, then the value stored in `path` is `test/some_dir/example1.ex`.

If `readme.md` is present next to `index.tsv`, it must contain a `<carousel/>` block. If `readme.md` is absent, this is treated as if a virtual `readme.md` existed next to `index.tsv` and contained nothing but that block. Carousels and similar auxiliary documentation pages are generated automatically from this pair of files. At the moment, the supported artifact types include images, videos, and PDF files. The `index.tsv` file must not be edited manually and is expected to be generated only through the controlling CLI tool.

### Generated and Service Blocks

### Peripheral Blocks
