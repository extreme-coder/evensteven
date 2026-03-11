function plot_song_summary(song, save_path)
% PLOT_SONG_SUMMARY Create a 2x2 subplot summary for a single song.
%
%   plot_song_summary(song)
%   plot_song_summary(song, 'output.png')

    figure('Position', [100 100 1000 800]);

    % 1. LUFS timeline
    subplot(2, 2, 1);
    if isfield(song.loudness, 'timestamps') && isfield(song.loudness, 'shortterm_lufs')
        t = song.loudness.timestamps;
        plot(t, song.loudness.shortterm_lufs, 'b-', 'LineWidth', 1.5);
        hold on;
        if isfield(song.loudness, 'momentary_lufs')
            plot(t, song.loudness.momentary_lufs, 'c-', 'LineWidth', 0.5);
        end
        if isfield(song.loudness, 'integrated_lufs') && ~isempty(song.loudness.integrated_lufs)
            yline(song.loudness.integrated_lufs, 'r--', 'LineWidth', 1);
        end
        hold off;
        legend('Short-term', 'Momentary', 'Integrated', 'Location', 'best');
    end
    xlabel('Time (s)');
    ylabel('LUFS');
    title(sprintf('Loudness: %s', song.song_name));
    grid on;

    % 2. Band energy
    subplot(2, 2, 2);
    if isfield(song.loudness, 'band_energy')
        be = song.loudness.band_energy;
        names = fieldnames(be);
        values = zeros(length(names), 1);
        for i = 1:length(names)
            val = be.(names{i});
            if isempty(val) || (isnumeric(val) && ~isfinite(val))
                values(i) = -60;
            else
                values(i) = val;
            end
        end
        bar(values);
        set(gca, 'XTickLabel', names);
        ylabel('Energy (dB)');
        title('Band Energy');
        grid on;
    end

    % 3. Vocal vs Accompaniment
    subplot(2, 2, 3);
    if isfield(song.balance, 'timestamps') && isfield(song.balance, 'vocal_energy_db')
        t = song.balance.timestamps;
        plot(t, song.balance.vocal_energy_db, 'g-', 'LineWidth', 1);
        hold on;
        plot(t, song.balance.accompaniment_energy_db, 'r-', 'LineWidth', 1);
        hold off;
        legend('Vocal', 'Accompaniment', 'Location', 'best');
        xlabel('Time (s)');
        ylabel('Energy (dB)');
        title('Vocal vs Accompaniment');
        grid on;
    end

    % 4. Masking risk
    subplot(2, 2, 4);
    if isfield(song.balance, 'timestamps') && isfield(song.balance, 'balance_db')
        t = song.balance.timestamps;
        plot(t, song.balance.balance_db, 'b-', 'LineWidth', 1);
        hold on;
        yline(-3, 'r--', 'Masking Threshold', 'LineWidth', 1);
        if isfield(song.balance, 'masking_risk')
            masked_idx = find(song.balance.masking_risk);
            if ~isempty(masked_idx)
                plot(t(masked_idx), song.balance.balance_db(masked_idx), 'r.', 'MarkerSize', 8);
            end
        end
        hold off;
        xlabel('Time (s)');
        ylabel('Balance (dB)');
        title(sprintf('Masking Risk (%.0f%%)', song.balance.masking_risk_score * 100));
        grid on;
    end

    sgtitle(sprintf('%s - %s', song.song_name, song.scores.grade), 'FontSize', 14);

    if nargin >= 2
        saveas(gcf, save_path);
        fprintf('Saved: %s\n', save_path);
    end
end
