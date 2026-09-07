![Logo](assets/brand/logo.svg)

# MANIFESTO

MANIFESTO defines the structure and working conventions of managed C++ projects. Companion repositories built around other ecosystems are not required to reproduce this topology.

## 0. Tree and Blocks

This section fixes the structure of the project: from source code to artifact, peripheral, service, and auto-generated blocks. If a block is present in the repository, its place and role are considered fixed. File names should not scream at me: wherever a name is not fixed separately, `snake_case` is used.

### Project Seed

A project must have both structure and meaning. `manifest.json` defines its formal side and stores its meta-details. `readme.md` reveals its idea and soul and must not be reduced to a dry run guide.

The `core/` and `app/` blocks contain the project’s source code. The presence of at least one of them is required. Both blocks use the same internal structure: `include/`, `src/`, `tests/`, and, where appropriate, `benchmarks/`. These directories form a consistent tree. If related files are present in more than one of them, they preserve the same relative path within the directory and follow a common naming pattern.

* `include/` defines the reference layer of this structure and contains primarily interface and template files.

* `src/` contains the block’s working source files, including implementation files and `entry-point` sources.

* `tests/` contains test files. Independently observable behaviour declared in `include/` belongs under test.

* `benchmarks/` is an optional directory for benchmark files and is introduced only where performance measurements are actually justified.

For files in `tests/` and `benchmarks/`, the same general principle is used: the base name is preserved, while the layer type is expressed by a suffix. The suffix for `tests/` is `_tests`; the suffix for `benchmarks/` is `_benchmarks`. If a test or benchmark `.cpp` grows too large, it may be split into logically grouped batches whose indices form a contiguous sequence beginning at `0` and ending no later than `9`. The index is omitted when no split is needed; a split test may therefore use names such as `foo_tests_0.cpp`, `foo_tests_1.cpp`, and `foo_tests_2.cpp`.

The distinction between `core/` and `app/` is semantic rather than structural. The `core/` block contains non-windowed project code. Code in `core/` may form a reusable library surface, including use outside the repository, while its `entry-point`s provide whatever runnable surface is appropriate to the project stage – for example a CLI, an executable example, diagnostics, or another compact interface. The `app/` block is intended for a windowed interface. Dependencies between these blocks are asymmetric: `app/` may depend on `core/`, whereas `core/` must remain independent of `app/`.

The tree does not require block-local operational README files. Runnable surfaces expose usage through their own help or manual interface, while reusable APIs are documented from their declarations.

### Asset Topology

The `assets/` directory has been designed to serve as the project’s artifact layer. It is not intended for arbitrary files and must not be used as some rubbish dump: the contents of `assets/` are grouped by purpose through a fixed first-level directory layout.

Within the project’s working environment, this directory is also treated as a source for copying or linking into `build/`, `.manifest/`, and other service environments; `assets/showcase/` is excluded from this rule, as it is oriented primarily toward documentation and project presentation.

* The `assets/brand/` directory is intended for the project’s visual identity. The preferred format here is `svg`, since it preserves the possibility of meaningful manual editing while remaining readable as code. This directory is also the home of `favicon.ico`.

* The `assets/showcase/` directory is intended for stable project artifacts: screenshots, screen recordings, demo materials, PDF files, and other media. Such files may be used, for example, in documentation and `readme.md`.

The contents of `showcase/` are treated primarily as artifacts in their own right rather than as line-oriented files meant for manual editing.

* The `assets/reports/` directory is intended for reports and run results that are deliberately retained as project artifacts. Its contents are generated automatically and may remain informative not only on their own, but also when compared across versions. The presence of `assets/reports/` can be fixed conveniently via `assets/reports/.gitkeep`. Files inside `reports/` use a suffix-based naming scheme: the current version is marked with the `--latest` suffix, while versioned snapshots use a suffix of the form `--YYYYMMDD-HHMMSS`.

