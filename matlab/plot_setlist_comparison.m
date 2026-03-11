function plot_setlist_comparison(data, save_path)
% PLOT_SETLIST_COMPARISON Bar chart of integrated LUFS per song.
%
%   plot_setlist_comparison(data)
%   plot_setlist_comparison(data, 'output.png')

    n = length(data.songs);
    names = cell(n, 1);
    lufs = zeros(n, 1);
    colors = zeros(n, 3);

    for i = 1:n
        names{i} = data.songs(i).song_name;
        val = data.songs(i).loudness.integrated_lufs;
        if isempty(val) || ~isfinite(val)
            lufs(i) = -60;
        else
            lufs(i) = val;
        end

        grade = data.songs(i).scores.grade;
        if strcmp(grade, 'Good')
            colors(i, :) = [0.2 0.8 0.3];
        elseif strcmp(grade, 'Fair')
            colors(i, :) = [1.0 0.8 0.2];
        else
            colors(i, :) = [0.9 0.3 0.2];
        end
    end

    figure('Position', [100 100 800 500]);
    b = bar(lufs);
    b.FaceColor = 'flat';
    b.CData = colors;

    hold on;
    median_lufs = data.setlist.median_lufs;
    if ~isempty(median_lufs) && isfinite(median_lufs)
        yline(median_lufs, 'k--', sprintf('Median: %.1f LUFS', median_lufs), ...
              'LineWidth', 2, 'LabelHorizontalAlignment', 'left');
    end
    hold off;

    set(gca, 'XTickLabel', names, 'XTickLabelRotation', 30);
    ylabel('Integrated LUFS');
    title(sprintf('Setlist Loudness Comparison - %s (%s)', ...
          data.project_name, data.setlist.grade));
    grid on;

    if nargin >= 2
        saveas(gcf, save_path);
        fprintf('Saved: %s\n', save_path);
    end
end
