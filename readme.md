# MANIFESTO

## Tree and Blocks

This section fixes the structure of the project: from source code to artifact, peripheral, service, and auto-generated blocks. If a block is present in the repository, its place and role are considered fixed. File names should not scream at me: wherever a name is not fixed separately, `snake_case` is used.

### Project Seed

A project must have both structure and meaning. `manifest.json` defines its formal side and stores its meta-details. `readme.md` reveals its idea and soul and must not be reduced to a dry run guide.

The `core/` and `app/` blocks are intended for placing the project's source code. The presence of at least one of them is required. Both blocks use the same internal structure: `include/`, `src/`, `tests/`, and, where appropriate, `benchmarks/`. These directories form a consistent tree. If related files are present in more than one of them, they preserve the same relative path within the directory and follow a common naming pattern.

* `include/` defines the reference layer of this structure. It is expected to contain primarily interface and template files; more specific rules for allowed extensions and related conventions are fixed in `manifest.json` and in coding style.

* `src/` contains the block's working source files, including implementation files and `entry-point` sources.

* `tests/` contains test files and is strongly encouraged for header-defined components with behaviour of their own. 

* `benchmarks/` is an optional directory for benchmark files and is introduced only where performance measurements are actually justified.

For files in `tests/` and `benchmarks/`, the same general principle is used: the base name is preserved, while the layer type is expressed by a suffix. The suffix for `tests/` is `_tests`; the suffix for `benchmarks/` is `_benchmarks`. If a source file becomes too large, its `.cpp` part may be split into a small number of logically grouped batches indexed from `0` to `9`; if there is only one such batch, the `0` index is preferably omitted.


The distinction between `core/` and `app/` lies primarily at the `entry-point` level. The `core/` block is intended for a CLI, so each of its `entry-point`s must be accompanied by a `readme.md`. The `app/` block is intended for a windowed interface, so a separate `readme.md` inside it is generally not required. In all other respects, both blocks follow the same structure. Dependencies between these blocks are asymmetric: `app/`may depend on`core/`, whereas `core/`must remain independent of`app/`.

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

The files `license`, `citation.cff` and `CMakeLists.txt`, together with the `.github/` directory, form a related group of auto-generated blocks whose contents are derived from templates with data substituted from `manifest.json`. The overall structure of these blocks remains predictable, while their final contents are determined by the description of the particular project and may therefore differ from one repository to another. These blocks are not meant to be edited manually and should be treated as derived artifacts rather than as independent points of configuration. See the section on `manifest.json` for details.

A project may contain up to three `.gitignore` files. The root `.gitignore` is common to all projects and is generated automatically. A second `.gitignore` may appear in `bindings/`; it is auto-extended according to the languages and toolchains actually used there. The third may appear in `assets/`, and this is the only `.gitignore` that may be edited manually. Even then, it is expected to be used exclusively for filtering subsets of `assets/dumps/`, so that the repository keeps the right balance: first and foremost, the project is a source-code repository, not a storage site for automatically generated dumps, however useful those dumps may sometimes be as examples or reference artifacts. In particular, `assets/` must not be turned into some rubbish dump.

The files `.clang-tidy` and `.clang-format` form a related pair of auto-generated root-level templates. Like the root `.gitignore`, they belong to the shared project layout rather than to the project-specific layer derived from `manifest.json`, so their contents remain the same across repositories. Together they define a common baseline for linting and formatting, including the behaviour expected by the CI workflows described in `.github/`. At present, however, both files are covered by `.gitignore` and are therefore not committed, even though they still remain part of the generated project layout; this policy may change in the future.

The `build/` and `.manifest/` directories form a related pair of local service workspaces that may appear at the project root and are expected to remain covered by `.gitignore`. Unlike blocks generated directly from fixed templates, these directories are less tightly bound to the internal template layer and are shaped more by the actual execution of the build and control flows. They may contain intermediate states, generated artifacts, caches, and other transient by-products of project activity, and for that reason should not be treated as part of the meaningful source tree. Manual editing and committing are both strongly discouraged.

### Peripheral Blocks

These blocks are not mandatory, yet whenever they appear they become integral parts of the project and are expected to occupy their fixed places in the tree.

* The `bindings/` directory is intended for integrations and wrapper layers around the main project for other languages and external ecosystems. Its first level of nesting is organized by target environment or language, although the concrete names of such subdirectories are fixed only as the corresponding support appears and stabilizes; the list of supported bindings may therefore be specified more explicitly later. In this context, a `java/` directory inside `bindings/` should be understood broadly: it may denote the JVM block as a whole rather than Java code in the narrow sense alone, so placing Kotlin code inside `bindings/java/` is considered normal. The `bindings/` block should also be expected to receive automatic `.gitignore` updates based on the set of languages and toolchain environments actually used in the project; in that sense, it forms a local ecosystem of its own, somewhat less dependent on the repository’s outer structure as a whole.

* The `shim/` directory is intended for thin external shims invoked by the main code only where such separation is genuinely justified. It is not a place for user-facing scripts, development utilities, build or run wrappers, or a substitute for the facade; for that reason, the name `scripts/` and `Makefile` are forbidden for this kind of block, since it blurs distinct roles. The contents of `shim/` are not meant to be invoked manually in the ordinary workflow, and preference should be given to keeping the main logic in C++ whenever possible, leaving `shim/` only for external integration points that are actually necessary.

* The `tex/` directory is intended for TeX sources and their related materials. Automatic PDF generation is performed only for `.tex` files located directly in the root of `tex/`; `.tex` files inside nested subdirectories are not processed automatically by default. The generated PDF artifacts are placed into `assets/showcase/` and added to `assets/showcase/index.tsv` automatically. The `tex/` directory may also contain any materials required for successful generation, including styles, bibliography sources, images, and other supporting files; its internal structure is otherwise left unrestricted.