* The `assets/dataset/` directory is intended for input data sets used in tests, examples, and related validation scenarios. The naming at the first level of nesting is fixed: `test/`, `train/`, `stress/`, and `todo/`. These branches are optional in general, but if `dataset/` is not empty, `test/` must be present. Below that layer, the structure is left unconstrained: both a flat file layout and arbitrary further subdivision into subdirectories are permitted.

* The `assets/dumps/` directory is intended for auto-generated outputs, including results produced from runs over data sets in `dataset/`. The first level of nesting in `dumps/` must mirror the first level of `dataset/`; in other words, `dumps/test/`, `dumps/train/`, `dumps/stress/`, and `dumps/todo/` are only valid as reflections of the corresponding input branches. Below that layer, the structure remains unrestricted.

At the same time, an explicit escape hatch is reserved here: despite the strict structural rule, `assets/.gitignore` may still ignore any subset of `dumps/`, including first-level directories.

* The `assets/sets/` directory is intended for artifacts that the application uses and renders at runtime, together with the basic metadata and settings associated with them. These may include themes, portable serialized state representations, configuration sets, and other runtime artifacts that the program treats as ready-made input entities.

* The `assets/templates/` directory is intended for file-based templates owned and used by the project when generating data, text, or other derived artifacts. The general rule here is simple: long template strings live in dedicated files rather than directly in code. The internal structure of `templates/` is left unconstrained and may branch arbitrarily if that helps organize the templates themselves.

Within any first-level directory inside `assets/`, the meta-pair `index.tsv` and `readme.md` may be used whenever an ordered listing of media or runtime artifacts is needed. The `index.tsv` file defines the subset of files that participates in a carousel, listing, or any other derived representation; the row order in the table also defines the display order. The table uses the columns `id`, `path`, `type`, `description`, and `datetime`.

In the `path` column, the file is referenced by its relative path from the directory that contains `index.tsv`; for example, if the table is located at `assets/dataset/index.tsv` and the file is located at `assets/dataset/test/some_dir/example1.ex`, then the value stored in `path` is `test/some_dir/example1.ex`.

If `readme.md` is present next to `index.tsv`, it must contain a `<carousel/>` block. If `readme.md` is absent, this is treated as if a virtual `readme.md` existed next to `index.tsv` and contained nothing but that block. Carousels and similar auxiliary documentation pages are generated automatically from this pair of files. Supported artifact types include images, videos, and PDF files.

The `index.tsv` file must not be edited manually and is generated only through the controlling CLI tool.

TODO: project documents — decide whether temporary requirements, architecture notes, and other formal project documents receive a conventional `assets/docs/` home, and what should remain there once the corresponding requirements have been absorbed into code, tests, and generated API documentation.

### Generated and Service Blocks

The files `license`, `citation.cff`, and `CMakeLists.txt`, together with the `.github/` directory, form a related group of auto-generated blocks whose contents are derived from templates with data substituted from `manifest.json`. The overall structure of these blocks remains predictable, while their final contents are determined by the description of the particular project and may therefore differ from one repository to another.

These blocks are derived artifacts, not independent points of configuration, and are not edited manually. See the section on `manifest.json` for details.

A project may contain up to three `.gitignore` files. The root `.gitignore` is common to all projects and is generated automatically. A second `.gitignore` may appear in `bindings/`; it is auto-extended according to the languages and toolchains actually used there. The third may appear in `assets/`, and this is the only `.gitignore` that may be edited manually.

Even then, it is used exclusively for filtering subsets of `assets/dumps/`, so that the repository keeps the right balance: first and foremost, the project is a source-code repository, not a storage site for automatically generated dumps, however useful those dumps may sometimes be as examples or reference artifacts. This exception does not make `assets/` a rubbish dump.

The files `.clang-tidy` and `.clang-format` form a related pair of auto-generated root-level templates. Like the root `.gitignore`, they belong to the shared project layout rather than to the project-specific layer derived from `manifest.json`, so their contents remain the same across repositories. Together they define the common linting and formatting baseline used by the CI workflows described in `.github/`.

Both files are covered by `.gitignore` and are not committed, while remaining part of the generated project layout.

