# Setlist Loudness Balancer

## Product Requirements Document (PRD)

## 1. Document Control

**Project Name:** Setlist Loudness Balancer
**Project Type:** Open-source desktop application
**Primary Languages:** C++, MATLAB
**Intended Users:** Amateur live musicians, small bands, solo performers, rehearsal leaders, student performers, volunteer audio teams
**Document Purpose:** Define the requirements for an open-source, user-friendly application that analyzes rehearsal or live performance recordings and produces practical loudness-balance feedback for setlists and songs.

---

## 2. Product Summary

Setlist Loudness Balancer is an open-source application that helps amateur live musicians understand and improve the balance of vocals and instruments across rehearsals, performances, and setlists. The application processes recorded audio, estimates relative loudness by source or frequency region, identifies sections where important parts are masked, and exports analysis-ready data for deeper inspection in MATLAB.

The system is designed as a reusable, documented software package with a performant C++ analysis engine and a MATLAB analysis/reporting layer. The C++ application handles audio ingestion, preprocessing, feature extraction, segmentation, loudness estimation, and export. MATLAB scripts load the exported results, generate visual summaries, compute higher-level metrics, and support reproducible analysis.

The product should feel approachable to non-technical musicians while still being structured and documented like research-grade software.

---

## 3. Problem Statement

Amateur live musicians often struggle to evaluate mix balance from rehearsal recordings. Common problems include:

* Lead vocals getting buried during choruses
* Guitar or keyboards masking vocals in midrange-heavy sections
* Bass or kick overwhelming quieter passages
* Inconsistent loudness from song to song in a setlist
* No simple way to compare rehearsal recordings quantitatively

Most available tools are either:

* too technical for non-experts,
* designed for professional mixing engineers,
* tied to expensive digital audio workstation workflows, or
* not oriented around clear, practical feedback for live performance preparation.

Users need a simple application that can process one or more recordings and answer questions such as:

* Are the vocals consistently audible?
* Which songs are too loud or too quiet relative to the rest of the set?
* Which sections have poor balance?
* Is one instrument dominating the mix?
* What changed between rehearsal takes?

---

## 4. Goals

### 4.1 Primary Goals

1. Provide easy-to-understand loudness and balance feedback from rehearsal or live recordings.
2. Support practical decision-making for amateur musicians preparing for live performance.
3. Demonstrate a robust software pipeline built in C++ with MATLAB-based analysis and reporting.
4. Produce reusable, well-documented, open-source code that can be extended by other developers.
5. Export structured outputs suitable for quantitative analysis and reproducible workflows.

### 4.2 Secondary Goals

1. Support comparing multiple songs and multiple takes.
2. Enable section-level analysis, such as verse, chorus, bridge, intro, or outro.
3. Provide simple visual summaries appropriate for non-technical users.
4. Offer modular architecture so future contributors can add source separation, machine learning models, or new visualizations.

### 4.3 Non-Goals

1. Replace a full digital audio workstation.
2. Perform studio-grade source isolation in version 1.
3. Provide real-time live mixing control in version 1.
4. Guarantee perfect source attribution from a single stereo recording.
5. Serve as a mastering tool.

---

## 5. User Personas

### 5.1 Amateur Band Member

A guitarist, vocalist, or drummer who records rehearsals on a phone or portable recorder and wants clear feedback on whether the band mix is balanced.

### 5.2 Band Leader / Musical Director

A person responsible for set cohesion who wants to compare songs and identify where vocals are consistently masked or where songs are too dense.

### 5.3 Volunteer Sound Operator

A non-professional audio helper at a church, school, or community venue who wants objective clues about recurring balance issues.

### 5.4 Student Research/Engineering Evaluator

A technical reviewer interested in the software architecture, modularity, export design, and documentation quality.

---

## 6. Use Cases

1. A band uploads rehearsal recordings for five songs and receives per-song loudness summaries.
2. A singer checks whether vocals are buried during choruses.
3. A group compares two takes of the same song and identifies whether the second take improved vocal clarity.
4. A solo performer using backing tracks checks whether lead vocal level remains intelligible across the set.
5. A user exports metrics to MATLAB and generates a PDF-like analysis summary for discussion with bandmates.

