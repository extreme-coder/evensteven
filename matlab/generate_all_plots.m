function generate_all_plots(json_path, output_dir)
% GENERATE_ALL_PLOTS Generate all EvenSteven plots and save as PNG.
%
%   generate_all_plots('analysis.json', 'output_dir')

    data = load_project_data(json_path);

    if ~exist(output_dir, 'dir')
        mkdir(output_dir);
    end

    % Setlist comparison
    plot_setlist_comparison(data, fullfile(output_dir, 'setlist_comparison.png'));
    close(gcf);

    % Per-song plots
    for i = 1:length(data.songs)
        song = data.songs(i);
        safe_name = regexprep(song.song_name, '[^a-zA-Z0-9_-]', '_');

        plot_song_summary(song, fullfile(output_dir, sprintf('%s_summary.png', safe_name)));
        close(gcf);

        plot_section_balance(song, fullfile(output_dir, sprintf('%s_sections.png', safe_name)));
        close(gcf);
    end

    fprintf('All plots saved to: %s\n', output_dir);
end