When present, the `build/` and `.manifest/` directories form a related pair of local service workspaces at the project root and remain covered by `.gitignore`. Unlike blocks generated directly from fixed templates, these directories are shaped primarily by the actual execution of build and control flows.

They contain intermediate states, caches, debug and diagnostic output, generated artifacts, and other transient by-products as required by active workflows. Tooling surfaces used only for development, analysis, documentation, automation, or CI – including fuller build descriptions and generated documentation configuration – are materialized there when needed. Such files belong to the active environment rather than the meaningful source tree; the tooling that creates them defines their concrete ownership and entry paths.

### Peripheral Blocks

These blocks are not mandatory, yet whenever they appear they become integral parts of the project and occupy their fixed places in the tree.

* The `bindings/` directory is intended for integrations and wrapper layers around the main project for other languages and external ecosystems. Its first level of nesting is organized by target environment or language; concrete subdirectory names are fixed by the support implemented by the project.

In this context, a `java/` directory inside `bindings/` denotes the JVM block broadly rather than Java code in the narrow sense alone, so Kotlin code inside `bindings/java/` is normal.

The `bindings/` block receives automatic `.gitignore` updates based on the languages and toolchain environments used in the project; in that sense, it forms a local ecosystem of its own, somewhat less dependent on the repository’s outer structure as a whole.

* The `shim/` directory is intended for thin external shims invoked by the main code only where such separation is genuinely justified. It is not a place for user-facing scripts, development utilities, build or run wrappers, or a substitute for the facade; for that reason, the names `scripts/` and `Makefile` are forbidden for this kind of block, since they blur distinct roles.

The contents of `shim/` are not invoked manually in the ordinary workflow. Main project logic remains in C++; `shim/` contains only external integration points that genuinely require a separate shim.

* The `tex/` directory is intended for TeX sources and their related materials. Automatic PDF generation is performed only for `.tex` files located directly in the root of `tex/`; `.tex` files inside nested subdirectories are not processed automatically by default. The generated PDF artifacts are placed into `assets/showcase/` and added to `assets/showcase/index.tsv` automatically.

The `tex/` directory may also contain any materials required for successful generation, including styles, bibliography sources, images, and other supporting files; its internal structure is otherwise left unrestricted.

## 1. Facade and Modes

A project must be able to present itself through a short and predictable build surface. The `facade` is the project’s promise to a `visitor`: after cloning the repository, the canonical three-command path produces a runnable `mvp` without requiring the visitor to learn the development machinery first.

The facade also clarifies the roles from which a project is approached. A `user` interacts with ready-made artifacts and does not enter the repository. A `visitor` clones the repository and follows the facade path, but does not enter development mode. A `developer` enters the fuller build, diagnostic, and control surface, whether or not they are currently modifying source code.

`readme.md` reveals the project’s idea, purpose, and soul; the facade verifies that the project is alive.

It narrows what the generated build surface materializes, not what the repository contains. Source code, tests, benchmarks, assets, and other project material remain present, and the developer does not distort natural project architecture merely to make the facade smaller.

The canonical facade path is fixed as:

```bash
cmake -S . -B build
cmake --build build
./build/mvp
```

The `mvp` is the visitor-facing runnable artifact. It guarantees a meaningful run while hiding the project’s internal artifact kinds and canonical artifact names from the visitor contract.

A visitor following this path does not deal with `.clang-tidy`, `.clang-format`, `Doxyfile`, private toolchain decisions, CI scaffolding, extended profiles, or other service noise.

Facade minimality is measured by visitor burden rather than by the smallest possible number of compiled targets. Its generated `CMakeLists.txt` remains short and readable, and the facade never justifies making natural dependencies optional or introducing extra abstraction solely to exclude them.

The facade is generated project state and is regenerated rather than authored. If an unsupported environment requires a small local adaptation, a visitor may patch the generated facade locally; environment-specific preferences do not become committed project policy, while defects in the generated facade belong in MANIFESTO itself.