---

## 7. Product Scope

### 7.1 In Scope for Version 1

* Offline analysis of mono or stereo audio recordings
* Single-recording and multi-recording batch analysis
* Song-level and section-level loudness summaries
* Relative balance estimation using source proxies or user-defined roles
* Detection of vocal masking risk using frequency-energy overlap heuristics
* Setlist-level normalization and comparison views
* Export to CSV and MATLAB-compatible data structures
* MATLAB scripts for analysis, plotting, and report generation
* Cross-platform command-line engine in C++
* User-friendly desktop GUI wrapper or lightweight frontend over the C++ backend
* Documentation for installation, usage, architecture, and contribution

### 7.2 Out of Scope for Version 1

* Cloud processing
* Real-time plugin for DAWs
* Mobile app
* Automatic multitrack mixing
* Perfect speaker/instrument diarization from uncontrolled recordings

---

## 8. Product Vision

The application should function as a practical “rehearsal mix coach.” A user records their set, imports recordings into the application, and receives plain-language feedback such as:

* “Vocals were likely masked in Songs 2 and 4 during loud sections.”
* “Song 3 is significantly louder than the rest of the set.”
* “Low-frequency energy dominates the intro of Song 1.”
* “Choruses in Song 5 show reduced vocal-to-band balance compared with verses.”

The user should not need advanced audio engineering knowledge to interpret these results.

---

## 9. Functional Requirements

## 9.1 Audio Import

The system shall:

1. Accept WAV as the primary supported format.
2. Optionally support FLAC and MP3 through documented third-party libraries.
3. Accept mono and stereo files.
4. Allow importing one file or multiple files in batch.
5. Validate file readability and provide clear user-facing error messages.
6. Capture metadata such as filename, duration, sample rate, channel count, and import timestamp.

## 9.2 Project and Session Management

The system shall:

1. Allow users to create an analysis session.
2. Allow users to group imported files into a setlist.
3. Allow users to assign song names and ordering.
4. Allow users to store and reload project configuration.
5. Preserve user-defined section labels and role mappings.

## 9.3 Preprocessing

The system shall:

1. Convert input audio into a standard internal representation.
2. Resample audio to a configurable sample rate if required.
3. Normalize signal representation for feature extraction without altering original data.
4. Apply optional noise-floor estimation.
5. Support channel summing or independent channel analysis.
6. Log preprocessing decisions for reproducibility.

## 9.4 Segmentation

The system shall:

1. Support whole-song analysis by default.
2. Support manual section labeling by the user.
3. Support automatic coarse segmentation using changes in energy, spectral content, or silence.
4. Allow export of section boundaries.
5. Permit users to edit or override automatic section boundaries.

## 9.5 Loudness Analysis

The system shall:

1. Estimate integrated loudness at song level.
2. Estimate short-term loudness over time.
3. Compute windowed RMS and peak-based summary measures.
4. Compute band-limited energy measures for low, mid, and high frequency regions.
5. Estimate dynamic range per song and per section.
6. Flag songs whose average loudness differs materially from the set median or target.
7. Produce relative loudness rankings across the setlist.

## 9.6 Balance Estimation

Because amateur users may only have stereo rehearsal recordings, the system shall support proxy-based balance estimation.

The system shall:

1. Estimate vocal-presence proxies using vocal-dominant frequency regions and optional speech/music heuristics.
2. Estimate accompaniment energy in competing frequency bands.
3. Compute a vocal-to-accompaniment balance score over time.
4. Identify sections where vocal masking risk exceeds a configurable threshold.
5. Estimate instrument dominance risk in broad categories such as low-end dominance, midrange congestion, and high-frequency harshness.
6. Allow future plugins or modules for better source separation without redesigning the pipeline.

## 9.7 Comparative Analysis

The system shall:

1. Compare songs within a setlist.
2. Compare multiple takes of the same song.
3. Highlight changes in loudness balance between takes.
4. Identify most balanced and least balanced songs using a transparent scoring method.
5. Generate setlist consistency metrics.

## 9.8 User Feedback and Recommendations

