function metrics = compute_secondary_metrics(data)
% COMPUTE_SECONDARY_METRICS Derive additional metrics from analysis data.
%
%   metrics = compute_secondary_metrics(data)
%
% Returns struct with: spectral_centroid, crest_factor, lra, cross_correlation.

    n = length(data.songs);
    metrics = struct();
    metrics.song_names = cell(n, 1);
    metrics.crest_factor = zeros(n, 1);
    metrics.lra = zeros(n, 1);
    metrics.dynamic_range = zeros(n, 1);

    for i = 1:n
        song = data.songs(i);
        metrics.song_names{i} = song.song_name;

        % Crest factor: peak_db - rms_db
        peak = song.loudness.peak_db;
        rms = song.loudness.rms_db;
        if ~isempty(peak) && ~isempty(rms) && isfinite(peak) && isfinite(rms)
            metrics.crest_factor(i) = peak - rms;
        end

        % Dynamic range
        if isfield(song.loudness, 'dynamic_range_db')
            metrics.dynamic_range(i) = song.loudness.dynamic_range_db;
        end

        % LRA approximation from short-term LUFS
        if isfield(song.loudness, 'shortterm_lufs')
            st = song.loudness.shortterm_lufs;
            st = st(isfinite(st));
            if length(st) > 10
                sorted = sort(st);
                p10 = sorted(round(length(sorted) * 0.10));
                p95 = sorted(round(length(sorted) * 0.95));
                metrics.lra(i) = p95 - p10;
            end
        end
    end

    % Cross-song LUFS correlation
    lufs_values = zeros(n, 1);
    for i = 1:n
        val = data.songs(i).loudness.integrated_lufs;
        if ~isempty(val) && isfinite(val)
            lufs_values(i) = val;
        end
    end
    metrics.lufs_values = lufs_values;
    metrics.lufs_mean = mean(lufs_values);
    metrics.lufs_std = std(lufs_values);

    fprintf('Secondary metrics computed for %d songs.\n', n);
end