Development mode continues from the same project at greater depth. It materializes tests, benchmarks, extended configurations, diagnostics, reports, documentation tooling, additional checks, and other service surfaces as required. The development surface is tooling-owned, exists locally or temporarily in CI, and remains environment-bound rather than authoritative as editable project state.

Once materialized, it remains inspectable and usable through ordinary terminal, IDE, and build tools; it does not require project-specific handwritten wrappers or a second handwritten CMake truth.

Local verification and GitHub-side automation apply the same project policy rather than separate workflows with separate meaning. Their generated files and build trees remain discoverable inside the project’s service workspaces, while the exact internal layout of those workspaces remains an implementation detail rather than part of the facade contract.

## 2. Code and Styles

Style is not treated here as a cosmetic layer. It includes naming, source placement, decomposition, control flow, documentation, and the local shape of code because these choices determine how easily the project can be read, navigated, tested, and changed.

The rules in this section define the clean state of project code. Violations produce diagnostics rather than build prohibitions. A diagnostic records a deviation from MANIFESTO; it never prescribes a mechanical remedy. File splitting, new directories, helper extraction, or new abstractions are justified only when they clarify responsibility, locality, or behaviour – never merely because they make one metric smaller.

### 2.1 Referents and Modules

The primary structural unit of the source tree is the `referent`: a header file under `include/` that defines a named entity or a coherent group of entities and serves as the point of reference for related files.

Files in `src/`, companion `.tpp` files, tests in `tests/`, and benchmarks in `benchmarks/` are interpreted in relation to a `referent`. Within `core/` and `app/`, every project-owned C++ source unit is therefore a `referent`, a companion of a `referent`, or an `entry-point` containing `main()`.

The meaningful source tree is treated as a `referent-tree`: its terminal nodes are `referent`s, while directories that organize them are `module`s. A directory qualifies as a `module` if and only if its subtree contains at least one descendant `referent`; a `module` may contain `referent`s directly, nested `module`s, or both. Subtrees that contain no `referent` are ignored when source structure is evaluated.

Directories express semantic grouping rather than file-count management. A flat heap of similarly named or prefixed files is undesirable for the same reason as an artificial chain of one-child directories: both increase navigation cost without clarifying responsibility.

TODO: placement — reconsider whether the structural mathematics belongs closer to Tree and Blocks once `referent` and `module` can be introduced there without front-loading the document.

### 2.2 Structural Shape

Let `T` denote the reduced `referent-tree`, obtained by excluding every subtree that contains no `referent`, and let `M(T)` denote the set of all `module`s in `T`.

For a node `v` in `T`, let `R(v)` denote the number of descendant `referent` nodes in the subtree rooted at `v`, let `B(v)` denote the number of direct children of `v` in `T`, and let `H(v)` denote the maximum depth from `v` to a descendant `referent`, measured in edges.

For a `module` `v` with direct children $c_1, \ldots, c_{B(v)}$, define

```math
p_i = \frac{R(c_i)}{R(v)}.
```

The local referent-count imbalance is zero for $B(v)\le1$:

```math
I_R(v)=0.
```

For $B(v)>1$, it is

```math
I_R(v)=1+\sum_{i=1}^{B(v)} p_i \log_{B(v)} p_i.
```

This is the complement of a normalized Shannon-entropy balance signal: it is minimal when descendant `referent`s are distributed evenly across the immediate children and increases as responsibility becomes concentrated in fewer branches [ref-entropy-balance]. The same general balance problem for multifurcating trees is also addressed by Colless-like indices [ref-colless-like].

The soft ideal depth is

```math
H_{\mathrm{ideal}}(v)=\max(1,\lceil \log_{\max(2,B(v))}R(v)\rceil).
```

The lower bound of one follows from measuring depth in edges: a `module` with one direct `referent` already has depth one.

For a measured value `x` and positive soft target `t`, define the relative excess

```math
E(x,t)=\max(0,\frac{x-t}{t}).
```

Depth excess is

