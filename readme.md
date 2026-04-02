![Logo](assets/brand/logo.svg)

# MANIFESTO

## 0. Tree and Blocks

This section fixes the structure of the project: from source code to artifact, peripheral, service, and auto-generated blocks. If a block is present in the repository, its place and role are considered fixed. File names should not scream at me: wherever a name is not fixed separately, `snake_case` is used.

### Project Seed

A project must have both structure and meaning. `manifest.json` defines its formal side and stores its meta-details. `readme.md` reveals its idea and soul and must not be reduced to a dry run guide.

The `core/` and `app/` blocks are intended for placing the project's source code. The presence of at least one of them is required. Both blocks use the same internal structure: `include/`, `src/`, `tests/`, and, where appropriate, `benchmarks/`. These directories form a consistent tree. If related files are present in more than one of them, they preserve the same relative path within the directory and follow a common naming pattern.

* `include/` defines the reference layer of this structure. It is expected to contain primarily interface and template files; more specific rules for allowed extensions and related conventions are fixed in `manifest.json` and in coding style.

* `src/` contains the block's working source files, including implementation files and `entry-point` sources.

* `tests/` contains test files and is strongly encouraged for header-defined components with behaviour of their own. 

* `benchmarks/` is an optional directory for benchmark files and is introduced only where performance measurements are actually justified.

For files in `tests/` and `benchmarks/`, the same general principle is used: the base name is preserved, while the layer type is expressed by a suffix. The suffix for `tests/` is `_tests`; the suffix for `benchmarks/` is `_benchmarks`. If a source file becomes too large, its `.cpp` part may be split into a small number of logically grouped batches indexed from `0` to `9`; if there is only one such batch, the `0` index is preferably omitted.


The distinction between `core/` and `app/` lies primarily at the `entry-point` level. The `core/` block is intended for a CLI, so each of its `entry-point`s must be accompanied by a `readme.md`. The `app/` block is intended for a windowed interface, so a separate `readme.md` inside it is generally not required. In all other respects, both blocks follow the same structure. Dependencies between these blocks are asymmetric: `app/` may depend on `core/`, whereas `core/` must remain independent of `app/`.

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

## 1. Facade and Modes/Surfaces

A project must be able to present itself in a minimally sufficient form. Not as a showcase, not as a promise, and not as the full internal kitchen, but as a subset already sufficient for building, running, and demonstrating its primary functionality. This subset is hereafter called the `facade`.

The introduction of the `facade` also clarifies that not everyone approaches the project in the same role. A `user` is someone who interacts with the project through ready-made artifacts and does not enter the repository at all. A `visitor` is someone who clones the repository and uses the `facade` path in order to build and run the project, but does not enter development mode. A `developer` is someone who works in development mode, whether or not they modify the code directly; entering the fuller build, diagnostic, and control flows is already sufficient for that role.

The `facade` is not intended for the `user`, who is expected to meet the project through releases and runtime artifacts. It is intended for the `visitor`: the one who wants a short and honest entry into the project without accepting its full internal burden in advance. The `developer`, by contrast, steps beyond that boundary and takes responsibility for the richer and stricter machinery of the project.

The `facade` does not replace the project and does not reduce it to a decorative shell. It fixes a mode of entry in which the project can already be built, run, and seen in action without forcing the `visitor` into its inner layers too early. `readme.md` may reveal the project's idea, purpose, and soul; the `facade` must make it possible to verify that the project is alive.

From this follows a simple requirement: entry through the `facade` must be short, stable, and uniform from project to project. Building and running the MVP must not require extra flags, manual profile selection, reading service files, or deciphering someone else's build magic. In the normal case, two or three commands should be enough.

A canonical `facade` path is therefore fixed as the following command sequence:

```bash
cmake -S . -B build
cmake --build build
./build/mvp
```

A `visitor` who came not to inspect the internals but simply to build and run the project should not be forced to deal with `.clang-tidy`, `.clang-format`, `Doxyfile`, private toolchain decisions, CI-related scaffolding, or any other service noise.

The `facade`, however, covers only the minimally sufficient form of the project. Everything that goes beyond the MVP — the full set of artifacts, extended configurations, warnings, diagnostics, additional checks, reports, and similar machinery — belongs not to the facade but to the development mode. That mode may be richer, stricter, and noisier; this is normal. What matters is that such noise remains justified and contained: it must not break the short entry path through the facade, and it must not impose its own demands where only a minimal working run is needed.

That difference becomes concrete in the build environment itself. The facade relies on a slim generated build surface and keeps the entry path short. Development mode, by contrast, requires fuller generated `CMakeLists.txt` surfaces that shape a richer internal build tree: separate component-level build paths, correct internal linking between libraries and runnable artifacts, profile-specific branches, and room for diagnostics, checks, and related service outputs. This additional structure exists to keep the project organized, not to excuse clutter. The earlier rule against turning the repository into some rubbish dump still applies here as well, but now at the level of the build environment itself.

For that reason, development mode does not begin from handwritten local `CMake` logic or from a naive manual command sequence. Under MANIFESTO, it is entered through the project’s generated tooling surface, which materializes the fuller development environment and keeps it coherent across different contexts. Once that surface has been materialized, manual terminal steps and IDE-driven workflows may still interact with it, inspect it, and in some cases continue parts of it. What remains fixed is the point of entry: the development surface is tooling-owned rather than handwritten.

That surface does not belong to the tracked repository state in the same sense as the facade-facing one. It may be materialized locally on a developer’s machine or temporarily inside CI, and in both cases it belongs to the active environment rather than to the committed project tree as such. Some of the artifacts produced there may still be valuable in their own right — including reports, documentation, and runnable outputs — but the surface that produces them remains generated and environment-bound rather than authoritative as editable project state.