The system shall generate user-facing messages such as:

1. “Vocals may be masked during chorus sections.”
2. “This song is louder than the set average.”
3. “The low-frequency region dominates the mix in the intro.”
4. “Take 2 shows improved vocal balance versus Take 1.”

The system shall: 5. Explain results in plain language. 6. Avoid overstating certainty. 7. Distinguish heuristic findings from directly measured statistics.

## 9.9 Export

The system shall export:

1. Song-level summary CSV files.
2. Section-level summary CSV files.
3. Time-series CSV files containing loudness and balance metrics.
4. MATLAB-friendly files or folder structures for direct import.
5. Configuration and analysis metadata for reproducibility.

## 9.10 MATLAB Analysis Layer

The MATLAB component shall:

1. Load exported C++ outputs.
2. Validate file structure and required fields.
3. Generate plots for loudness over time, setlist comparisons, and section balance.
4. Compute secondary summary metrics.
5. Produce publication-quality or portfolio-quality figures.
6. Support a scripted report workflow.
7. Be organized into reusable functions rather than a single monolithic script.

## 9.11 Open-Source Requirements

The project shall:

1. Be hosted in a public repository.
2. Include a permissive or research-friendly open-source license.
3. Include build instructions for all supported platforms.
4. Include example data or synthetic test data if distribution rights allow.
5. Include contribution guidelines.
6. Include issue templates and a basic roadmap.

---

## 10. User Experience Requirements

## 10.1 Design Principles

The application must be:

* approachable,
* clear,
* minimally technical in presentation,
* transparent about uncertainty,
* fast for typical rehearsal recordings,
* visually simple and uncluttered.

## 10.2 Target UX Characteristics

1. A new user should be able to import files and run analysis without reading technical documentation.
2. The default workflow should require minimal configuration.
3. Advanced settings should be available but not intrusive.
4. Users should be able to understand major results within a few minutes.
5. Technical outputs should still be accessible for advanced users.

## 10.3 Core User Workflow

1. Create or open a project.
2. Import one or more rehearsal recordings.
3. Assign song names and optional set order.
4. Run analysis.
5. Review summary cards or tables.
6. Inspect song-level and section-level plots.
7. Export data and open MATLAB scripts for deeper analysis.

## 10.4 GUI Requirements

If a GUI is included in version 1, it shall:

1. Provide drag-and-drop audio import.
2. Show processing progress and status.
3. Present results as simple cards, tables, and plots.
4. Use plain-language labels.
5. Allow export with one click.
6. Allow users to toggle between beginner and advanced views.

---

## 11. System Architecture

## 11.1 High-Level Architecture

The application shall use a two-part architecture:

### C++ Core

Responsible for:

* audio I/O
* preprocessing
* feature extraction
* segmentation
* loudness and balance estimation
* result serialization/export

### MATLAB Analysis Package

Responsible for:

* loading exported data
* advanced metric computation
* figure generation
* comparison workflows
* optional summary report generation

## 11.2 Architectural Principles

1. Modular pipeline design
2. Clear interfaces between stages
3. Separation of algorithmic core and presentation layer
4. Reproducibility via configuration files and logs
5. Easy substitution of analysis modules

## 11.3 Proposed C++ Modules

* `audio_io`
* `preprocessing`
* `segmentation`
* `features`
* `loudness`
* `balance`
* `scoring`
* `export`
* `config`
* `logging`
* `cli`
* `gui_adapter` (if GUI is implemented)

## 11.4 Proposed MATLAB Modules

* `load_project_data.m`
* `plot_song_summary.m`
* `plot_setlist_comparison.m`
* `plot_section_balance.m`
* `compute_secondary_metrics.m`
* `generate_summary_report.m`
* `validate_exports.m`

---

## 12. Technical Requirements

## 12.1 C++ Requirements

1. The analysis engine shall be implemented in modern C++.
2. The codebase shall favor readable, modular, maintainable design.
3. The engine shall compile on at least one major desktop platform, with portability considered.
4. The engine shall provide a command-line interface for reproducible use.
5. The build system shall be based on CMake.
6. External dependencies shall be documented clearly.