```math
D(v)=E(H(v),H_{\mathrm{ideal}}(v)).
```

To detect excessive flatness, define

```math
B_{\mathrm{soft}}(v)=\lceil\sqrt{R(v)}\rceil,
```

and

```math
W(v)=E(B(v),B_{\mathrm{soft}}(v)).
```

The soft width of a module is defined by the square-root rule. It makes increasingly wide flat growth visible without replacing semantic decomposition with a fixed directory-size constant. Software-architecture literature independently treats decomposition and component balance as analyzability concerns [ref-analyzability].

The local structural hint is

```math
L(v)=\frac{1}{2}I_R(v)+\frac{1}{3}D(v)+\frac{1}{6}W(v).
```

The `3:2:1` weighting gives referent imbalance precedence over depth and width while keeping all three structural effects visible. `L(v)` is a diagnostic hint, not a normalized quality score. Tree-level reporting aggregates local hints without replacing the local causes they describe.

### 2.3 Volume and Navigation

Tree shape cannot reveal an oversized terminal `referent`, so textual volume is measured independently from topology.

For a source file `f`, let $P(f)$ denote its physical line count, including blank and comment-only lines, and let $S(f)$ denote its effective source line count, excluding them. $P(f)$ measures navigation span; $S(f)$ measures implementation concentration. Navigation and spatial orientation are comprehension costs rather than cosmetic concerns [ref-navigation] [ref-spatial-navigation].

For a `referent` $r$, let $`\mathcal{P}(r)`$ denote its production files: its header and production companions, excluding tests and benchmarks. Define

```math
P(r)=\sum_{f\in\mathcal{P}(r)}P(f), \qquad S(r)=\sum_{f\in\mathcal{P}(r)}S(f).
```

For a `module` $v$, let $`\mathcal{R}(v)`$ denote its descendant `referent`s and define

```math
S(v)=\sum_{r\in\mathcal{R}(v)}S(r).
```

For the direct children $c_i$ of $v$, define

```math
q_i=\frac{S(c_i)}{S(v)}.
```

The corresponding implementation-volume imbalance is zero when $B(v)\le1$ or $S(v)=0$:

```math
I_S(v)=0.
```

Otherwise,

```math
I_S(v)=1+\sum_{i=1}^{B(v)} q_i \log_{B(v)}q_i,
```

with the usual convention $0\log 0=0$.

`I_R(v)` and `I_S(v)` remain independent. The first measures imbalance in the distribution of named responsibilities; the second measures implementation-volume concentration even when referent counts are balanced. Architectural analyzability work similarly separates decomposition from distribution of implementation volume [ref-analyzability].

File-size bounds are derived from representative C++ corpora rather than a universal average because source-file size distributions are strongly skewed. `core/` and `app/` use separate reference corpora so framework-heavy application code is not judged by the distribution of non-windowed code [ref-file-size] [ref-metric-thresholds].

Let

```math
b(f)\in\{\mathrm{core},\mathrm{app}\}, \qquad k(f)\in\{\mathrm{header},\mathrm{source}\}.
```

identify the source block and file role. For a representative benchmark corpus $`\mathcal{B}`$, warning bounds are expressed by quantiles:

```math
T^{P}_{b,k}(q)=Q_q(\{P(f)\mid f\in\mathcal{B}_{b,k}\}),
```

and

```math
T^{S}_{b,k}(q)=Q_q(\{S(f)\mid f\in\mathcal{B}_{b,k}\}).
```

Ordinary and stronger warnings use distinct quantiles. Separate corpora give `app/` its own reference distribution rather than an arbitrary multiplier.

TODO: file-volume bounds — choose the ordinary and strong warning quantiles; define corpus construction so one large project cannot dominate by file count; maintain separate `core/`, Qt-oriented `app/`, and other relevant C++ reference corpora.

### 2.4 Functions and Complexity

Function span, control-flow complexity, and nesting describe different reading costs and remain independently visible.

For a function `g`, let `P(g)` denote the physical line span of its definition. The soft span target is

