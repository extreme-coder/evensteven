% DEMO_WORKFLOW Complete example of loading and analyzing EvenSteven data.
%
% This script demonstrates the full MATLAB workflow:
% 1. Load analysis JSON
% 2. Validate data
% 3. Generate plots
% 4. Compute secondary metrics
%
% Run from the matlab/ directory:
%   cd matlab
%   run('examples/demo_workflow.m')

% Add parent directory to path
addpath('..');

% Configuration
json_path = fullfile('..', 'examples', 'expected_outputs', 'analysis.json');
output_dir = fullfile('examples', 'figures');

fprintf('=== EvenSteven MATLAB Demo Workflow ===\n\n');

% Step 1: Load data
fprintf('Step 1: Loading project data...\n');
data = load_project_data(json_path);
fprintf('  Project: %s\n', data.project_name);
fprintf('  Songs: %d\n', length(data.songs));
fprintf('\n');

% Step 2: Validate
fprintf('Step 2: Validating exports...\n');
valid = validate_exports(data);
if ~valid
    error('Validation failed. Check the analysis.json file.');
end
fprintf('\n');

% Step 3: Generate plots
fprintf('Step 3: Generating plots...\n');
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

plot_setlist_comparison(data, fullfile(output_dir, 'setlist_comparison.png'));

for i = 1:length(data.songs)
    song = data.songs(i);
    fprintf('  Processing: %s\n', song.song_name);
    plot_song_summary(song, fullfile(output_dir, sprintf('song_summary_%s.png', song.song_id)));
    plot_section_balance(song, fullfile(output_dir, sprintf('section_balance_%s.png', song.song_id)));
end
fprintf('\n');

% Step 4: Secondary metrics
fprintf('Step 4: Computing secondary metrics...\n');
metrics = compute_secondary_metrics(data);

fprintf('\n=== Results Summary ===\n');
fprintf('Setlist Grade: %s\n', data.setlist.grade);
fprintf('Median LUFS: %.1f\n', data.setlist.median_lufs);
fprintf('Consistency Score: %.2f\n', data.setlist.consistency_score);
fprintf('\nPer-Song:\n');
for i = 1:length(data.songs)
    song = data.songs(i);
    fprintf('  %s: %.1f LUFS, Grade=%s, Crest=%.1f dB\n', ...
            song.song_name, song.loudness.integrated_lufs, ...
            song.scores.grade, metrics.crest_factor(i));
end

fprintf('\nFigures saved to: %s\n', output_dir);
fprintf('=== Done ===\n');