## 12.2 MATLAB Requirements

1. MATLAB scripts shall be compatible with a documented baseline MATLAB version.
2. Scripts shall avoid unnecessary toolbox requirements where practical.
3. MATLAB functions shall include comments, input validation, and examples.
4. Figures shall be reproducible from exported data.
5. MATLAB code shall mirror the modularity standards of the C++ side.

## 12.3 Interoperability Requirements

1. The exported data format shall be documented.
2. The C++ outputs shall be directly consumable by MATLAB scripts.
3. File naming and folder structure shall be consistent and machine-readable.
4. Each analysis run shall include metadata sufficient to reproduce the result.

---

## 13. Data Model Requirements

The system shall define structured entities for:

* Project
* Recording
* Song
* Section
* Frame or time window
* Metric
* Analysis configuration
* Export manifest

Each exported result should include:

* project identifier
* song identifier
* take identifier if applicable
* timestamp or sample range
* metric name
* metric value
* units where appropriate
* confidence or heuristic note where applicable

---

## 14. Algorithm Requirements

## 14.1 Loudness Metrics

The system shall support at minimum:

* integrated loudness estimate
* short-term loudness estimate
* RMS energy
* peak summary
* dynamic range summary
* frequency-band energy summaries

## 14.2 Balance Metrics

The system shall compute at minimum:

* vocal-presence proxy score
* accompaniment energy proxy score
* vocal-to-accompaniment balance ratio or difference
* masking-risk indicator
* section-level balance stability score
* setlist-level consistency score

## 14.3 Confidence and Interpretation

Because some estimates are heuristic, the system shall:

1. Mark heuristic outputs clearly.
2. Provide confidence categories where possible.
3. Avoid language suggesting perfect separation or certainty.

---

## 15. Performance Requirements

1. The system should process typical rehearsal recordings efficiently on consumer hardware.
2. The interface should remain responsive during analysis.
3. Batch analysis should scale reasonably for a small setlist.
4. Memory usage should remain bounded and documented.
5. Failures should be recoverable without corrupting project files.

---

## 16. Reliability Requirements

1. The application shall validate inputs before processing.
2. The application shall handle unsupported or corrupt files gracefully.
3. Export routines shall verify successful file creation.
4. Logs shall capture warnings and errors.
5. Partial failures in batch mode shall not discard successful analyses of other files.

---

## 17. Documentation Requirements

The repository shall include:

1. README with product overview and quick start
2. Installation guide
3. Build guide for the C++ engine
4. MATLAB usage guide
5. Architecture overview
6. Configuration reference
7. Export format reference
8. Contribution guide
9. Sample workflows
10. Troubleshooting guide

All public functions and major modules should include developer-facing comments.

---

## 18. Accessibility and Usability Requirements

1. UI text shall avoid unnecessary technical jargon.
2. Key outputs shall be color-independent where possible.
3. Graphs shall use clear labels and legends.
4. Error messages shall be actionable.
5. The application should support keyboard navigation if a GUI is included.

---

## 19. Security and Privacy Requirements

1. Audio analysis shall run locally in version 1.
2. User recordings shall not be uploaded by default.
3. Logs shall avoid exposing unnecessary personal file paths in shared reports when possible.
4. Exported reports shall contain only analysis-related metadata unless the user opts in to more detail.

---

## 20. Testing Requirements

## 20.1 Unit Testing

The project shall include tests for:

* preprocessing utilities
* segmentation utilities
* loudness calculations
* export routines
* configuration parsing

## 20.2 Integration Testing

The project shall include tests for:

* end-to-end single-file processing
* end-to-end batch processing
* export-to-MATLAB compatibility
* project save/load workflows

## 20.3 Validation Testing

The project shall include validation on:

* synthetic audio with known loudness differences
* controlled mixtures with known vocal-to-accompaniment ratios
* repeated runs for consistency

## 20.4 Usability Testing

At minimum, informal testing should confirm that first-time users can:

* import files,
* run analysis,
* understand the summary output,
* export results, without developer assistance.

---

## 21. Success Metrics

The project will be considered successful if:

1. Users can run analysis on a rehearsal recording with minimal setup.
2. The application produces interpretable song-level and setlist-level feedback.
3. The C++ and MATLAB components work together cleanly.
4. The codebase is understandable and extendable by another developer.
5. Documentation is sufficient for an external user to install and run the software.
6. Example outputs are portfolio-worthy and clearly connected to user needs.

---

## 22. Release Criteria for Version 1

Version 1 is release-ready when all of the following are true:

1. C++ command-line analysis pipeline is functional.
2. Export format is stable and documented.
3. MATLAB plotting and summary scripts run successfully on exported data.
4. At least one basic user-facing interface exists, either CLI-first with polished outputs or a lightweight GUI.
5. README and installation instructions are complete.
6. Example dataset and demo workflow are included.
7. Core test suite passes.

---

## 23. Future Enhancements

Potential future versions may include:

* improved vocal detection or source separation
* live monitoring mode
* DAW integration
* room-response estimation
* genre-specific heuristics
* ML-based masking prediction
* web-based report viewer
* multitrack support

---

## 24. Recommended Repository Structure

```text
setlist-loudness-balancer/
├── README.md
├── LICENSE
├── CMakeLists.txt
├── docs/
│   ├── architecture.md
│   ├── export_format.md
│   ├── contributing.md
│   └── user_guide.md
├── cpp/
│   ├── include/
│   ├── src/
│   ├── tests/
│   └── third_party/
├── matlab/
│   ├── load_project_data.m
│   ├── compute_secondary_metrics.m
│   ├── plot_song_summary.m
│   ├── plot_setlist_comparison.m
│   ├── plot_section_balance.m
│   ├── generate_summary_report.m
│   └── examples/
├── config/
├── examples/
│   ├── audio/
│   └── expected_outputs/
└── output/
```

---

## 25. Example User Stories

1. As a singer, I want to know which songs bury the vocals so that I can adjust rehearsal priorities.
2. As a band leader, I want to compare songs across a setlist so that the overall show feels more balanced.
3. As a volunteer sound operator, I want plain-language recommendations so that I can make practical changes without deep engineering expertise.
4. As a developer, I want modular C++ analysis components and documented MATLAB scripts so that I can extend the tool.

---

## 26. Example Acceptance Criteria

### 26.1 Single Song Analysis

* Given a valid WAV file, the application imports it successfully.
* The application computes song-level loudness metrics.
* The application exports summary CSV files.
* MATLAB scripts load the exports without manual editing.

### 26.2 Setlist Comparison

* Given multiple song files, the application compares relative loudness across songs.
* The output identifies songs above and below the set average.
* MATLAB generates a setlist comparison figure.

### 26.3 Section-Level Feedback

* Given manually defined or automatically detected sections, the application computes section-level balance measures.
* The output flags at least one section with elevated masking risk when such a condition exists.

### 26.4 Reproducibility

* Running analysis twice with the same configuration on the same input yields consistent outputs.

---

## 27. Risks and Mitigations

### Risk 1: Poor source attribution from stereo rehearsal recordings

**Mitigation:** Frame balance metrics as heuristic proxies, not exact source-isolated measurements.

### Risk 2: Scope becoming too ambitious

**Mitigation:** Prioritize a clean version 1 centered on loudness, segmentation, export, and MATLAB analysis.

### Risk 3: User confusion around technical metrics

**Mitigation:** Present plain-language summaries and tooltips alongside quantitative values.

### Risk 4: Dependency complexity for open-source users

**Mitigation:** Keep dependencies minimal, use CMake, and provide sample build configurations.

---

## 28. Positioning Statement

Setlist Loudness Balancer is a user-friendly open-source rehearsal analysis application that transforms ordinary live recordings into practical loudness and balance feedback. Built with a performant C++ core and a MATLAB analysis layer, it combines engineering rigor with musician-friendly usability.

---

## 29. Final Requirement Summary

The project must demonstrate:

* strong C++ software engineering,
* meaningful MATLAB integration,
* modular and documented design,
* clear practical value for end users,
* open-source readiness,
* and a polished research-software mindset.

This combination makes the application both a compelling portfolio project and a credible example of analysis-oriented software development.