```math
T_P^{\mathrm{function}}=60,
```

using the C++ Core Guidelines’ practical one-editor-screen bound as the reference [ref-cpp-guidelines]. The corresponding relative excess is

```math
F_P(g)=E(P(g),60).
```

Cyclomatic complexity follows McCabe. For a control-flow graph `G` with `e` edges, `n` nodes, and `p` connected components,

```math
\mu(G)=e-n+2p.
```

For the connected control-flow graph of one function,

```math
\mu(g)=e_g-n_g+2.
```

The soft complexity target is

```math
T_\mu^{\mathrm{function}}=10,
```

and

```math
F_\mu(g)=E(\mu(g),10).
```

These diagnostics remain separate: a long linear function and a short branch-heavy function are different problems. Findings on the same entity are presented together without collapsing their causes into a single verdict; combinations of distinct metrics are more informative than one measurement alone [ref-mccabe] [ref-cpp-guidelines] [ref-metric-combination].

TODO: function nesting — define $N(g)$ as maximum control-flow nesting depth, select a C++-appropriate warning target, and test whether it adds useful signal beyond cyclomatic complexity rather than merely duplicating it.

### 2.5 Naming

Project-owned file and directory names remain `snake_case`. Ordinary project-owned C++ identifiers use `snake_case`; leading or trailing underscores and scope prefixes such as `m_` produce naming diagnostics rather than define a second naming system. External declarations owned by dependencies are outside this policy.

Names are concise as well as meaningful. The checker does not reward verbosity and imposes no minimum word count: conventional short names such as `lhs`, `rhs`, `id`, `x`, or `y` are not expanded for style machinery, and no natural-language corpus is used to judge whether a short name is descriptive enough.

For a `snake_case` identifier $n$, let $W(n)$ denote the number of underscore-separated segments and $C(n)$ its character count.

For ordinary project identifiers, the upper warning bounds are

```math
W(n)>4, \qquad C(n)>25.
```

These bounds define mechanical limits on naming verbosity and are supported by identifier-naming research [ref-identifiers] [ref-identifier-guidelines]. Test names naturally encode the subject together with scenario or expectation, so their word-count bound is

```math
W_{\mathrm{test}}(n)>7.
```

No lower word or character bound is defined, and the checker does not infer identifier meaning from natural language.

TODO: naming exceptions — calibrate the test-name character bound; decide the sparse exception model for code symbols such as macros and generated include guards; keep file and directory naming independent of those code-level exceptions.

### 2.6 Locality and Namespaces

Ordinary project logic has explicit ownership in the `referent` structure and does not accumulate as hidden translation-unit-local mass. A useful helper may belong to an existing referent and does not require its own file or abstraction, but substantial named behaviour has an explicit declaration and a deliberate place in the source tree.

Every anonymous namespace produces one locality diagnostic. For an anonymous namespace `u`, the checker reports

```math
A_{\mathrm{anon}}(u)=1,
```

along with its declaration count and effective line count instead of emitting a separate warning for every declaration inside it. LLVM documents the same locality-of-reference problem: a reader may need to search far above a declaration to discover that it is hidden inside an anonymous namespace [ref-llvm-style].

The diagnostic is non-blocking, but a diagnostically clean project stage contains no anonymous namespaces. Any anonymous namespace is diagnostic debt rather than a second accepted storage model for project logic.

### 2.7 Control Flow and Lambdas

All control-flow bodies use braces, including single-statement bodies. Missing braces produce a style diagnostic; the goal is stable syntactic shape under later edits rather than compactness for its own sake [ref-cert-braces].

Lambdas are held to tighter limits than ordinary named functions because their main value is short, local behaviour. For a lambda `l`, reuse its physical span `P(l)` and cyclomatic complexity $`\mu(l)`$, and let $N(l)$ denote maximum nested control-flow depth inside the lambda body.

The warning targets are

```math
T_P^{\lambda}=5,\qquad T_\mu^{\lambda}=2,\qquad T_N^{\lambda}=1.
```