The same applies to verification flows. Local checks and GitHub-side automation must not become two separate truths that drift apart over time. They are expected to continue the same project logic across different environments: one closer to the developer’s machine, the other closer to the repository’s public control surface. Their concrete commands and actors may be introduced later, but their unity of intent belongs here.

The facade and the development mode therefore do not describe two different projects. They describe two different depths of entry into the same one. They may differ sharply in noise level, strictness, and internal machinery, yet they are still expected to coexist within the same `build/` directory and to remain non-conflicting even when each path is designed to stand on its own.

## 2. Code and Styles

This section does not treat style as a cosmetic layer. Style includes naming, file placement, directory structure, and the local shape of code, because these choices determine whether ambiguity, review cost, and structural drift are reduced or merely postponed. These conventions are fixed by shared policy rather than left to per-project taste.

The primary structural unit of the source tree is the `referent`: a header file under `include/` that defines a named entity or a coherent group of entities and serves as the point of reference for related files.

Files in `src/`, companion `.tpp` files, tests in `tests/`, benchmarks in `benchmarks/`, and other permitted companion layers are interpreted in relation to a `referent`. They do not form independent primary units: they belong to a `referent`, extend it, and preserve an explicit connection to it through path, name, or role. The only exception is formed by `entry-point` files containing `main()`: they are not `referent`s and are not required to implement anything else.

For the purposes of this section, the meaningful source tree is treated as a `referent-tree`: its terminal nodes are `referent`s, while directories that organize them are treated as `module`s. A directory qualifies as a `module` if and only if its subtree contains at least one descendant `referent`; a `module` may therefore contain `referent`s directly, nested `module`s, or both. Subtrees that contain no `referent` are ignored when the structural shape of the code is evaluated.

The depth and width of the `referent-tree` must be justified by the shape of the `referent` surface. Directories are introduced not for decorative nesting and not for the mechanical redistribution of files, but to express real semantic boundaries between parts of the code. A flat storage heap is no better than a chain of directories whose only job is to pretend that structure exists.

For that reason, the `referent-tree` may be evaluated through balance-oriented diagnostic measures that expose excessive flatness, unnecessary depth, and weakly justified branching. These measures are diagnostic rather than absolute: their purpose is not to force artificial symmetry, but to make suspicious structural shapes visible.

To make such measures precise, let `T` denote the reduced `referent-tree`, obtained from the source tree by excluding every subtree that contains no `referent`, so that only structurally meaningful code organization remains. Let `M(T)` denote the set of all `module`s in `T`.

For each node `v` in `T`, let `R(v)` denote the number of `referent` nodes in the subtree rooted at `v`, let `B(v)` denote the number of direct children of `v` in `T`, and let `H(v)` denote the maximum depth from `v` to a descendant `referent`, measured in edges.

For a `module` node `v` with direct children $c_1, \ldots, c_{B(v)}$, define the share of each child in the descendant `referent` volume of `v` by

$$
p_i = \frac{R(c_i)}{R(v)}.
$$

These shares make it possible to evaluate the local branching shape of a `module` in a way that naturally accommodates non-binary branching [ref-colless-like].

The first local measure is therefore a branching-imbalance term:

$$
I(v)=
\begin{cases}
0, & B(v)\le 1, \\
1-\dfrac{-\sum_{i=1}^{B(v)} p_i \log p_i}{\log B(v)}, & B(v)>1.
\end{cases}
$$

This term is minimal when the descendant `referent`s are distributed as evenly as possible across the immediate children of `v`, and increases as the distribution becomes more skewed. In that sense, it uses a normalized entropy signal to penalize weakly justified branching [ref-entropy-balance].

The second local measure captures excess depth. It follows the general idea that a balanced tree should not allow some leaves to drift much farther away than others without structural justification [ref-balanced-tree]. Let

$$
H_{\mathrm{ideal}}(v)=\left\lceil \log_{\max(2,B(v))} R(v) \right\rceil
$$

be the soft ideal depth of the subtree rooted at `v`, and define

$$
D(v)=
\max\left(
0,
\frac{H(v)-H_{\mathrm{ideal}}(v)}
{\max(1,H_{\mathrm{ideal}}(v))}
\right).
$$

This term remains zero while the subtree depth is broadly consistent with its branching profile and descendant `referent` volume, and grows once the subtree begins to stretch into chains of directories whose structural contribution is weak.

The third local measure captures width overload. Since a structurally flat dumping ground is no more desirable than gratuitous nesting, define the soft width target

$$
B_{\mathrm{soft}}(v)=\left\lceil \sqrt{R(v)} \right\rceil
$$

and the corresponding width penalty

$$
W(v)=
\max\left(
0,
\frac{B(v)-B_{\mathrm{soft}}(v)}
{\max(1,B_{\mathrm{soft}}(v))}
\right).
$$

This term is not presented as a standard balance index, but is introduced here as a corrective signal against overly wide and weakly organized `module`s.

These three terms may also be combined into a single local structural hint score,

$$
L(v)=0.50 I(v)+0.35 D(v)+0.15 W(v).
$$

Here, branching imbalance is treated as the strongest signal, excess depth as the second, and width overload as the third. A global tree-level score may in turn be obtained by aggregating `L(v)` over all `module`s in `M(T)` with weights derived from descendant `referent` volume. Such a score should be treated as diagnostic rather than absolute: its purpose is to expose suspicious shapes in the tree, not to force semantically justified structure into artificial symmetry.

## 3. `manifest.json`

## 4. Marx and Engels
