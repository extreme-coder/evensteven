function data = load_project_data(json_path)
% LOAD_PROJECT_DATA Load EvenSteven analysis JSON into MATLAB struct.
%
%   data = load_project_data('path/to/analysis.json')
%
% Returns a struct with fields: project_id, project_name, timestamp,
% setlist, songs (array of structs).

    if nargin < 1
        json_path = fullfile('..', 'examples', 'expected_outputs', 'analysis.json');
    end

    raw = fileread(json_path);
    data = jsondecode(raw);

    fprintf('Loaded project: %s (%d songs)\n', data.project_name, length(data.songs));
end