A threshold excess yields the corresponding lambda diagnostic; several excesses are presented together as unnamed behavioural complexity. The span target follows the small-function range in the C++ Core Guidelines, while the stricter complexity and nesting limits keep unnamed behaviour simpler than named functions [ref-cpp-guidelines].

### 2.8 Conditional Compilation

Platform, backend, provider, and environment differences are expressed structurally rather than scattered through shared source files as deep conditional-compilation trees. Compact local conditional branches remain acceptable where introducing a separate abstraction would reduce locality and clarity.

For a source file `f`, let $`N_{\mathrm{pp}}(f)`$ denote the maximum nesting depth of preprocessor conditionals.

The warning model is

```math
N_{\mathrm{pp}}(f)>2
```

for an ordinary warning and

```math
N_{\mathrm{pp}}(f)>3
```

for a stronger warning. Developer studies of the C preprocessor report broad discomfort with deeper nesting and comprehension problems around conditional-compilation structure [ref-preprocessor]. These diagnostics constrain nesting, not the refactoring strategy; an abstraction introduced only to silence the checker is itself a worse structural result.

### 2.9 Documentation

Documentation belongs to a mature interface and follows interface stability. Before the interface stabilizes, requirements, architecture, naming, and tests take precedence over prematurely polished API prose.

For mature project-owned code, named declarations in headers are the primary documentation surface and support Doxygen extraction. Documentation describes contracts, intent, assumptions, and non-obvious behaviour rather than restating syntax. Implementation comments remain secondary and explain only information that cannot be expressed clearly through structure, naming, declarations, or tests.

Generated documentation configuration is tooling material rather than handwritten project policy; `Doxyfile` and related surfaces are materialized in the service workspace when needed.

TODO: documentation maturity — define when missing Doxygen coverage begins to generate diagnostics; decide the lifecycle of temporary project documents and the boundary between API documentation, retained `assets/` artifacts, and generated Pages output.

TODO: text hygiene — reconsider the older ASCII-only source/Markdown and English-only comment conventions before promoting either into current policy.

### 2.10 Diagnostic Cleanliness

Let $`\mathcal{W}(P)`$ denote the active set of style and structural diagnostics for a project snapshot $P$, and let

```math
N(P)=|\mathcal{W}(P)|.
```

A project stage with $N(P)=0$ is `diagnostically clean`. Every active diagnostic is diagnostic debt. Diagnostics do not block building, running, or releasing the project; they state exactly where the current project snapshot deviates from MANIFESTO.

Warnings differ in category and severity, but severity describes the strength or degree of the deviation rather than changing it into a build prohibition. Reports preserve independent causes, merge related findings on the same entity when that improves explanation, and remain filterable by category and severity. A diagnostic explains what is wrong, where it is wrong, and which measurements produced that conclusion; the bibliography supports the policy rather than replacing the explanation.

Policy precedes tooling. Incomplete automatic detection does not weaken a rule, and the existence of a checker does not define the rule.

## 3. manifest.json

* TODO: source of truth — define the manifest as the compact editable description of project intent, while keeping generated mechanics, tool flags, procedural CMake, warning presets, and other implementation detail outside it.

* TODO: atomic model — identify the smallest stable atoms only after sections 0–2 and Marx/Engels semantics are settled; do not revive the old `component`, `modules`, or `file_units` model where the `referent` topology already determines structure.

* TODO: project identity — define the minimal project metadata and schema/version information that genuinely belongs in every manifest.

* TODO: artifacts — describe intended build artifacts without reproducing a handwritten CMake target graph; distinguish internal artifact references from external project dependencies.

* TODO: facade/MVP — record which project artifact supplies the visitor-facing `mvp` while keeping `mvp` itself a facade contract rather than the canonical identity of that artifact.

* TODO: tests and benchmarks — express required support and intent, leaving concrete target emission and discovery of referent companions to generated tooling.

* TODO: dependencies — model dependencies on other MANIFESTO-managed repositories as separate source projects that build and install independently, then enter the consumer through normal package/library semantics rather than a shared CMake target graph.

