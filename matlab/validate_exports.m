function valid = validate_exports(data)
% VALIDATE_EXPORTS Check that analysis JSON has required fields.
%
%   valid = validate_exports(data)
%
% Returns true if all required fields are present and valid.

    valid = true;

    % Check top-level fields
    required_fields = {'project_id', 'project_name', 'timestamp', 'setlist', 'songs'};
    for i = 1:length(required_fields)
        if ~isfield(data, required_fields{i})
            fprintf('Missing top-level field: %s\n', required_fields{i});
            valid = false;
        end
    end

    % Check setlist fields
    setlist_fields = {'median_lufs', 'consistency_score', 'grade'};
    for i = 1:length(setlist_fields)
        if ~isfield(data.setlist, setlist_fields{i})
            fprintf('Missing setlist field: %s\n', setlist_fields{i});
            valid = false;
        end
    end

    % Check each song
    song_fields = {'song_id', 'song_name', 'loudness', 'balance', 'scores'};
    for s = 1:length(data.songs)
        song = data.songs(s);
        for i = 1:length(song_fields)
            if ~isfield(song, song_fields{i})
                fprintf('Song %d missing field: %s\n', s, song_fields{i});
                valid = false;
            end
        end

        % Validate loudness arrays
        if isfield(song, 'loudness') && isfield(song.loudness, 'shortterm_lufs')
            if isfield(song.loudness, 'timestamps')
                if length(song.loudness.shortterm_lufs) ~= length(song.loudness.timestamps)
                    fprintf('Song %d: shortterm_lufs/timestamps size mismatch\n', s);
                    valid = false;
                end
            end
        end
    end

    if valid
        fprintf('Validation passed.\n');
    else
        fprintf('Validation FAILED.\n');
    end
end
