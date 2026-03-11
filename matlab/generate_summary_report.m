function generate_summary_report(json_path, output_dir)
% GENERATE_SUMMARY_REPORT Generate all plots and save as PNGs.
%
%   generate_summary_report('analysis.json', 'figures/')

    if nargin < 1
        json_path = fullfile('..', 'examples', 'expected_outputs', 'analysis.json');
    end
    if nargin < 2
        output_dir = fullfile('examples', 'figures');
    end

    if ~exist(output_dir, 'dir')
        mkdir(output_dir);
    end

    % Load data
    data = load_project_data(json_path);

    % Validate
    validate_exports(data);

    % Setlist comparison
    plot_setlist_comparison(data, fullfile(output_dir, 'setlist_comparison.png'));

    % Per-song summaries
    for i = 1:length(data.songs)
        song = data.songs(i);
        fname = sprintf('song_summary_%s.png', song.song_id);
        plot_song_summary(song, fullfile(output_dir, fname));

        fname2 = sprintf('section_balance_%s.png', song.song_id);
        plot_section_balance(song, fullfile(output_dir, fname2));
    end

    % Secondary metrics
    metrics = compute_secondary_metrics(data);
    fprintf('\nSummary Report Generated.\n');
    fprintf('  Songs: %d\n', length(data.songs));
    fprintf('  Mean LUFS: %.1f\n', metrics.lufs_mean);
    fprintf('  LUFS StdDev: %.1f\n', metrics.lufs_std);
    fprintf('  Output: %s\n', output_dir);
end