* TODO: dependency resolution — separate update policy from resolved state: `manifest.json` describes the source/channel, while a lock surface records the exact resolved commit; decide the final name and scope of that lock surface.

* TODO: dependency lifecycle — define when resolution/update occurs versus when an already resolved world is merely built, while preserving a single mutable local realization rather than a warehouse of version directories.

* TODO: package scope — keep requirements local to the smallest artifact/referent closure that needs them rather than promoting every dependency to project-global state.

* TODO: paths and ownership — require manifest paths to remain project-relative and incapable of escaping the project ownership boundary.

* TODO: code-style exceptions — allow sparse explicit exceptions such as macro naming conventions while keeping shared defaults implicit; exceptions should describe deviations, not restate defaults.

* TODO: assets — add manifest-level asset policy only where generated staging or selection actually requires project-specific intent; do not duplicate the fixed asset topology.

* TODO: editable/generated boundary — state which tracked surfaces are derived from the manifest and which local surfaces are disposable environment state, without making generated implementation detail part of the manifest schema.

* TODO: validation — require the complete manifest and its dependency/reference graph to validate before generation or other state-changing work begins.

## 4. Marx and Engels

* TODO: two actors only — remove MANIFESTO as a runtime actor; the project keeps the document/repository name, while the development interface is divided between Marx and Engels.

* TODO: actor boundary — define actors by responsibility rather than by today’s exact command grammar: Marx owns state-changing/materializing flows; Engels owns read-only, diagnostic, interpretive, and reporting flows.

* TODO: facade of the MANIFESTO project — keep both Marx and Engels as real facade artifacts and decide whether the repository’s visitor-facing `mvp` should resolve to Engels as the more viewer-oriented surface.

* TODO: developer promise — Marx and Engels should hide generated artifact locations and service-workspace mechanics behind stable operations; developers should not need to discover and invoke generated internals manually.

* TODO: help and manuals — every CLI actor should expose complete `--help` / `help` information, guide malformed commands toward the relevant help invocation, and provide a manual or man-page surface where appropriate.

* TODO: wrong actor guidance — a request made to the wrong actor should identify the owning actor instead of failing as an unrelated parse error.

* TODO: dependency diagnostics — missing or incompatible dependencies should be explained before heavy work where possible; installation hints may be offered without turning the project into a per-platform package-manager manual.

* TODO: resolution/materialization — assign ownership for dependency resolution, lock updates, local source checkout/build/install materialization, and ordinary builds without collapsing dependent projects into one build graph.

* TODO: sync/materialization — define the operation that regenerates tracked derived surfaces and the operation(s) that materialize local developer-only surfaces; generated state remains derived rather than an authoring mode.

* TODO: mutation safety — model mutations through validated project state, reject impossible requests before side effects, avoid rewriting unrelated structure, and preserve the previous valid state when a state-changing operation fails where practical.

* TODO: workspace validity — workspace-wide operations must not silently omit invalid managed projects; invalid state should remain visible and attributable.

* TODO: diagnostics — expose the section-2 warning categories, severity levels, raw measurements, merged findings, and filters without turning style diagnostics into build gates.

* TODO: reports — keep a canonical machine-readable representation of diagnostics/reports, with human-readable terminal, CI, and Pages presentations derived from the same information.

* TODO: CI unity — local development checks and GitHub Actions must invoke the same project policy through Marx/Engels rather than maintain a parallel CI-only truth.

* TODO: documentation/Pages — define how generated Doxygen, retained reports/showcase artifacts, and other public documentation are materialized for GitHub Pages without introducing a separate web-project topology into MANIFESTO-managed C++ repositories.

* TODO: CI enforcement — distinguish non-blocking style diagnostics from genuinely failed tests/builds/required checks, and decide where advisory versus gating CI policy belongs.

* TODO: public grammar — only after actor responsibilities stabilize, decide which command names, selectors, profiles, exit/error classes, and report filters are stable public contract and which remain implementation detail.
