function plot_section_balance(song, save_path)
% PLOT_SECTION_BALANCE Grouped bar chart of balance metrics per section.
%
%   plot_section_balance(song)
%   plot_section_balance(song, 'output.png')

    if ~isfield(song, 'sections') || isempty(song.sections)
        fprintf('No sections available for %s\n', song.song_name);
        return;
    end

    n = length(song.sections);
    labels = cell(n, 1);
    durations = zeros(n, 1);
    masking_pcts = zeros(n, 1);

    for i = 1:n
        sec = song.sections(i);
        labels{i} = sec.label;
        durations(i) = sec.end_s - sec.start_s;

        % Compute masking percentage for this section
        if isfield(song.balance, 'timestamps') && isfield(song.balance, 'masking_risk')
            t = song.balance.timestamps;
            mask = song.balance.masking_risk;
            idx = (t >= sec.start_s) & (t <= sec.end_s);
            if any(idx)
                masking_pcts(i) = sum(mask(idx)) / sum(idx) * 100;
            end
        end
    end

    figure('Position', [100 100 800 500]);

    subplot(2, 1, 1);
    bar(durations);
    set(gca, 'XTickLabel', labels);
    ylabel('Duration (s)');
    title(sprintf('Section Durations: %s', song.song_name));
    grid on;

    subplot(2, 1, 2);
    b = bar(masking_pcts);
    b.FaceColor = 'flat';
    for i = 1:n
        if masking_pcts(i) > 30
            b.CData(i, :) = [0.9 0.3 0.2];
        elseif masking_pcts(i) > 10
            b.CData(i, :) = [1.0 0.8 0.2];
        else
            b.CData(i, :) = [0.2 0.8 0.3];
        end
    end
    set(gca, 'XTickLabel', labels);
    ylabel('Masking Risk (%)');
    title('Masking Risk per Section');
    grid on;

    if nargin >= 2
        saveas(gcf, save_path);
        fprintf('Saved: %s\n', save_path);
    end
end
